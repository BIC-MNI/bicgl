/*
 * egl_windows.c — GLFW+EGL windowing backend for bicgl.
 *
 * Uses GLFW to create windows and OpenGL contexts (via EGL on x2go/X11).
 * Falls back to GLFW_EGL_CONTEXT_API when the default GLX path fails,
 * which is exactly the x2go case (no GLX extension on the X server).
 *
 * Font rendering uses stored_font.c (pre-rasterised 8×13 bitmap font)
 * plus an optional compact X11 system font loaded via Xlib — no GLX
 * required.  The X11 display handle is obtained from GLFW after the first
 * window is created via glfwGetX11Display().
 *
 * This file implements the full WS_* interface declared in
 * EGL_windows/Include/egl_window_prototypes.h.
 */

/* Include GLFW native header FIRST, before any X11 headers that do
 * #undef Status (WS_graphics.h).  glfw3native.h pulls in Xrandr.h which
 * uses Status as a function return type; if Status is already undef-ed,
 * compilation fails.  Including it here keeps Status intact. */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>  /* glfwGetX11Display(), glfwGetX11Window() */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <volume_io.h>
#include <WS_graphics.h>       /* does #undef Status — must come after glfw3native.h */

/* OpenGL */
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

/* X11 — for font loading only (Xlib.h / Xutil.h already pulled in above) */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

/* Forward declaration of stored_font functions */
void     create_fixed_font( GLuint fontOffset );
void     create_sized_font( GLuint fontOffset );
int      get_fixed_font_n_chars( void );
VIO_Real get_fixed_font_height( void );
VIO_Real get_fixed_font_width( char ch );

/* Forward declaration — defined in the font section below */
static VIO_BOOL load_x11_font_glists( GLuint list_base );

/* -----------------------------------------------------------------------
 * GLFW globals
 * --------------------------------------------------------------------- */

static Display    *s_x11_display   = NULL;  /* from glfwGetX11Display()  */
static GLFWwindow *s_first_glfw_win = NULL; /* share target for 2nd+ wins */

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
    int x = (int) s_last_cursor_x;
    int y = flip_y( ws, (int) s_last_cursor_y );

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
    if( codepoint < 256 && key_down_callback )
        (*key_down_callback)( ws->window_id, (int) codepoint, 0, 0, 0 );
}

static void glfw_cursor_pos_cb( GLFWwindow *w, double xpos, double ypos )
{
    s_last_cursor_x = xpos;
    s_last_cursor_y = ypos;
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws || !mouse_motion_callback ) return;
    (*mouse_motion_callback)( ws->window_id, (int) xpos, flip_y( ws, (int) ypos ) );
}

static void glfw_mouse_button_cb( GLFWwindow *w, int button, int action, int mods )
{
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws ) return;
    Window_id wid = ws->window_id;
    s_current_mods = glfw_mods_to_bicgl( mods );
    int x = (int) s_last_cursor_x;
    int y = flip_y( ws, (int) s_last_cursor_y );

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
    int x = (int) s_last_cursor_x;
    int y = flip_y( ws, (int) s_last_cursor_y );
    (void) xoffset;
    if( yoffset > 0.0 && scroll_up_callback   ) (*scroll_up_callback)(  wid, x, y, s_current_mods );
    if( yoffset < 0.0 && scroll_down_callback ) (*scroll_down_callback)(wid, x, y, s_current_mods );
}

