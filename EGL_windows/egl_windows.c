/*
 * egl_windows.c — GLFW+EGL windowing backend for bicgl.
 *
 * Uses GLFW to create windows and OpenGL contexts (via EGL on x2go/X11).
 * Falls back to GLFW_EGL_CONTEXT_API when the default GLX path fails,
 * which is exactly the x2go case (no GLX extension on the X server).
 *
 * Font rendering uses the shared font_atlas/font_render_gl stb_truetype
 * pipeline (bundled JetBrains Mono, baked natively at the window's actual
 * backing-scale pixel density) -- the same rendering path used by the GLX
 * and GLUT backends, so text looks identical across all three and across
 * both Linux and macOS.
 *
 * This file implements the full WS_* interface declared in
 * EGL_windows/Include/egl_window_prototypes.h.
 */

/* Include GLFW native header FIRST, before any X11 headers that do
 * #undef Status (WS_graphics.h).  glfw3native.h pulls in Xrandr.h which
 * uses Status as a function return type; if Status is already undef-ed,
 * compilation fails.  Including it here keeps Status intact.
 *
 * None of this applies on macOS: GLFW has no X11 backend there (Cocoa only),
 * so GLFW_EXPOSE_NATIVE_X11 is never defined and the native/X11 font-loading
 * path below compiles out, falling back to the stored bitmap font. */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#if !defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>  /* glfwGetX11Display(), glfwGetX11Window() */
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <volume_io.h>
#include <WS_graphics.h>       /* does #undef Status — must come after glfw3native.h */

/* OpenGL */
#define GL_GLEXT_PROTOTYPES
#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#if !defined(__APPLE__)
/* X11 — needed for the Window_id type and native-Wayland detection below
 * (Xlib.h / Xutil.h already pulled in above via WS_graphics.h) */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <font_atlas.h>
#include <font_render_gl.h>
#include <jetbrains_mono_ttf.h>  /* generated: embedded font bytes, see CMakeLists.txt */

/* -----------------------------------------------------------------------
 * GLFW globals
 * --------------------------------------------------------------------- */

#if defined(__APPLE__)
static void       *s_x11_display    = NULL;  /* always NULL on macOS       */
#else
static Display    *s_x11_display    = NULL;  /* from glfwGetX11Display(); NULL
                                                 result also drives native-
                                                 Wayland detection below     */
#endif
static GLFWwindow *s_first_glfw_win = NULL;  /* share target for 2nd+ wins */
static int         s_context_api    = 0;     /* GLFW_NATIVE_CONTEXT_API or
                                                GLFW_EGL_CONTEXT_API; 0 = not
                                                yet determined               */
static VIO_BOOL    s_is_native_wayland = FALSE; /* TRUE when running native
                                                   Wayland (not XWayland)    */

/* -----------------------------------------------------------------------
 * Window registry — maps GLFWwindow* → WSwindow
 * --------------------------------------------------------------------- */

#define  MAX_EGL_WINDOWS  32

static struct {
    GLFWwindow *glfw;
    WSwindow    ws;
} s_windows[MAX_EGL_WINDOWS];
static int s_n_windows = 0;

/* The window whose GL context is currently current */
static WSwindow  s_current_window = NULL;

static void register_window( GLFWwindow *glfw, WSwindow ws )
{
    if( s_n_windows < MAX_EGL_WINDOWS )
    {
        s_windows[s_n_windows].glfw = glfw;
        s_windows[s_n_windows].ws   = ws;
        ++s_n_windows;
    }
}

static void unregister_window( GLFWwindow *glfw )
{
    int i;
    for( i = 0; i < s_n_windows; ++i )
    {
        if( s_windows[i].glfw == glfw )
        {
            s_windows[i] = s_windows[--s_n_windows];
            return;
        }
    }
}

static WSwindow lookup_window( GLFWwindow *glfw )
{
    int i;
    for( i = 0; i < s_n_windows; ++i )
        if( s_windows[i].glfw == glfw )
            return s_windows[i].ws;
    return NULL;
}

/* -----------------------------------------------------------------------
 * Global event callbacks (same pattern as glut_windows.c)
 * --------------------------------------------------------------------- */

static void (*display_callback)         ( Window_id );
static void (*display_overlay_callback) ( Window_id );
static void (*resize_callback)          ( Window_id, int, int, int, int );
static void (*key_down_callback)        ( Window_id, int, int, int, int );
static void (*key_up_callback)          ( Window_id, int, int, int, int );
static void (*mouse_motion_callback)    ( Window_id, int, int );
static void (*left_down_callback)       ( Window_id, int, int, int );
static void (*left_up_callback)         ( Window_id, int, int, int );
static void (*middle_down_callback)     ( Window_id, int, int, int );
static void (*middle_up_callback)       ( Window_id, int, int, int );
static void (*right_down_callback)      ( Window_id, int, int, int );
static void (*right_up_callback)        ( Window_id, int, int, int );
static void (*scroll_up_callback)       ( Window_id, int, int, int );
static void (*scroll_down_callback)     ( Window_id, int, int, int );
static void (*iconify_callback)         ( Window_id );
static void (*deiconify_callback)       ( Window_id );
static void (*enter_callback)           ( Window_id );
static void (*leave_callback)           ( Window_id );
static void (*quit_callback)            ( Window_id );

/* -----------------------------------------------------------------------
 * Timer support
 * --------------------------------------------------------------------- */

typedef struct
{
    struct timeval  fire_time;
    void          (*func)( void * );
    void           *data;
    VIO_BOOL        active;
} timer_entry;

static timer_entry  *s_timers   = NULL;
static int           s_n_timers = 0;

static void timeval_add_seconds( struct timeval *tv, VIO_Real seconds )
{
    long usec = (long)( seconds * 1e6 );
    tv->tv_sec  += usec / 1000000L;
    tv->tv_usec += usec % 1000000L;
    if( tv->tv_usec >= 1000000L )
    {
        tv->tv_sec  += 1;
        tv->tv_usec -= 1000000L;
    }
}

/* Returns microseconds until tv (negative if past) */
static long timeval_usec_until( const struct timeval *tv )
{
    struct timeval now;
    gettimeofday( &now, NULL );
    return ( tv->tv_sec  - now.tv_sec  ) * 1000000L
         + ( tv->tv_usec - now.tv_usec );
}

