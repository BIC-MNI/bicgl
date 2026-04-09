/*
 * egl_windows.c — EGL + X11 windowing backend for bicgl.
 *
 * Creates X11 windows for display and event handling (works on any X11
 * server, including x2go which has no GLX extension).  OpenGL contexts
 * are created via EGL, which does not require the GLX X11 extension and
 * works with Mesa's software (llvmpipe) renderer on x2go.
 *
 * Font rendering uses stored_font.c (pre-rasterised 8×13 bitmap font
 * uploaded as OpenGL display lists) — no glXUseXFont() required.
 *
 * This file implements the full WS_* interface declared in
 * EGL_windows/Include/egl_window_prototypes.h.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <volume_io.h>
#include <WS_graphics.h>

/* EGL + OpenGL headers */
#include <EGL/egl.h>
#include <EGL/eglext.h>    /* EGL_PLATFORM_X11_KHR */
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

/* X11 headers */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>

/* Forward declaration of stored_font functions */
void    create_fixed_font( GLuint fontOffset );
void    create_sized_font( GLuint fontOffset );
int     get_fixed_font_n_chars( void );
VIO_Real get_fixed_font_height( void );
VIO_Real get_fixed_font_width( char ch );

/* -----------------------------------------------------------------------
 * EGL globals (one EGL display, shared across all windows)
 * --------------------------------------------------------------------- */

static Display    *s_display   = NULL;
static EGLDisplay  s_egl_dpy   = EGL_NO_DISPLAY;
static EGLConfig   s_egl_cfg   = 0;
static EGLContext  s_shared_ctx = EGL_NO_CONTEXT;  /* shared GL context  */
static int         s_screen    = 0;
static Atom        s_wm_delete_window = None;

/* Per-window EGL data (opaque from outside this file) */
struct egl_window_data
{
    EGLSurface  surface;
    EGLContext  context;    /* context that owns the surface (shared objs) */
};

/* -----------------------------------------------------------------------
 * Window registry — maps X11 Window → WSwindow
 * --------------------------------------------------------------------- */

#define  MAX_EGL_WINDOWS  32

static struct {
    Window    x11;
    WSwindow  ws;
} s_windows[MAX_EGL_WINDOWS];
static int s_n_windows = 0;

/* The window whose EGL context is currently current */
static WSwindow  s_current_window = NULL;

static void register_window( Window x11, WSwindow ws )
{
    if( s_n_windows < MAX_EGL_WINDOWS )
    {
        s_windows[s_n_windows].x11 = x11;
        s_windows[s_n_windows].ws  = ws;
        ++s_n_windows;
    }
}

static void unregister_window( Window x11 )
{
    int i;
    for( i = 0; i < s_n_windows; ++i )
    {
        if( s_windows[i].x11 == x11 )
        {
            s_windows[i] = s_windows[--s_n_windows];
            return;
        }
    }
}

