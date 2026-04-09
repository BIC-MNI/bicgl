#ifndef  DEF_EGL_WS_GRAPHICS
#define  DEF_EGL_WS_GRAPHICS

#include  <graphics_base.h>

#include  <X11/Xlib.h>
#include  <X11/Xutil.h>

#ifdef  Status
#undef  Status
#endif

typedef  Window   Window_id;

/* Opaque pointer to EGL surface + context data.
   Defined in egl_windows.c — keeps EGL headers out of GS_graphics.h. */
struct egl_window_data;

typedef  struct
{
    Window_id               window_id;
    struct egl_window_data *egl;          /* EGL surface + context         */
    int                     width;
    int                     height;
    int                     font_list_base;       /* FIXED_FONT GL lists (8px advance)  */
    int                     font_list_base_sized; /* SIZED_FONT GL lists (6px advance)  */
    VIO_BOOL                is_visible;
    VIO_BOOL                redisplay_pending;
    /* Fields expected by GS_window_struct (via OpenGL_graphics/windows.c) */
    int                     init_x, init_y;
    int                     border_width, border_height;
    VIO_BOOL                is_new;
}
WS_window_struct;

typedef  WS_window_struct *  WSwindow;

#include  <egl_window_prototypes.h>

#endif