/* -----------------------------------------------------------------------
 * Idle support
 * --------------------------------------------------------------------- */

typedef struct
{
    void (*func)( void * );
    void  *data;
} idle_entry;

static idle_entry  *s_idles   = NULL;
static int          s_n_idles = 0;

/* -----------------------------------------------------------------------
 * Event-loop control
 * --------------------------------------------------------------------- */

static VIO_BOOL s_quit_loop = FALSE;

/* -----------------------------------------------------------------------
 * Modifier and key translation helpers
 * --------------------------------------------------------------------- */

static int glfw_mods_to_bicgl( int mods )
{
    return ( (mods & GLFW_MOD_SHIFT)   ? SHIFT_KEY_BIT : 0 )
         | ( (mods & GLFW_MOD_CONTROL) ? CTRL_KEY_BIT  : 0 )
         | ( (mods & GLFW_MOD_ALT)     ? ALT_KEY_BIT   : 0 );
}

/* Map GLFW special key codes to bicgl key constants.
 * Returns TRUE and sets *out when the key is a special (non-printable) key.
 * Printable keys are handled by the char callback. */
static VIO_BOOL translate_glfw_special( int key, int *out )
{
    switch( key )
    {
    case GLFW_KEY_LEFT:          *out = LEFT_ARROW_KEY;   return TRUE;
    case GLFW_KEY_RIGHT:         *out = RIGHT_ARROW_KEY;  return TRUE;
    case GLFW_KEY_UP:            *out = UP_ARROW_KEY;     return TRUE;
    case GLFW_KEY_DOWN:          *out = DOWN_ARROW_KEY;   return TRUE;
    case GLFW_KEY_LEFT_SHIFT:    *out = LEFT_SHIFT_KEY;   return TRUE;
    case GLFW_KEY_RIGHT_SHIFT:   *out = RIGHT_SHIFT_KEY;  return TRUE;
    case GLFW_KEY_LEFT_CONTROL:  *out = LEFT_CTRL_KEY;    return TRUE;
    case GLFW_KEY_RIGHT_CONTROL: *out = RIGHT_CTRL_KEY;   return TRUE;
    case GLFW_KEY_LEFT_ALT:      *out = LEFT_ALT_KEY;     return TRUE;
    case GLFW_KEY_RIGHT_ALT:     *out = RIGHT_ALT_KEY;    return TRUE;
    case GLFW_KEY_F1:            *out = BICGL_F1_KEY;     return TRUE;
    case GLFW_KEY_F2:            *out = BICGL_F2_KEY;     return TRUE;
    case GLFW_KEY_F3:            *out = BICGL_F3_KEY;     return TRUE;
    case GLFW_KEY_F4:            *out = BICGL_F4_KEY;     return TRUE;
    case GLFW_KEY_F5:            *out = BICGL_F5_KEY;     return TRUE;
    case GLFW_KEY_F6:            *out = BICGL_F6_KEY;     return TRUE;
    case GLFW_KEY_F7:            *out = BICGL_F7_KEY;     return TRUE;
    case GLFW_KEY_F8:            *out = BICGL_F8_KEY;     return TRUE;
    case GLFW_KEY_F9:            *out = BICGL_F9_KEY;     return TRUE;
    case GLFW_KEY_F10:           *out = BICGL_F10_KEY;    return TRUE;
    case GLFW_KEY_F11:           *out = BICGL_F11_KEY;    return TRUE;
    case GLFW_KEY_F12:           *out = BICGL_F12_KEY;    return TRUE;
    case GLFW_KEY_PAGE_UP:       *out = BICGL_PGUP_KEY;   return TRUE;
    case GLFW_KEY_PAGE_DOWN:     *out = BICGL_PGDN_KEY;   return TRUE;
    case GLFW_KEY_HOME:          *out = BICGL_HOME_KEY;   return TRUE;
    case GLFW_KEY_END:           *out = BICGL_END_KEY;    return TRUE;
    case GLFW_KEY_INSERT:        *out = BICGL_INSERT_KEY; return TRUE;
    case GLFW_KEY_DELETE:        *out = 127;              return TRUE;
    case GLFW_KEY_ENTER:         *out = '\r';             return TRUE;
    case GLFW_KEY_KP_ENTER:      *out = '\r';             return TRUE;
    case GLFW_KEY_TAB:           *out = '\t';             return TRUE;
    case GLFW_KEY_BACKSPACE:     *out = '\b';             return TRUE;
    case GLFW_KEY_ESCAPE:        *out = '\033';           return TRUE;
    default:                     return FALSE;
    }
}

/* Y-coordinate flip (OpenGL is bottom-up, GLFW/X11 is top-down) */
static int flip_y( WSwindow ws, int y )
{
    return ws->height - 1 - y;
}

/* Scale a logical cursor coordinate to framebuffer (physical) pixels.
 * GLFW delivers cursor positions in logical pixels; all internal bicgl
 * coordinates are in framebuffer pixels (matching glViewport). */
static int cursor_px( float scale, double logical )
{
    return (int)( logical * scale );
}

/* Set ws->width/height and ws->dpi_scale from the GLFW window gw.
 *
 * ws->width/height must always equal glfwGetFramebufferSize — that is what
 * OpenGL actually renders into, and what glViewport and the layout engine
 * must use.
 *
 * On GLFW 3.3 with the X11 backend (covers both native X11 and XWayland):
 *   glfwGetFramebufferSize == logical window size in both cases.
 *   The compositor on XWayland handles pixel doubling transparently.
 *   So ws->width = fb_w = logical_w on both platforms.
 *
 * dpi_scale is used only for scaling cursor coordinates from logical to
 * framebuffer pixels (cursor_px()).  On native X11 fb==logical so scale=1.0.
 * On XWayland fb==logical too, BUT glfwGetWindowContentScale returns 2.0
 * (the compositor scale) — and GLFW already delivers cursor positions in
 * logical pixels, so we should NOT scale them either.  dpi_scale = 1.0
 * everywhere on the X11/XWayland GLFW backend.
 *
 * If a future native Wayland backend is used (GLFW_PLATFORM=wayland),
 * glfwGetFramebufferSize will return true physical pixels and fb/logical
 * will naturally give the correct scale > 1.0. */