static WSwindow lookup_window( Window x11 )
{
    int i;
    for( i = 0; i < s_n_windows; ++i )
        if( s_windows[i].x11 == x11 )
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
 * Key binding — replicate static bind_special_keys() from x_windows.c
 * --------------------------------------------------------------------- */

static void bind_special_keys( void )
{
    /* No-op: key translation is done directly in translate_key() via
     * KeySym switch — XRebindKeysym is not used because it can deadlock
     * on remote X11 servers (x2go, SSH X11 forwarding). */
    (void)0;
}

/* -----------------------------------------------------------------------
 * Modifier-key extraction from X11 event state mask
 * --------------------------------------------------------------------- */

static int get_modifiers( unsigned int state )
{
    int mod = 0;
    if( state & ShiftMask   ) mod |= SHIFT_KEY_BIT;
    if( state & ControlMask ) mod |= CTRL_KEY_BIT;
    if( state & Mod1Mask    ) mod |= ALT_KEY_BIT;   /* Alt / Meta */
    return mod;
}

/* -----------------------------------------------------------------------
 * Key translation — maps XEvent to a bicgl key code.
 *
 * We do NOT use XRebindKeysym (it can deadlock on remote X11 servers such
 * as x2go / SSH X11 forwarding).  Instead we:
 *   1. Try XLookupString for printable / ASCII keys.
 *   2. Fall through to a direct KeySym → bicgl-code table for special keys.
 * --------------------------------------------------------------------- */

static VIO_BOOL translate_key( XEvent *xe, int *key )
{
    char            buf[10];
    KeySym          sym;
    XComposeStatus  comp;
    int             n;

    n = XLookupString( &xe->xkey, buf, (int)sizeof(buf), &sym, &comp );
    if( n >= 1 )
    {
        *key = (int)((unsigned char *)buf)[0];
        return TRUE;
    }

    /* Special / non-printing keys — map KeySym directly */
    switch( sym )
    {
    case XK_Left:       *key = LEFT_ARROW_KEY;    return TRUE;
    case XK_Right:      *key = RIGHT_ARROW_KEY;   return TRUE;
    case XK_Down:       *key = DOWN_ARROW_KEY;    return TRUE;
    case XK_Up:         *key = UP_ARROW_KEY;      return TRUE;
    case XK_Shift_L:    *key = LEFT_SHIFT_KEY;    return TRUE;
    case XK_Shift_R:    *key = RIGHT_SHIFT_KEY;   return TRUE;
    case XK_Control_L:  *key = LEFT_CTRL_KEY;     return TRUE;
    case XK_Control_R:  *key = RIGHT_CTRL_KEY;    return TRUE;
    case XK_Alt_L:      *key = LEFT_ALT_KEY;      return TRUE;
    case XK_Alt_R:      *key = RIGHT_ALT_KEY;     return TRUE;
    case XK_F1:         *key = BICGL_F1_KEY;      return TRUE;
    case XK_F2:         *key = BICGL_F2_KEY;      return TRUE;
    case XK_F3:         *key = BICGL_F3_KEY;      return TRUE;
    case XK_F4:         *key = BICGL_F4_KEY;      return TRUE;
    case XK_F5:         *key = BICGL_F5_KEY;      return TRUE;
    case XK_F6:         *key = BICGL_F6_KEY;      return TRUE;
    case XK_F7:         *key = BICGL_F7_KEY;      return TRUE;
    case XK_F8:         *key = BICGL_F8_KEY;      return TRUE;
    case XK_F9:         *key = BICGL_F9_KEY;      return TRUE;
    case XK_F10:        *key = BICGL_F10_KEY;     return TRUE;
    case XK_F11:        *key = BICGL_F11_KEY;     return TRUE;
    case XK_F12:        *key = BICGL_F12_KEY;     return TRUE;
    case XK_Page_Up:    *key = BICGL_PGUP_KEY;    return TRUE;
    case XK_Page_Down:  *key = BICGL_PGDN_KEY;    return TRUE;
    case XK_Home:       *key = BICGL_HOME_KEY;    return TRUE;
    case XK_End:        *key = BICGL_END_KEY;     return TRUE;
    case XK_Insert:     *key = BICGL_INSERT_KEY;  return TRUE;
    case XK_Delete:     *key = 127;               return TRUE;
    default:            return FALSE;
    }
}

/* -----------------------------------------------------------------------
 * Y-coordinate flip (OpenGL is bottom-up, X11 is top-down)
 * --------------------------------------------------------------------- */

static int flip_y( WSwindow ws, int y )
{
    return ws->height - 1 - y;
}

/* -----------------------------------------------------------------------
 * EGL initialisation (called once, lazily, from WS_create_window)
 * --------------------------------------------------------------------- */

static VIO_BOOL egl_init( void )
{
    static VIO_BOOL done = FALSE;
    if( done ) return TRUE;

    s_display = XOpenDisplay( NULL );
    if( !s_display )
    {
        print_error( "EGL backend: cannot open X display.\n" );
        return FALSE;
    }
    s_screen = DefaultScreen( s_display );

    s_wm_delete_window = XInternAtom( s_display, "WM_DELETE_WINDOW", False );

    /* When running over SSH X11 forwarding (or any remote X11 connection),
     * DRI3 file-descriptor passing across the TCP tunnel is impossible.
     * Force Mesa's "swrast" software-rasteriser driver, which uses plain
     * Xlib pixel-transfer (XPutImage) rather than DRI3/DRI2/KMS, so it
     * works over any X11 connection.
     *
     * setenv(..., 0) means "set only if not already in the environment",
     * so the user can always override from their shell. */
    setenv( "LIBGL_ALWAYS_SOFTWARE",       "1",     0 );
    setenv( "MESA_LOADER_DRIVER_OVERRIDE", "swrast", 0 );
    setenv( "EGL_PLATFORM",                "x11",   0 );

    /* Try eglGetPlatformDisplay first (EGL_EXT_platform_x11 / EGL 1.5)
     * so the platform is unambiguous even with GLVND dispatch. */
    s_egl_dpy = EGL_NO_DISPLAY;
#if defined(EGL_PLATFORM_X11_EXT)
    {
        PFNEGLGETPLATFORMDISPLAYEXTPROC fn =
            (PFNEGLGETPLATFORMDISPLAYEXTPROC)
            eglGetProcAddress( "eglGetPlatformDisplayEXT" );
        if( fn )
            s_egl_dpy = fn( EGL_PLATFORM_X11_EXT, (void *) s_display, NULL );
    }
#endif
    if( s_egl_dpy == EGL_NO_DISPLAY )
        s_egl_dpy = eglGetDisplay( (EGLNativeDisplayType) s_display );

    if( s_egl_dpy == EGL_NO_DISPLAY )
    {
        print_error( "EGL backend: eglGetDisplay failed (EGL error 0x%x).\n",
                     (unsigned) eglGetError() );
        return FALSE;
    }

    EGLint major = 0, minor = 0;
    if( !eglInitialize( s_egl_dpy, &major, &minor ) )
    {
        print_error( "EGL backend: eglInitialize failed (EGL error 0x%x).\n"
                     "  Hint: try setting MESA_LOADER_DRIVER_OVERRIDE=swrast\n"
                     "        and LIBGL_ALWAYS_SOFTWARE=1 in your environment.\n",
                     (unsigned) eglGetError() );
        return FALSE;
    }
    fprintf( stderr, "EGL backend: initialised EGL %d.%d.\n", major, minor );

    if( !eglBindAPI( EGL_OPENGL_API ) )
    {
        print_error( "EGL backend: eglBindAPI(EGL_OPENGL_API) failed.\n" );
        return FALSE;
    }

    /* Request an OpenGL 2.x-capable RGBA config with depth */
    EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_DEPTH_SIZE,      16,
        EGL_NONE
    };

    EGLint n_configs = 0;
    if( !eglChooseConfig( s_egl_dpy, attribs, &s_egl_cfg, 1, &n_configs )
        || n_configs < 1 )
    {
        print_error( "EGL backend: eglChooseConfig failed.\n" );
        return FALSE;
    }

    done = TRUE;
    return TRUE;
}

