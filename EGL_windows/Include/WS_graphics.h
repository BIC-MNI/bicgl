#ifndef  DEF_EGL_WS_GRAPHICS
#define  DEF_EGL_WS_GRAPHICS

#include  <graphics_base.h>

#if defined(__APPLE__)
/* No X11 on macOS (native Cocoa via GLFW) — Window_id just needs to match
 * X11's Window (unsigned long) in size so it can still hold the synthesized
 * ids produced by the EGL_windows backend on non-X11 platforms. */
typedef  unsigned long   Window_id;
#else
/* X11 — provides typedef Window Window_id; also used for font loading */
#include  <X11/Xlib.h>
#include  <X11/Xutil.h>

#ifdef  Status
#undef  Status
#endif

typedef  Window   Window_id;
#endif

/* GLFW — for GLFWwindow* field in WS_window_struct */
#define GLFW_INCLUDE_NONE
#include  <GLFW/glfw3.h>

typedef  struct
{
    Window_id    window_id;
    GLFWwindow  *glfw;                 /* GLFW window handle                 */
    int          width;                /* framebuffer (physical) pixels      */
    int          height;               /* framebuffer (physical) pixels      */
    int          logical_width;        /* window (logical) pixels            */
    int          logical_height;       /* window (logical) pixels            */
    float        dpi_scale_x;          /* width  / logical_width             */
    float        dpi_scale_y;          /* height / logical_height            */
    VIO_BOOL     is_visible;
    VIO_BOOL     redisplay_pending;
    /* Fields expected by GS_window_struct (via OpenGL_graphics/windows.c) */
    int          init_x, init_y;
    int          border_width, border_height;
    VIO_BOOL     is_new;
}
WS_window_struct;

typedef  WS_window_struct *  WSwindow;

#include  <egl_window_prototypes.h>

#endif