static void update_window_scale( WSwindow ws, GLFWwindow *gw,
                                 int logical_w, int logical_h )
{
    int fb_w = logical_w, fb_h = logical_h;
    glfwGetFramebufferSize( gw, &fb_w, &fb_h );
    ws->logical_width  = logical_w;
    ws->logical_height = logical_h;
    ws->width          = fb_w;
    ws->height         = fb_h;
    ws->dpi_scale_x    = ( logical_w > 0 ) ? (float) fb_w / logical_w : 1.0f;
    ws->dpi_scale_y    = ( logical_h > 0 ) ? (float) fb_h / logical_h : 1.0f;
}

/* -----------------------------------------------------------------------
 * Cursor state (updated in cursor-pos callback; used by other callbacks)
 * --------------------------------------------------------------------- */

static double s_last_cursor_x = 0.0;
static double s_last_cursor_y = 0.0;
static int    s_current_mods  = 0;

/* -----------------------------------------------------------------------
 * GLFW callbacks
 * --------------------------------------------------------------------- */

static void glfw_error_cb( int code, const char *desc )
{
    fprintf( stderr, "GLFW error %d: %s\n", code, desc ? desc : "" );
}

static void glfw_key_cb( GLFWwindow *w, int key, int sc, int action, int mods )
{
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    (void) sc;
    if( !ws ) return;

    s_current_mods = glfw_mods_to_bicgl( mods );
    int bicgl_key = 0;
    int x = cursor_px( ws->dpi_scale_x, s_last_cursor_x );
    int y = flip_y( ws, cursor_px( ws->dpi_scale_y, s_last_cursor_y ) );

    if( action == GLFW_PRESS || action == GLFW_REPEAT )
    {
        if( translate_glfw_special( key, &bicgl_key ) )
        {
            if( key_down_callback )
                (*key_down_callback)( ws->window_id, bicgl_key, x, y, s_current_mods );
        }
        else if( (mods & GLFW_MOD_CONTROL) &&
                 key >= GLFW_KEY_A && key <= GLFW_KEY_Z )
        {
            /* Ctrl+letter — no char event is generated by GLFW */
            bicgl_key = key - GLFW_KEY_A + 1;
            if( key_down_callback )
                (*key_down_callback)( ws->window_id, bicgl_key, x, y, s_current_mods );
        }
        /* Printable keys handled by glfw_char_cb */
    }
    else if( action == GLFW_RELEASE )
    {
        if( translate_glfw_special( key, &bicgl_key ) )
        {
            if( key_up_callback )
                (*key_up_callback)( ws->window_id, bicgl_key, x, y, s_current_mods );
        }
        else if( (mods & GLFW_MOD_CONTROL) &&
                 key >= GLFW_KEY_A && key <= GLFW_KEY_Z )
        {
            bicgl_key = key - GLFW_KEY_A + 1;
            if( key_up_callback )
                (*key_up_callback)( ws->window_id, bicgl_key, x, y, s_current_mods );
        }
        else if( key > 0 && key < 128 )
        {
            /* Key-up for printable ASCII */
            if( key_up_callback )
                (*key_up_callback)( ws->window_id, key, x, y, s_current_mods );
        }
    }
}

static void glfw_char_cb( GLFWwindow *w, unsigned int codepoint )
{
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws ) return;
    /* Deliver printable key-down via the char callback (correctly composed) */
    int x = cursor_px( ws->dpi_scale_x, s_last_cursor_x );
    int y = flip_y( ws, cursor_px( ws->dpi_scale_y, s_last_cursor_y ) );
    if( codepoint < 256 && key_down_callback )
        (*key_down_callback)( ws->window_id, (int) codepoint, x, y, s_current_mods );
}

static void glfw_cursor_pos_cb( GLFWwindow *w, double xpos, double ypos )
{
    s_last_cursor_x = xpos;
    s_last_cursor_y = ypos;
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws || !mouse_motion_callback ) return;
    int px = cursor_px( ws->dpi_scale_x, xpos );
    int py = flip_y( ws, cursor_px( ws->dpi_scale_y, ypos ) );
    (*mouse_motion_callback)( ws->window_id, px, py );
}

static void glfw_mouse_button_cb( GLFWwindow *w, int button, int action, int mods )
{
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws ) return;
    Window_id wid = ws->window_id;
    s_current_mods = glfw_mods_to_bicgl( mods );
    int x = cursor_px( ws->dpi_scale_x, s_last_cursor_x );
    int y = flip_y( ws, cursor_px( ws->dpi_scale_y, s_last_cursor_y ) );

    if( action == GLFW_PRESS )
    {
        switch( button )
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            if( left_down_callback   ) (*left_down_callback)(  wid, x, y, s_current_mods ); break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            if( middle_down_callback ) (*middle_down_callback)(wid, x, y, s_current_mods ); break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if( right_down_callback  ) (*right_down_callback)( wid, x, y, s_current_mods ); break;
        }
    }
    else if( action == GLFW_RELEASE )
    {
        switch( button )
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            if( left_up_callback   ) (*left_up_callback)(  wid, x, y, s_current_mods ); break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            if( middle_up_callback ) (*middle_up_callback)(wid, x, y, s_current_mods ); break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if( right_up_callback  ) (*right_up_callback)( wid, x, y, s_current_mods ); break;
        }
    }
}

static void glfw_scroll_cb( GLFWwindow *w, double xoffset, double yoffset )
{
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws ) return;
    Window_id wid = ws->window_id;
    int x = cursor_px( ws->dpi_scale_x, s_last_cursor_x );
    int y = flip_y( ws, cursor_px( ws->dpi_scale_y, s_last_cursor_y ) );
    (void) xoffset;
    if( yoffset > 0.0 && scroll_up_callback   ) (*scroll_up_callback)(  wid, x, y, s_current_mods );
    if( yoffset < 0.0 && scroll_down_callback ) (*scroll_down_callback)(wid, x, y, s_current_mods );
}

/* Called when the content scale changes (e.g. window moved between monitors).
 * Since ws->width/height always come from glfwGetFramebufferSize (which
 * reflects the actual GL renderable size), a scale change only matters if the
 * framebuffer size also changes — which will be reported by
 * glfw_framebuffer_size_cb and glfw_window_size_cb.  Nothing to do here. */