/* -----------------------------------------------------------------------
 * WS_initialize
 * --------------------------------------------------------------------- */

void  WS_initialize( void )
{
    /* Nothing to do here — EGL is initialised lazily on first
       WS_create_window() call, once we know a display is needed. */
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

    if( !egl_init() )
        return VIO_ERROR;

    if( initial_x_size <= 0 ) initial_x_size = 600;
    if( initial_y_size <= 0 ) initial_y_size = 600;
    if( initial_x_pos  <  0 ) initial_x_pos  = 0;
    if( initial_y_pos  <  0 ) initial_y_pos  = 0;

    /* Get the X11 visual that EGL chose */
    EGLint visual_id = 0;
    eglGetConfigAttrib( s_egl_dpy, s_egl_cfg, EGL_NATIVE_VISUAL_ID, &visual_id );

    XVisualInfo vi_template;
    int         n_vi = 0;
    vi_template.visualid = (VisualID) visual_id;
    XVisualInfo *vi = XGetVisualInfo( s_display, VisualIDMask, &vi_template, &n_vi );
    if( !vi || n_vi < 1 )
    {
        /* Fallback: use default visual */
        vi_template.screen = s_screen;
        vi_template.depth  = DefaultDepth( s_display, s_screen );
        vi = XGetVisualInfo( s_display, VisualScreenMask|VisualDepthMask,
                             &vi_template, &n_vi );
        if( !vi || n_vi < 1 )
        {
            print_error( "EGL backend: cannot find X11 visual.\n" );
            return VIO_ERROR;
        }
    }

    Colormap cmap = XCreateColormap( s_display,
                                     RootWindow( s_display, s_screen ),
                                     vi->visual, AllocNone );

    XSetWindowAttributes swa;
    swa.colormap   = cmap;
    swa.border_pixel = 0;
    swa.event_mask = KeyPressMask | KeyReleaseMask |
                     PointerMotionMask |
                     ButtonPressMask  | ButtonReleaseMask |
                     ExposureMask     | StructureNotifyMask |
                     FocusChangeMask  | EnterWindowMask | LeaveWindowMask;

    Window parent_x11 = ( parent != NULL ) ? parent->window_id
                                           : RootWindow( s_display, s_screen );

    Window x11_win = XCreateWindow( s_display, parent_x11,
                                    initial_x_pos, initial_y_pos,
                                    (unsigned)initial_x_size,
                                    (unsigned)initial_y_size,
                                    0,
                                    vi->depth, InputOutput,
                                    vi->visual,
                                    CWColormap | CWBorderPixel | CWEventMask,
                                    &swa );
    XFree( vi );
    XFreeColormap( s_display, cmap );

    XStoreName( s_display, x11_win, title ? title : "" );

    /* Register WM_DELETE_WINDOW protocol so we get ClientMessage on close */
    XSetWMProtocols( s_display, x11_win, &s_wm_delete_window, 1 );

    /* Create EGL surface */
    EGLSurface egl_surf = eglCreateWindowSurface( s_egl_dpy, s_egl_cfg,
                                                  (EGLNativeWindowType) x11_win,
                                                  NULL );
    if( egl_surf == EGL_NO_SURFACE )
    {
        print_error( "EGL backend: eglCreateWindowSurface failed.\n" );
        XDestroyWindow( s_display, x11_win );
        return VIO_ERROR;
    }

    /* Context attributes: request OpenGL 2.1 */
    EGLint ctx_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 2,
        EGL_CONTEXT_MINOR_VERSION, 1,
        EGL_NONE
    };

    EGLContext egl_ctx = eglCreateContext( s_egl_dpy, s_egl_cfg,
                                           s_shared_ctx, ctx_attribs );
    if( egl_ctx == EGL_NO_CONTEXT )
    {
        /* Some Mesa versions don't support EGL_CONTEXT_MAJOR/MINOR_VERSION
           with the desktop GL API — fall back to no version hint */
        EGLint ctx_attribs_simple[] = { EGL_NONE };
        egl_ctx = eglCreateContext( s_egl_dpy, s_egl_cfg,
                                    s_shared_ctx, ctx_attribs_simple );
    }
    if( egl_ctx == EGL_NO_CONTEXT )
    {
        print_error( "EGL backend: eglCreateContext failed.\n" );
        eglDestroySurface( s_egl_dpy, egl_surf );
        XDestroyWindow( s_display, x11_win );
        return VIO_ERROR;
    }

    /* First window's context becomes the shared context for subsequent ones */
    if( s_shared_ctx == EGL_NO_CONTEXT )
        s_shared_ctx = egl_ctx;

    /* Allocate and fill the opaque EGL data */
    struct egl_window_data *edata =
        (struct egl_window_data *) malloc( sizeof(struct egl_window_data) );
    if( !edata )
    {
        print_error( "EGL backend: out of memory.\n" );
        eglDestroyContext( s_egl_dpy, egl_ctx );
        eglDestroySurface( s_egl_dpy, egl_surf );
        XDestroyWindow( s_display, x11_win );
        return VIO_ERROR;
    }
    edata->surface = egl_surf;
    edata->context = egl_ctx;

    /* Fill in the WSwindow fields */
    window->window_id         = x11_win;
    window->egl               = edata;
    window->width             = initial_x_size;
    window->height            = initial_y_size;
    window->is_visible        = TRUE;
    window->redisplay_pending = FALSE;
    window->init_x            = initial_x_pos;
    window->init_y            = initial_y_pos;
    window->border_width      = 0;
    window->border_height     = 0;
    window->is_new            = TRUE;

    /* Make context current and load font display lists */
    eglMakeCurrent( s_egl_dpy, egl_surf, egl_surf, egl_ctx );
    s_current_window = window;

    window->font_list_base = (int) glGenLists( 128 );
    create_fixed_font( (GLuint) window->font_list_base );

    window->font_list_base_sized = (int) glGenLists( 128 );
    create_sized_font( (GLuint) window->font_list_base_sized );

    bind_special_keys();

    register_window( x11_win, window );

    /* Map (show) the window */
    XMapWindow( s_display, x11_win );
    XFlush( s_display );

    /* Report back what we actually support */
    if( actual_colour_map_mode      ) *actual_colour_map_mode      = FALSE;
    if( actual_double_buffer_flag   ) *actual_double_buffer_flag   = TRUE;
    if( actual_depth_buffer_flag    ) *actual_depth_buffer_flag    = TRUE;
    if( actual_n_overlay_planes     ) *actual_n_overlay_planes     = 0;

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
    if( s_display && window && window->window_id )
        XStoreName( s_display, window->window_id, title ? title : "" );
}