static void glfw_window_size_cb( GLFWwindow *w, int width, int height )
{
    WSwindow ws = (WSwindow) glfwGetWindowUserPointer( w );
    if( !ws ) return;
    ws->width  = width;
    ws->height = height;
    if( resize_callback )
    {
        int xpos = 0, ypos = 0;
        glfwGetWindowPos( w, &xpos, &ypos );
        (*resize_callback)( ws->window_id, xpos, ypos, width, height );
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
    glfwSetErrorCallback( glfw_error_cb );
    if( !glfwInit() )
        fprintf( stderr, "GLFW backend: glfwInit() failed.\n" );
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

    /* Use the first window as the shared-context target for subsequent ones */
    GLFWwindow *share = s_first_glfw_win;

    /* Try the default context API first (GLX on X11 — uses hardware if available).
     * Fall back to EGL when GLX fails (x2go, SSH X11 forwarding, no DRI3). */
    GLFWwindow *gw = glfwCreateWindow( initial_x_size, initial_y_size,
                                       title ? title : "", NULL, share );
    if( !gw )
    {
        glfwWindowHint( GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API );
        gw = glfwCreateWindow( initial_x_size, initial_y_size,
                               title ? title : "", NULL, share );
    }
    if( !gw )
    {
        const char *desc = NULL;
        glfwGetError( &desc );
        fprintf( stderr, "GLFW backend: glfwCreateWindow failed: %s\n",
                 desc ? desc : "unknown error" );
        return VIO_ERROR;
    }

    /* First window: cache X11 display handle for font loading */
    if( !s_first_glfw_win )
    {
        s_first_glfw_win = gw;
#ifdef GLFW_EXPOSE_NATIVE_X11
        s_x11_display = glfwGetX11Display();
        if( !s_x11_display )
            fprintf( stderr, "GLFW backend: glfwGetX11Display() returned NULL "
                             "(Wayland session?); font loading will use fallback.\n" );
#endif
    }

    glfwSetWindowPos( gw, initial_x_pos, initial_y_pos );

    /* Make the new window's context current */
    glfwMakeContextCurrent( gw );
    s_current_window = window;

    /* Get the X11 Window handle — used as Window_id throughout bicgl */
    Window x11_win = 0;
#ifdef GLFW_EXPOSE_NATIVE_X11
    x11_win = glfwGetX11Window( gw );
#endif
    if( x11_win == 0 )
    {
        /* Wayland or other non-X11 backend: synthesise a unique ID */
        x11_win = (Window)(size_t) gw;
    }

    /* Fill in WSwindow fields */
    window->glfw              = gw;
    window->window_id         = x11_win;
    window->width             = initial_x_size;
    window->height            = initial_y_size;
    window->is_visible        = TRUE;
    window->redisplay_pending = TRUE;
    window->init_x            = initial_x_pos;
    window->init_y            = initial_y_pos;
    window->border_width      = 0;
    window->border_height     = 0;
    window->is_new            = TRUE;

    /* Load font display lists (context must be current) */
    window->font_list_base = (int) glGenLists( 128 );
    create_fixed_font( (GLuint) window->font_list_base );

    window->font_list_base_sized = (int) glGenLists( 128 );
    if( !load_x11_font_glists( (GLuint) window->font_list_base_sized ) )
        create_sized_font( (GLuint) window->font_list_base_sized );

    /* Register GLFW event callbacks */
    glfwSetWindowUserPointer(    gw, window );
    glfwSetKeyCallback(          gw, glfw_key_cb );
    glfwSetCharCallback(         gw, glfw_char_cb );
    glfwSetCursorPosCallback(    gw, glfw_cursor_pos_cb );
    glfwSetMouseButtonCallback(  gw, glfw_mouse_button_cb );
    glfwSetScrollCallback(       gw, glfw_scroll_cb );
    glfwSetWindowSizeCallback(   gw, glfw_window_size_cb );
    glfwSetWindowRefreshCallback(gw, glfw_refresh_cb );
    glfwSetWindowCloseCallback(  gw, glfw_close_cb );
    glfwSetWindowIconifyCallback(gw, glfw_iconify_cb );
    glfwSetCursorEnterCallback(  gw, glfw_cursor_enter_cb );
    glfwSetWindowFocusCallback(  gw, glfw_focus_cb );

    register_window( gw, window );

    glfwShowWindow( gw );

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
        glfwGetWindowPos( s_current_window->glfw, x_pos, y_pos );
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
        glfwSwapBuffers( s_current_window->glfw );
}

/* -----------------------------------------------------------------------
 * X11 system font → OpenGL display-list loader (SIZED_FONT path)
 *
 * Rasterises each ASCII glyph to an X11 Pixmap, reads the pixels back
 * with XGetImage, and uploads them as GL display lists via glBitmap.
 * The X11 display is obtained from GLFW via glfwGetX11Display().
 *
 * Returns TRUE on success and sets s_sized_font_advance / _height.
 * Falls back to create_sized_font() (downsampled stored font) on failure.
 * --------------------------------------------------------------------- */

static float s_sized_font_advance = 7.0f;
static float s_sized_font_height  = 10.0f;

static VIO_BOOL load_x11_font_glists( GLuint list_base )
{
    /* Preferred fonts, tried in order.  6x10 / 6x12 give a good balance
     * between readability and fitting within the register button width. */
    static const char *candidates[] = {
        "6x10",
        "-misc-fixed-medium-r-normal--10-100-75-75-c-60-iso8859-1",
        "6x12",
        "-misc-fixed-medium-r-normal--12-120-75-75-c-70-iso8859-1",
        "5x8",
        "-misc-fixed-medium-r-normal--8-80-75-75-c-50-iso8859-1",
        "5x7",
        "-misc-fixed-medium-r-normal--7-70-75-75-c-50-iso8859-1",
        NULL
    };

    if( !s_x11_display )
        return FALSE;

    XFontStruct *fs = NULL;
    int fi;
    for( fi = 0; candidates[fi]; ++fi )
    {
        fs = XLoadQueryFont( s_x11_display, candidates[fi] );
        if( fs ) break;
    }
    if( !fs )
    {
        fprintf( stderr, "GLFW backend: no compact X11 font found, "
                         "using downsampled fallback.\n" );
        return FALSE;
    }

    int fwidth  = fs->max_bounds.width;
    int fheight = fs->ascent + fs->descent;
    int fascent = fs->ascent;
    int screen  = DefaultScreen( s_x11_display );
    int stride  = ( fwidth + 7 ) / 8;

    Pixmap pix = XCreatePixmap( s_x11_display,
                                RootWindow( s_x11_display, screen ),
                                (unsigned) fwidth, (unsigned) fheight,
                                (unsigned) DefaultDepth( s_x11_display, screen ) );
    GC gc = XCreateGC( s_x11_display, pix, 0, NULL );
    XSetFont( s_x11_display, gc, fs->fid );

    unsigned long black = BlackPixel( s_x11_display, screen );
    unsigned long white = WhitePixel( s_x11_display, screen );

    GLubyte *bits = (GLubyte *) malloc( (size_t)( stride * fheight ) );
    if( !bits )
    {
        XFreeGC( s_x11_display, gc );
        XFreePixmap( s_x11_display, pix );
        XFreeFont( s_x11_display, fs );
        return FALSE;
    }

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    int c;
    for( c = 32; c < 127; ++c )
    {
        char ch = (char) c;

        XSetForeground( s_x11_display, gc, black );
        XFillRectangle( s_x11_display, pix, gc, 0, 0,
                        (unsigned) fwidth, (unsigned) fheight );
        XSetForeground( s_x11_display, gc, white );
        XDrawString( s_x11_display, pix, gc, 0, fascent, &ch, 1 );

        XImage *img = XGetImage( s_x11_display, pix, 0, 0,
                                 (unsigned) fwidth, (unsigned) fheight,
                                 AllPlanes, ZPixmap );

        memset( bits, 0, (size_t)( stride * fheight ) );
        int row, col;
        for( row = 0; row < fheight; ++row )
        {
            int gl_row = fheight - 1 - row;
            for( col = 0; col < fwidth; ++col )
            {
                if( XGetPixel( img, col, row ) != black )
                    bits[ gl_row * stride + col / 8 ] |=
                        (GLubyte)( 0x80u >> ( col % 8 ) );
            }
        }
        XDestroyImage( img );

        float advance = (float) fwidth;
        if( fs->per_char )
        {
            int idx = c - (int) fs->min_char_or_byte2;
            if( idx >= 0 && c <= (int) fs->max_char_or_byte2 )
            {
                int w = fs->per_char[idx].width;
                if( w > 0 ) advance = (float) w;
            }
        }

        glNewList( (GLuint) c + list_base, GL_COMPILE );
        glBitmap( (GLsizei) fwidth, (GLsizei) fheight,
                  0.0f, (float) fs->descent,
                  advance, 0.0f,
                  bits );
        glEndList();
    }

    free( bits );
    XFreeGC( s_x11_display, gc );
    XFreePixmap( s_x11_display, pix );

    s_sized_font_advance = (float) fwidth;
    s_sized_font_height  = (float) fheight;

    XFreeFont( s_x11_display, fs );
    return TRUE;
}

/* -----------------------------------------------------------------------
 * Font / text — uses stored_font.c display lists
 * --------------------------------------------------------------------- */

void  WS_draw_text( Font_types type, VIO_Real size, VIO_STR string )
{
    (void)size;
    if( !string || !s_current_window ) return;

    if( type == SIZED_FONT )
        glListBase( (GLuint) s_current_window->font_list_base_sized );
    else
        glListBase( (GLuint) s_current_window->font_list_base );

    glCallLists( (GLsizei) strlen(string), GL_UNSIGNED_BYTE,
                 (const GLubyte *) string );
}

VIO_Real  WS_get_character_height( Font_types type, VIO_Real size )
{
    if( type == SIZED_FONT )
        return size;
    return get_fixed_font_height();
}

VIO_Real  WS_get_text_length( VIO_STR str, Font_types type, VIO_Real size )
{
    (void)size;
    if( !str ) return 0.0;

    if( type == SIZED_FONT )
        return (VIO_Real) strlen(str) * (VIO_Real) s_sized_font_advance;
    else
        return (VIO_Real) strlen(str) * get_fixed_font_width( str[0] );
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
        *x_size = mw;
        *y_size = mh;
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
        window->redisplay_pending = TRUE;
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
        glfwSetWindowPos( window->glfw, x, y );
    if( cx > 0 && cy > 0 )
    {
        glfwSetWindowSize( window->glfw, cx, cy );
        window->width  = cx;
        window->height = cy;
    }
}