static void glfw_content_scale_cb( GLFWwindow *w, float sx, float sy )
{
    (void) w; (void) sx; (void) sy;
}

static void glfw_window_size_cb( GLFWwindow *w, int width, int height )
{
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws ) return;
    /* Store the logical (window-coordinate) size.  Physical framebuffer
     * dimensions live in ws->width/height and are maintained exclusively
     * by glfw_framebuffer_size_cb.  Do NOT call glfwGetFramebufferSize()
     * here — on Wayland it returns physical pixels and feeding those back
     * as the logical size would start a resize feedback loop. */
    ws->logical_width  = width;
    ws->logical_height = height;
    /* Recompute dpi_scale from the (already stored) framebuffer size. */
    if( width  > 0 ) ws->dpi_scale_x = (float) ws->width  / width;
    if( height > 0 ) ws->dpi_scale_y = (float) ws->height / height;
}

static void glfw_framebuffer_size_cb( GLFWwindow *w, int fb_w, int fb_h )
{
    /* Authoritative physical pixel dimensions.  On native Wayland this
     * fires after glfwShowWindow with the true 2× framebuffer size; on
     * X11 (including XWayland) it fires with fb == logical (scale=1.0).
     * Always trust this value and fire the resize callback from here
     * so that the layout engine always uses the correct GL dimensions. */
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws ) return;
    ws->width  = fb_w;
    ws->height = fb_h;
    /* On native Wayland the fb callback fires BEFORE the window_size
     * callback, so ws->logical_width/height may still hold stale values.
     * Query the fresh logical size now to compute the correct DPI ratio. */
    if( s_is_native_wayland )
    {
        int log_w = 0, log_h = 0;
        glfwGetWindowSize( w, &log_w, &log_h );
        if( log_w > 0 ) ws->logical_width  = log_w;
        if( log_h > 0 ) ws->logical_height = log_h;
    }
    if( ws->logical_width  > 0 ) ws->dpi_scale_x = (float) fb_w / ws->logical_width;
    if( ws->logical_height > 0 ) ws->dpi_scale_y = (float) fb_h / ws->logical_height;
    if( resize_callback )
    {
        int xpos = 0, ypos = 0;
        glfwSetErrorCallback( NULL );
        glfwGetWindowPos( w, &xpos, &ypos );
        glfwSetErrorCallback( glfw_error_cb );
        (*resize_callback)( ws->window_id, xpos, ypos, fb_w, fb_h );
    }
}

static void glfw_refresh_cb( GLFWwindow *w )
{
    /* Window contents need redraw (e.g. uncovered after occlusion) */
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( ws ) ws->redisplay_pending = TRUE;
}

static void glfw_close_cb( GLFWwindow *w )
{
    /* Don't let GLFW auto-close; let register decide */
    glfwSetWindowShouldClose( w, GLFW_FALSE );
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws ) return;
    if( quit_callback )
        (*quit_callback)( ws->window_id );
    else
        s_quit_loop = TRUE;
}

static void glfw_iconify_cb( GLFWwindow *w, int iconified )
{
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws ) return;
    if( iconified )
    { if( iconify_callback   ) (*iconify_callback)(   ws->window_id ); }
    else
    { if( deiconify_callback ) (*deiconify_callback)( ws->window_id ); }
}

static void glfw_cursor_enter_cb( GLFWwindow *w, int entered )
{
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws ) return;
    if( entered )
    { if( enter_callback ) (*enter_callback)( ws->window_id ); }
    else
    { if( leave_callback ) (*leave_callback)( ws->window_id ); }
}

static void glfw_focus_cb( GLFWwindow *w, int focused )
{
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws ) return;
    if( focused )
    { if( enter_callback ) (*enter_callback)( ws->window_id ); }
    else
    { if( leave_callback ) (*leave_callback)( ws->window_id ); }
}

/* -----------------------------------------------------------------------
 * WS_initialize
 * --------------------------------------------------------------------- */

void  WS_initialize( void )
{
#if !defined(__APPLE__) && (GLFW_VERSION_MAJOR > 3 || (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 4))
    /* GLFW 3.4+: native Wayland path is broken; force X11 (XWayland).
     * Not applicable on macOS, which only has the native Cocoa platform. */
    glfwInitHint( GLFW_PLATFORM, GLFW_PLATFORM_X11 );
#endif
    glfwSetErrorCallback( glfw_error_cb );
    if( !glfwInit() )
        fprintf( stderr, "GLFW backend: glfwInit() failed.\n" );
}

VIO_BOOL  WS_is_native_wayland( void )
{
    return s_is_native_wayland;
}

/* -----------------------------------------------------------------------
 * WS_create_window
 * --------------------------------------------------------------------- */