void  WS_delete_window( WSwindow window )
{
    if( !window ) return;

    if( s_current_window == window )
    {
        eglMakeCurrent( s_egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
        s_current_window = NULL;
    }

    unregister_window( window->window_id );

    if( window->egl )
    {
        if( window->egl->context != s_shared_ctx )
            eglDestroyContext( s_egl_dpy, window->egl->context );
        eglDestroySurface( s_egl_dpy, window->egl->surface );
        free( window->egl );
        window->egl = NULL;
    }

    XDestroyWindow( s_display, window->window_id );
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
    (void)bitplanes;   /* No overlay support in EGL backend */
    if( !window || !window->egl ) return;
    if( s_current_window == window ) return;

    eglMakeCurrent( s_egl_dpy,
                    window->egl->surface,
                    window->egl->surface,
                    window->egl->context );
    s_current_window = window;
}

int  WS_get_n_overlay_planes( void )
{
    return 0;
}

void  WS_get_window_position( int *x_pos, int *y_pos )
{
    if( s_current_window )
    {
        *x_pos = s_current_window->init_x;
        *y_pos = s_current_window->init_y;
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
    if( s_current_window && s_current_window->egl )
        eglSwapBuffers( s_egl_dpy, s_current_window->egl->surface );
}

/* -----------------------------------------------------------------------
 * Font / text — uses stored_font.c display lists
 * --------------------------------------------------------------------- */

void  WS_draw_text( Font_types type, VIO_Real size, VIO_STR string )
{
    (void)size;
    if( !string || !s_current_window ) return;

    /* Use the compact 6-pixel-advance font for SIZED_FONT so that text
     * fits within register's fixed-width button areas. */
    if( type == SIZED_FONT )
        glListBase( (GLuint) s_current_window->font_list_base_sized );
    else
        glListBase( (GLuint) s_current_window->font_list_base );

    glCallLists( (GLsizei) strlen(string), GL_UNSIGNED_BYTE,
                 (const GLubyte *) string );
}

VIO_Real  WS_get_character_height( Font_types type, VIO_Real size )
{
    /* For SIZED_FONT, return the requested size so register's layout
     * allocates the correct line height for the requested font size.
     * For FIXED_FONT, return the actual bitmap height (13px). */
    if( type == SIZED_FONT )
        return size;
    return get_fixed_font_height();
}

VIO_Real  WS_get_text_length( VIO_STR str, Font_types type, VIO_Real size )
{
    (void)size;
    if( !str ) return 0.0;

    /* SIZED_FONT uses the 6-pixel-advance display lists; FIXED_FONT uses 8. */
    if( type == SIZED_FONT )
        return (VIO_Real)( strlen(str) ) * 6.0;
    else
        return (VIO_Real)( strlen(str) ) * get_fixed_font_width( str[0] );
}

/* -----------------------------------------------------------------------
 * Screen size
 * --------------------------------------------------------------------- */

void  WS_get_screen_size( int *x_size, int *y_size )
{
    if( s_display )
    {
        *x_size = DisplayWidth(  s_display, s_screen );
        *y_size = DisplayHeight( s_display, s_screen );
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

void WS_set_update_function(         void (*f)(Window_id) )                   { display_callback         = f; }
void WS_set_update_overlay_function( void (*f)(Window_id) )                   { display_overlay_callback = f; }
void WS_set_resize_function(         void (*f)(Window_id,int,int,int,int) )   { resize_callback          = f; }
void WS_set_key_down_function(       void (*f)(Window_id,int,int,int,int) )   { key_down_callback        = f; }
void WS_set_key_up_function(         void (*f)(Window_id,int,int,int,int) )   { key_up_callback          = f; }
void WS_set_mouse_movement_function( void (*f)(Window_id,int,int) )           { mouse_motion_callback    = f; }
void WS_set_left_mouse_down_function(  void (*f)(Window_id,int,int,int) )     { left_down_callback       = f; }
void WS_set_left_mouse_up_function(    void (*f)(Window_id,int,int,int) )     { left_up_callback         = f; }
void WS_set_middle_mouse_down_function(void (*f)(Window_id,int,int,int) )     { middle_down_callback     = f; }
void WS_set_middle_mouse_up_function(  void (*f)(Window_id,int,int,int) )     { middle_up_callback       = f; }
void WS_set_right_mouse_down_function( void (*f)(Window_id,int,int,int) )     { right_down_callback      = f; }
void WS_set_right_mouse_up_function(   void (*f)(Window_id,int,int,int) )     { right_up_callback        = f; }
void WS_set_scroll_up_function(        void (*f)(Window_id,int,int,int) )     { scroll_up_callback       = f; }
void WS_set_scroll_down_function(      void (*f)(Window_id,int,int,int) )     { scroll_down_callback     = f; }
void WS_set_iconify_function(          void (*f)(Window_id) )                 { iconify_callback         = f; }
void WS_set_deiconify_function(        void (*f)(Window_id) )                 { deiconify_callback       = f; }
void WS_set_enter_function(            void (*f)(Window_id) )                 { enter_callback           = f; }
void WS_set_leave_function(            void (*f)(Window_id) )                 { leave_callback           = f; }
void WS_set_quit_function(             void (*f)(Window_id) )                 { quit_callback            = f; }

/* -----------------------------------------------------------------------
 * Timer and idle management
 * --------------------------------------------------------------------- */

void  WS_add_timer_function( VIO_Real seconds, void (*func)(void*), void *data )
{
    int i;
    timer_entry *te;

    /* Find a free slot */
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
 * Dispatch a single X event
 * --------------------------------------------------------------------- */

static void dispatch_xevent( XEvent *xe )
{
    WSwindow   ws;
    Window_id  wid;
    int        x, y, mod, key;

    wid = xe->xany.window;
    ws  = lookup_window( wid );

    switch( xe->type )
    {
    case KeyPress:
        if( !key_down_callback ) break;
        if( translate_key( xe, &key ) )
        {
            mod = get_modifiers( xe->xkey.state );
            x   = xe->xkey.x;
            y   = ws ? flip_y( ws, xe->xkey.y ) : xe->xkey.y;
            (*key_down_callback)( wid, key, x, y, mod );
        }
        break;

    case KeyRelease:
        if( !key_up_callback ) break;
        if( translate_key( xe, &key ) )
        {
            mod = get_modifiers( xe->xkey.state );
            x   = xe->xkey.x;
            y   = ws ? flip_y( ws, xe->xkey.y ) : xe->xkey.y;
            (*key_up_callback)( wid, key, x, y, mod );
        }
        break;

    case MotionNotify:
        if( !mouse_motion_callback ) break;
        x = xe->xmotion.x;
        y = ws ? flip_y( ws, xe->xmotion.y ) : xe->xmotion.y;
        (*mouse_motion_callback)( wid, x, y );
        break;

    case ButtonPress:
        mod = get_modifiers( xe->xbutton.state );
        x   = xe->xbutton.x;
        y   = ws ? flip_y( ws, xe->xbutton.y ) : xe->xbutton.y;
        switch( xe->xbutton.button )
        {
        case Button1: if( left_down_callback   ) (*left_down_callback)(  wid,x,y,mod); break;
        case Button2: if( middle_down_callback ) (*middle_down_callback)(wid,x,y,mod); break;
        case Button3: if( right_down_callback  ) (*right_down_callback)( wid,x,y,mod); break;
        case Button4: if( scroll_up_callback   ) (*scroll_up_callback)(  wid,x,y,mod); break;
        case Button5: if( scroll_down_callback ) (*scroll_down_callback)(wid,x,y,mod); break;
        }
        break;

    case ButtonRelease:
        mod = get_modifiers( xe->xbutton.state );
        x   = xe->xbutton.x;
        y   = ws ? flip_y( ws, xe->xbutton.y ) : xe->xbutton.y;
        switch( xe->xbutton.button )
        {
        case Button1: if( left_up_callback   ) (*left_up_callback)(  wid,x,y,mod); break;
        case Button2: if( middle_up_callback ) (*middle_up_callback)(wid,x,y,mod); break;
        case Button3: if( right_up_callback  ) (*right_up_callback)( wid,x,y,mod); break;
        }
        break;

    case ConfigureNotify:
        if( ws )
        {
            ws->width  = xe->xconfigure.width;
            ws->height = xe->xconfigure.height;
        }
        if( resize_callback )
            (*resize_callback)( wid,
                                xe->xconfigure.x, xe->xconfigure.y,
                                xe->xconfigure.width, xe->xconfigure.height );
        break;

    case Expose:
        if( xe->xexpose.count == 0 && ws )
            ws->redisplay_pending = TRUE;
        break;

    case MapNotify:
        if( deiconify_callback ) (*deiconify_callback)( wid );
        break;

    case UnmapNotify:
        if( iconify_callback ) (*iconify_callback)( wid );
        break;

    case EnterNotify:
    case FocusIn:
        if( enter_callback ) (*enter_callback)( wid );
        break;

    case LeaveNotify:
    case FocusOut:
        if( leave_callback ) (*leave_callback)( wid );
        break;

    case DestroyNotify:
        if( quit_callback ) (*quit_callback)( wid );
        break;

    case ClientMessage:
        if( (Atom)xe->xclient.data.l[0] == s_wm_delete_window )
        {
            if( quit_callback )
                (*quit_callback)( wid );
            else
                s_quit_loop = TRUE;
        }
        break;

    default:
        break;
    }
}

/* -----------------------------------------------------------------------
 * Fire expired timers; return microseconds until the next one (or -1)
 * --------------------------------------------------------------------- */

static long fire_timers( void )
{
    int   i;
    long  min_usec = -1L;

    for( i = 0; i < s_n_timers; ++i )
    {
        if( !s_timers[i].active ) continue;

        long usec = timeval_usec_until( &s_timers[i].fire_time );
        if( usec <= 0 )
        {
            s_timers[i].active = FALSE;
            (*s_timers[i].func)( s_timers[i].data );
        }
        else
        {
            if( min_usec < 0 || usec < min_usec )
                min_usec = usec;
        }
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
            /* Make the window's GL context current before invoking callback */
            WS_set_bitplanes( ws, NORMAL_PLANES );
            if( display_callback )
                (*display_callback)( ws->window_id );
        }
    }
}

/* -----------------------------------------------------------------------
 * WS_event_loop — select()-based to avoid busy-waiting
 * --------------------------------------------------------------------- */

void  WS_event_loop( void )
{
    int x11_fd;

    if( !s_display ) return;
    x11_fd = ConnectionNumber( s_display );

    s_quit_loop = FALSE;

    while( !s_quit_loop )
    {
        /* 1. Drain all pending X events */
        while( XPending( s_display ) )
        {
            XEvent xe;
            XNextEvent( s_display, &xe );
            dispatch_xevent( &xe );
            if( s_quit_loop ) break;
        }
        if( s_quit_loop ) break;

        /* 2. Call idle functions */
        if( s_n_idles > 0 )
        {
            int i;
            for( i = 0; i < s_n_idles; ++i )
                (*s_idles[i].func)( s_idles[i].data );
        }

        /* 3. Fire any expired timers */
        long next_timer_usec = fire_timers();

        /* 4. Fire pending redraws */
        fire_redraws();

        /* 5. Wait for next X event or next timer, whichever comes first.
              If there are idle functions, use a zero timeout (poll only). */
        if( !s_quit_loop )
        {
            struct timeval tv;
            struct timeval *tvp = NULL;

            if( s_n_idles > 0 )
            {
                tv.tv_sec  = 0;
                tv.tv_usec = 0;
                tvp = &tv;
            }
            else if( next_timer_usec > 0 )
            {
                tv.tv_sec  = next_timer_usec / 1000000L;
                tv.tv_usec = next_timer_usec % 1000000L;
                tvp = &tv;
            }

            fd_set fds;
            FD_ZERO( &fds );
            FD_SET( x11_fd, &fds );
            select( x11_fd + 1, &fds, NULL, NULL, tvp );
        }
    }
}

void  WS_exit_loop( void )
{
    s_quit_loop = TRUE;
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
    if( s_display && window->window_id )
    {
        if( is_visible )
            XMapWindow(   s_display, window->window_id );
        else
            XUnmapWindow( s_display, window->window_id );
        XFlush( s_display );
    }
}

void  WS_set_geometry( WSwindow window, int x, int y, int cx, int cy )
{
    if( !window || !s_display ) return;

    unsigned int mask = 0;
    XWindowChanges wc;
    memset( &wc, 0, sizeof(wc) );

    if( x >= 0 && y >= 0 )
    {
        wc.x    = x;
        wc.y    = y;
        mask   |= CWX | CWY;
    }
    if( cx > 0 && cy > 0 )
    {
        wc.width  = cx;
        wc.height = cy;
        mask     |= CWWidth | CWHeight;
        window->width  = cx;
        window->height = cy;
    }
    if( mask )
        XConfigureWindow( s_display, window->window_id, mask, &wc );
}
