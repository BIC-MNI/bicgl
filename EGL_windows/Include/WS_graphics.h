#ifndef  DEF_EGL_WS_GRAPHICS
#define  DEF_EGL_WS_GRAPHICS

#include  <graphics_base.h>

/* X11 — provides typedef Window Window_id; also used for font loading */
#include  <X11/Xlib.h>
#include  <X11/Xutil.h>

#ifdef  Status
#undef  Status
#endif

/* GLFW — for GLFWwindow* field in WS_window_struct */
#define GLFW_INCLUDE_NONE
#include  <GLFW/glfw3.h>

typedef  Window   Window_id;

typedef  struct
{
    Window_id    window_id;
    GLFWwindow  *glfw;                /* GLFW window handle                  */
    int          width;
    int          height;
    int          font_list_base;       /* FIXED_FONT GL lists (8px advance)  */
    int          font_list_base_sized; /* SIZED_FONT GL lists (6px advance)  */
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