VIO_Status  WS_create_window(
    VIO_STR  title,
    int      initial_x_pos,
    int      initial_y_pos,
    int      initial_x_size,
    int      initial_y_size,
    VIO_BOOL  colour_map_mode,
    VIO_BOOL  double_buffer_flag,
    VIO_BOOL  depth_buffer_flag,
    int      n_overlay_planes,
    VIO_BOOL  *actual_colour_map_mode,
    VIO_BOOL  *actual_double_buffer_flag,
    VIO_BOOL  *actual_depth_buffer_flag,
    int      *actual_n_overlay_planes,
    WSwindow  window,
    WSwindow  parent )
{
    (void) colour_map_mode;
    (void) double_buffer_flag;
    (void) depth_buffer_flag;
    (void) n_overlay_planes;
    (void) parent;   /* GLFW parent window not used; child windows are toplevel */

    if( initial_x_size <= 0 ) initial_x_size = 600;
    if( initial_y_size <= 0 ) initial_y_size = 600;
    if( initial_x_pos  <  0 ) initial_x_pos  = 0;
    if( initial_y_pos  <  0 ) initial_y_pos  = 0;

    glfwDefaultWindowHints();
    glfwWindowHint( GLFW_CLIENT_API,   GLFW_OPENGL_API );
    glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
    glfwWindowHint( GLFW_DEPTH_BITS,   16 );
    glfwWindowHint( GLFW_VISIBLE,      GLFW_FALSE );   /* shown after setup */

    /* Use the first window as the shared-context target for subsequent ones.
     * All windows must use the same context creation API as the first one;
     * mixing GLX and EGL contexts in a share group is not allowed and causes
     * GLFW error 65539 ("Context creation APIs do not match between contexts"). */
    GLFWwindow *share = s_first_glfw_win;

    if( s_context_api != 0 )
    {
        /* API already determined by the first window — use it directly. */
        if( s_context_api == GLFW_EGL_CONTEXT_API )
            glfwWindowHint( GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API );
    }

    /* On native Wayland the dimensions passed to glfwCreateWindow become the
     * compositor's "restore geometry" (used when un-maximising).  Clamp them
     * to the work area BEFORE window creation so un-maximize never produces a
     * window bigger than the screen.  This only works for 2nd+ windows since
     * glfwGetPrimaryMonitor() returns NULL before the first window exists. */
    if( s_is_native_wayland )
    {
        GLFWmonitor *mon = glfwGetPrimaryMonitor();
        if( mon )
        {
            int mx, my, mw, mh;
            glfwGetMonitorWorkarea( mon, &mx, &my, &mw, &mh );
            if( mw > 0 && initial_x_size > mw ) initial_x_size = mw;
            if( mh > 0 && initial_y_size > mh ) initial_y_size = mh;
        }
    }

    GLFWwindow *gw = glfwCreateWindow( initial_x_size, initial_y_size,
                                       title ? title : "", NULL, share );
    if( !gw && s_context_api == 0 )
    {
        /* First window: default API (GLX) failed — retry with EGL. */
        glfwWindowHint( GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API );
        gw = glfwCreateWindow( initial_x_size, initial_y_size,
                               title ? title : "", NULL, share );
        if( gw )
            s_context_api = GLFW_EGL_CONTEXT_API;
    }
    if( !gw )
    {
        const char *desc = NULL;
        glfwGetError( &desc );
        fprintf( stderr, "GLFW backend: glfwCreateWindow failed: %s\n",
                 desc ? desc : "unknown error" );
        return VIO_ERROR;
    }
    /* Record the API used by the first window so all later windows match. */
    if( s_context_api == 0 )
        s_context_api = GLFW_NATIVE_CONTEXT_API;

    /* First window: probe X11 display availability to detect native Wayland */
    if( !s_first_glfw_win )
    {
        s_first_glfw_win = gw;
#ifdef GLFW_EXPOSE_NATIVE_X11
        /* Suppress the "X11: Platform not initialized" error that GLFW emits
         * when running on a Wayland backend (glfwGetX11Display is a no-op
         * there).  The null-check + warning below already handles that case. */
        glfwSetErrorCallback( NULL );
        s_x11_display = glfwGetX11Display();
        glfwSetErrorCallback( glfw_error_cb );
        if( !s_x11_display )
        {
            fprintf( stderr, "GLFW backend: glfwGetX11Display() returned NULL "
                             "(Wayland session?).\n" );
            s_is_native_wayland = TRUE;
        }
#endif
    }

    /* Wayland does not allow apps to set window position; suppress the error */
    glfwSetErrorCallback( NULL );
    glfwSetWindowPos( gw, initial_x_pos, initial_y_pos );
    glfwSetErrorCallback( glfw_error_cb );

    /* Make the new window's context current */
    glfwMakeContextCurrent( gw );
    s_current_window = window;

    /* Get the X11 Window handle — used as Window_id throughout bicgl */
    Window_id x11_win = 0;
#ifdef GLFW_EXPOSE_NATIVE_X11
    /* Suppress the "X11: Platform not initialized" error on Wayland */
    glfwSetErrorCallback( NULL );
    x11_win = glfwGetX11Window( gw );
    glfwSetErrorCallback( glfw_error_cb );
#endif
    if( x11_win == 0 )
    {
        /* Wayland, macOS/Cocoa, or other non-X11 backend: synthesise a
         * unique ID from the GLFWwindow* pointer. */
        x11_win = (Window_id)(size_t) gw;
    }

    /* Fill in WSwindow fields */
    window->glfw              = gw;
    window->window_id         = x11_win;
    /* Compute physical pixel dimensions.
     * On XWayland: use content_scale * logical (compositor handles scaling).
     * On native X11: use glfwGetFramebufferSize (== logical, scale=1.0). */
    update_window_scale( window, gw, initial_x_size, initial_y_size );
    window->is_visible        = TRUE;
    window->redisplay_pending = TRUE;
    window->init_x            = initial_x_pos;
    window->init_y            = initial_y_pos;
    window->border_width      = 0;
    window->border_height     = 0;
    window->is_new            = TRUE;

    /* Register GLFW event callbacks */
    glfwSetWindowUserPointer(    gw, window );
    glfwSetKeyCallback(          gw, glfw_key_cb );
    glfwSetCharCallback(         gw, glfw_char_cb );
    glfwSetCursorPosCallback(    gw, glfw_cursor_pos_cb );
    glfwSetMouseButtonCallback(  gw, glfw_mouse_button_cb );
    glfwSetScrollCallback(       gw, glfw_scroll_cb );
    glfwSetWindowSizeCallback(         gw, glfw_window_size_cb );
    glfwSetWindowContentScaleCallback( gw, glfw_content_scale_cb );
    glfwSetFramebufferSizeCallback(    gw, glfw_framebuffer_size_cb );
    glfwSetWindowRefreshCallback(      gw, glfw_refresh_cb );
    glfwSetWindowCloseCallback(        gw, glfw_close_cb );
    glfwSetWindowIconifyCallback(      gw, glfw_iconify_cb );
    glfwSetCursorEnterCallback(        gw, glfw_cursor_enter_cb );
    glfwSetWindowFocusCallback(        gw, glfw_focus_cb );

    register_window( gw, window );

    /* Clamp window size to the monitor work area.  This must happen after
     * glfwCreateWindow because on Wayland the monitor list is not populated
     * until the first window is created (wl_registry roundtrip).  Clamping
     * before glfwShowWindow ensures the compositor's "restore" geometry
     * (used when un-maximising) fits on screen. */
    int  clamped_x = 0, clamped_y = 0;
    VIO_BOOL  did_clamp = FALSE;
    {
        GLFWmonitor *mon = glfwGetPrimaryMonitor();
        if( mon )
        {
            int mx, my, mw, mh;
            glfwGetMonitorWorkarea( mon, &mx, &my, &mw, &mh );
            if( mw > 0 && mh > 0 &&
                ( initial_x_size > mw || initial_y_size > mh ) )
            {
                if( initial_x_size > mw ) initial_x_size = mw;
                if( initial_y_size > mh ) initial_y_size = mh;
                glfwSetWindowSize( gw, initial_x_size, initial_y_size );
                update_window_scale( window, gw, initial_x_size,
                                     initial_y_size );
                clamped_x = initial_x_size;
                clamped_y = initial_y_size;
                did_clamp = TRUE;
            }
        }
    }

    glfwShowWindow( gw );

    /* On X11, glfwSetWindowSize on a hidden window may be ignored, so
     * re-apply the clamped size after the window is mapped.  On native
     * Wayland skip this — the pre-creation clamp already set the correct
     * restore geometry, and calling glfwSetWindowSize after map fights
     * with the compositor's configure events. */
    if( did_clamp && !s_is_native_wayland )
    {
        glfwPollEvents();   /* complete xdg_surface configure round-trip */
        glfwSetWindowSize( gw, clamped_x, clamped_y );
    }

    /* glfwGetWindowContentScale() returns the correct scale both before and
     * after glfwShowWindow, so no re-query is needed here.  Async scale
     * changes (e.g. window moved to a different DPI monitor) are handled by
     * glfw_content_scale_cb. */

    if( actual_colour_map_mode    ) *actual_colour_map_mode    = FALSE;
    if( actual_double_buffer_flag ) *actual_double_buffer_flag = TRUE;
    if( actual_depth_buffer_flag  ) *actual_depth_buffer_flag  = TRUE;
    if( actual_n_overlay_planes   ) *actual_n_overlay_planes   = 0;

    return VIO_OK;
}

/* -----------------------------------------------------------------------
 * Window management
 * --------------------------------------------------------------------- */

VIO_BOOL  WS_set_double_buffer_state( WSwindow window, VIO_BOOL flag )
{
    (void)window;
    return flag;
}

VIO_BOOL  WS_set_colour_map_state( WSwindow window, VIO_BOOL flag )
{
    (void)window;
    return flag;
}

void  WS_set_window_title( WSwindow window, VIO_STR title )
{
    if( window && window->glfw )
        glfwSetWindowTitle( window->glfw, title ? title : "" );
}

void  WS_delete_window( WSwindow window )
{
    if( !window ) return;

    if( s_current_window == window )
    {
        glfwMakeContextCurrent( NULL );
        s_current_window = NULL;
    }

    if( window->glfw )
    {
        unregister_window( window->glfw );
        glfwDestroyWindow( window->glfw );
        window->glfw = NULL;
    }
    window->window_id = 0;
}

VIO_BOOL  WS_window_has_overlay_planes( WSwindow window )
{
    (void)window;
    return FALSE;
}

void  WS_set_current_window( WSwindow window )
{
    WS_set_bitplanes( window, NORMAL_PLANES );
}

Window_id  WS_get_window_id( WSwindow window )
{
    return window->window_id;
}

void  WS_set_bitplanes( WSwindow window, Bitplane_types bitplanes )
{
    (void)bitplanes;
    if( !window || !window->glfw ) return;
    if( s_current_window == window ) return;
    glfwMakeContextCurrent( window->glfw );
    s_current_window = window;
}

int  WS_get_n_overlay_planes( void )
{
    return 0;
}

void  WS_get_window_position( int *x_pos, int *y_pos )
{
    if( s_current_window && s_current_window->glfw )
    {
        /* Wayland does not provide window position; suppress the error */
        glfwSetErrorCallback( NULL );
        glfwGetWindowPos( s_current_window->glfw, x_pos, y_pos );
        glfwSetErrorCallback( glfw_error_cb );
    }
    else
    {
        *x_pos = 0;
        *y_pos = 0;
    }
}

void  WS_get_window_size( int *x_size, int *y_size )
{
    if( s_current_window )
    {
        *x_size = s_current_window->width;
        *y_size = s_current_window->height;
    }
    else
    {
        *x_size = 0;
        *y_size = 0;
    }
}

/* Integer backing-store scale factor of the current window (1 on non-HiDPI
 * / X11, 2 on a Retina Mac).  Applications lay out their fixed-pixel UI
 * geometry in framebuffer pixels (see WS_get_window_size), so they use this
 * to scale those constants to match the DPI-scaled fonts. */
int  WS_get_window_content_scale( void )
{
    int scale;

    if( !s_current_window )
        return 1;

    scale = (int)( s_current_window->dpi_scale_y + 0.5f );
    return ( scale < 1 ) ? 1 : scale;
}

void  WS_set_colour_map_entry( WSwindow window, Bitplane_types bp, int ind, VIO_Colour col )
{
    (void)window; (void)bp; (void)ind; (void)col;
}

void  WS_set_overlay_colour_map_entry( WSwindow window, int ind, VIO_Colour col )
{
    (void)window; (void)ind; (void)col;
}

void  WS_swap_buffers( void )
{
    if( s_current_window && s_current_window->glfw )
    {
        glfwSwapBuffers( s_current_window->glfw );
    }
}

/* -----------------------------------------------------------------------
 * Font / text — shared stb_truetype pipeline (font_atlas + font_render_gl)
 *
 * One process-wide FontAtlasCache, built from the embedded JetBrains Mono
 * bytes. Cache key is the physical (framebuffer) pixel_height the atlas
 * should be baked at -- requested "size" scaled by the window's current
 * dpi_scale_y, continuously (not rounded to an integer scale the way the
 * old bitmap-upscaling code had to). This is what actually fixes the
 * Retina blur: the glyphs are rasterised natively at the display's real
 * pixel density instead of being upscaled after the fact.
 * --------------------------------------------------------------------- */

static FontAtlasCache *s_font_atlas_cache = NULL;

static FontAtlasCache *get_font_atlas_cache( void )
{
    if( !s_font_atlas_cache )
        s_font_atlas_cache = font_atlas_cache_create(
            jetbrains_mono_ttf, jetbrains_mono_ttf_len, 8 );
    return s_font_atlas_cache;
}

static FontAtlas *get_atlas_for( Font_types type, VIO_Real size )
{
    float dpi_scale = s_current_window ? s_current_window->dpi_scale_y : 1.0f;
    (void) type;

    float pixel_height = font_render_gl_pixel_height( (float) size, dpi_scale );
    return font_atlas_cache_get( get_font_atlas_cache(), pixel_height );
}

void  WS_draw_text( Font_types type, VIO_Real size, VIO_STR string )
{
    if( !string || !s_current_window ) return;

    FontAtlas *atlas = get_atlas_for( type, size );
    if( !atlas ) return;

    font_render_gl_draw_text( atlas, (const char *) string );
}

VIO_Real  WS_get_character_height( Font_types type, VIO_Real size )
{
    if( !s_current_window )
        return (VIO_Real) font_render_gl_pixel_height( (float) size, 1.0f );

    FontAtlas *atlas = get_atlas_for( type, size );
    if( !atlas )
        return (VIO_Real) font_render_gl_pixel_height( (float) size, 1.0f );

    return (VIO_Real) atlas->ascent;
}

VIO_Real  WS_get_text_length( VIO_STR str, Font_types type, VIO_Real size )
{
    if( !str ) return 0.0;

    if( !s_current_window )
        return (VIO_Real) strlen(str) * font_render_gl_pixel_height( (float) size, 1.0f ) * 0.6;

    FontAtlas *atlas = get_atlas_for( type, size );
    if( !atlas )
        return (VIO_Real) strlen(str) * font_render_gl_pixel_height( (float) size, 1.0f ) * 0.6;

    return (VIO_Real) strlen(str) * (VIO_Real) atlas->advance_width;
}

/* -----------------------------------------------------------------------
 * Screen size
 * --------------------------------------------------------------------- */

void  WS_get_screen_size( int *x_size, int *y_size )
{
    GLFWmonitor *mon = glfwGetPrimaryMonitor();
    if( mon )
    {
        int mx, my, mw, mh;
        glfwGetMonitorWorkarea( mon, &mx, &my, &mw, &mh );
        /* glfwGetMonitorWorkarea returns logical pixels; scale to physical. */
        float sx = 1.0f, sy = 1.0f;
        glfwGetMonitorContentScale( mon, &sx, &sy );
        *x_size = (int)( mw * sx );
        *y_size = (int)( mh * sy );
    }
    else
    {
        *x_size = 1024;
        *y_size = 768;
    }
}

void  WS_set_mouse_position( int x_screen, int y_screen )
{
    (void)x_screen; (void)y_screen;
}

/* -----------------------------------------------------------------------
 * Callback registration
 * --------------------------------------------------------------------- */

void WS_set_update_function(         void (*f)(Window_id) )                 { display_callback         = f; }
void WS_set_update_overlay_function( void (*f)(Window_id) )                 { display_overlay_callback = f; }
void WS_set_resize_function(         void (*f)(Window_id,int,int,int,int) ) { resize_callback          = f; }
void WS_set_key_down_function(       void (*f)(Window_id,int,int,int,int) ) { key_down_callback        = f; }
void WS_set_key_up_function(         void (*f)(Window_id,int,int,int,int) ) { key_up_callback          = f; }
void WS_set_mouse_movement_function( void (*f)(Window_id,int,int) )         { mouse_motion_callback    = f; }
void WS_set_left_mouse_down_function(  void (*f)(Window_id,int,int,int) )   { left_down_callback       = f; }
void WS_set_left_mouse_up_function(    void (*f)(Window_id,int,int,int) )   { left_up_callback         = f; }
void WS_set_middle_mouse_down_function(void (*f)(Window_id,int,int,int) )   { middle_down_callback     = f; }
void WS_set_middle_mouse_up_function(  void (*f)(Window_id,int,int,int) )   { middle_up_callback       = f; }
void WS_set_right_mouse_down_function( void (*f)(Window_id,int,int,int) )   { right_down_callback      = f; }
void WS_set_right_mouse_up_function(   void (*f)(Window_id,int,int,int) )   { right_up_callback        = f; }
void WS_set_scroll_up_function(        void (*f)(Window_id,int,int,int) )   { scroll_up_callback       = f; }
void WS_set_scroll_down_function(      void (*f)(Window_id,int,int,int) )   { scroll_down_callback     = f; }
void WS_set_iconify_function(          void (*f)(Window_id) )               { iconify_callback         = f; }
void WS_set_deiconify_function(        void (*f)(Window_id) )               { deiconify_callback       = f; }
void WS_set_enter_function(            void (*f)(Window_id) )               { enter_callback           = f; }
void WS_set_leave_function(            void (*f)(Window_id) )               { leave_callback           = f; }
void WS_set_quit_function(             void (*f)(Window_id) )               { quit_callback            = f; }

/* -----------------------------------------------------------------------
 * Timer and idle management
 * --------------------------------------------------------------------- */

void  WS_add_timer_function( VIO_Real seconds, void (*func)(void*), void *data )
{
    int i;
    timer_entry *te;

    for( i = 0; i < s_n_timers; ++i )
        if( !s_timers[i].active ) break;

    if( i >= s_n_timers )
    {
        SET_ARRAY_SIZE( s_timers, s_n_timers, s_n_timers + 1, 1 );
        ++s_n_timers;
    }

    te = &s_timers[i];
    gettimeofday( &te->fire_time, NULL );
    timeval_add_seconds( &te->fire_time, seconds );
    te->func   = func;
    te->data   = data;
    te->active = TRUE;
}

void  WS_add_idle_function( void (*func)(void*), void *data )
{
    idle_entry e;
    e.func = func;
    e.data = data;
    ADD_ELEMENT_TO_ARRAY( s_idles, s_n_idles, e, 1 );
}

void  WS_remove_idle_function( void (*func)(void*), void *data )
{
    int i;
    for( i = 0; i < s_n_idles; ++i )
        if( s_idles[i].func == func && s_idles[i].data == data ) break;

    if( i < s_n_idles )
        DELETE_ELEMENT_FROM_ARRAY( s_idles, s_n_idles, i, 1 );
}

/* -----------------------------------------------------------------------
 * Fire expired timers (two-pass to handle callbacks that re-register).
 *
 * Problem: timer_function() deactivates itself then re-registers into the
 * same slot.  A combined fire+scan loop misses the re-registered entry,
 * returns -1, and the event loop blocks indefinitely.
 *
 * Solution: first pass fires all expired timers; second pass (after all
 * callbacks have run) scans for the soonest upcoming deadline.
 * --------------------------------------------------------------------- */

static void fire_timers( void )
{
    int i;
    int n = s_n_timers;   /* snapshot before firing — callbacks may grow array */
    for( i = 0; i < n; ++i )
    {
        if( !s_timers[i].active ) continue;
        if( timeval_usec_until( &s_timers[i].fire_time ) <= 0 )
        {
            s_timers[i].active = FALSE;
            (*s_timers[i].func)( s_timers[i].data );
        }
    }
}

/* Return microseconds until the soonest active timer, or -1 if none. */
static long next_timer_usec( void )
{
    int  i;
    long min_usec = -1L;
    for( i = 0; i < s_n_timers; ++i )
    {
        if( !s_timers[i].active ) continue;
        long usec = timeval_usec_until( &s_timers[i].fire_time );
        if( usec > 0 && (min_usec < 0 || usec < min_usec) )
            min_usec = usec;
    }
    return min_usec;
}

/* -----------------------------------------------------------------------
 * Fire pending redraws
 * --------------------------------------------------------------------- */

static void fire_redraws( void )
{
    int i;
    for( i = 0; i < s_n_windows; ++i )
    {
        WSwindow ws = s_windows[i].ws;
        if( ws && ws->redisplay_pending && ws->is_visible )
        {
            ws->redisplay_pending = FALSE;
            WS_set_bitplanes( ws, NORMAL_PLANES );
            if( display_callback )
                (*display_callback)( ws->window_id );
        }
    }
}

/* -----------------------------------------------------------------------
 * WS_event_loop — GLFW poll/wait loop
 * --------------------------------------------------------------------- */

void  WS_event_loop( void )
{
    s_quit_loop = FALSE;

    /* Sync all windows to their true framebuffer size.  During window
     * creation (WS_create_window → glfwShowWindow), the Wayland compositor
     * may reconfigure already-created windows, but resize_callback is not
     * yet set at that point (it is set by initialize_callbacks, which runs
     * just before WS_event_loop).  Re-query each window now and fire the
     * resize callback so that the layout engine sees the correct size. */
    if( resize_callback )
    {
        int i;
        for( i = 0; i < s_n_windows; ++i )
        {
            WSwindow ws = s_windows[i].ws;
            GLFWwindow *gw = s_windows[i].glfw;
            if( !ws || !gw ) continue;
            int fb_w = 0, fb_h = 0;
            int log_w = 0, log_h = 0;
            glfwGetFramebufferSize( gw, &fb_w, &fb_h );
            glfwGetWindowSize( gw, &log_w, &log_h );
            ws->width          = fb_w;
            ws->height         = fb_h;
            ws->logical_width  = log_w;
            ws->logical_height = log_h;
            ws->dpi_scale_x    = ( log_w > 0 ) ? (float)fb_w / log_w : 1.0f;
            ws->dpi_scale_y    = ( log_h > 0 ) ? (float)fb_h / log_h : 1.0f;
            {
                int xpos = 0, ypos = 0;
                glfwSetErrorCallback( NULL );
                glfwGetWindowPos( gw, &xpos, &ypos );
                glfwSetErrorCallback( glfw_error_cb );
                (*resize_callback)( ws->window_id, xpos, ypos, fb_w, fb_h );
            }
        }
    }

    while( !s_quit_loop )
    {
        /* 1. Dispatch all pending GLFW/X11 events via registered callbacks */
        glfwPollEvents();
        if( s_quit_loop ) break;

        /* 2. Call idle functions */
        if( s_n_idles > 0 )
        {
            int i;
            for( i = 0; i < s_n_idles; ++i )
                (*s_idles[i].func)( s_idles[i].data );
        }

        /* 3. Fire expired timers */
        fire_timers();

        /* 4. Fire pending redraws */
        fire_redraws();

        /* 5. Block until next event or next timer, whichever comes first.
              Use zero timeout (keep looping) when idle functions are active
              or any window has a pending redraw. */
        if( !s_quit_loop )
        {
            VIO_BOOL any_pending = FALSE;
            int pi;
            for( pi = 0; pi < s_n_windows; ++pi )
                if( s_windows[pi].ws && s_windows[pi].ws->redisplay_pending )
                    { any_pending = TRUE; break; }

            if( s_n_idles > 0 || any_pending )
            {
                /* Busy — glfwPollEvents() at the top of the loop already
                 * yields to the OS, so we don't busy-spin at 100% CPU. */
            }
            else
            {
                long deadline_usec = next_timer_usec();
                if( deadline_usec > 0 )
                    glfwWaitEventsTimeout( (double)deadline_usec / 1e6 );
                else
                    glfwWaitEvents();
            }
        }
    }
}

void  WS_exit_loop( void )
{
    s_quit_loop = TRUE;
    glfwPostEmptyEvent();   /* wake up glfwWaitEvents() if currently blocking */
}

/* -----------------------------------------------------------------------
 * Redisplay / visibility / geometry
 * --------------------------------------------------------------------- */

void  WS_set_update_flag( WSwindow window )
{
    if( window )
    {
        window->redisplay_pending = TRUE;
        glfwPostEmptyEvent();   /* wake glfwWaitEvents() if blocking */
    }
}

void  WS_set_visibility( WSwindow window, VIO_BOOL is_visible )
{
    if( !window ) return;
    window->is_visible = is_visible;
    if( window->glfw )
    {
        if( is_visible )
            glfwShowWindow( window->glfw );
        else
            glfwHideWindow( window->glfw );
    }
}

void  WS_set_geometry( WSwindow window, int x, int y, int cx, int cy )
{
    if( !window || !window->glfw ) return;

    if( x >= 0 && y >= 0 )
    {
        /* Wayland does not allow apps to set window position; suppress the error */
        glfwSetErrorCallback( NULL );
        glfwSetWindowPos( window->glfw, x, y );
        glfwSetErrorCallback( glfw_error_cb );
    }
    if( cx > 0 && cy > 0 )
    {
        glfwSetWindowSize( window->glfw, cx, cy );
        update_window_scale( window, window->glfw, cx, cy );
    }
}
