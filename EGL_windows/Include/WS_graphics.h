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

/* -----------------------------------------------------------------------
 * Per-window font cache
 *
 * Each entry holds one loaded (Font_types, size) combination as a bank
 * of 128 GL bitmap display lists.  Metrics (height, per-char widths) are
 * stored so that WS_get_character_height / WS_get_text_length can return
 * accurate values matching the GLX backend.
 * --------------------------------------------------------------------- */
#define EGL_FONT_CACHE_SIZE 8

typedef struct
{
    Font_types    type;
    int           size;                /* rounded point size                 */
    unsigned int  list_base;           /* base of 128 GL display lists       */
    float         advance;             /* fallback advance (max glyph width) */
    float         height;              /* true ascent from X11 font          */
    short         char_widths[128];    /* per-char widths, index = char-0    */
                                       /* 0 means "use advance"              */
    int           valid;
} EglFontEntry;

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
    EglFontEntry font_cache[EGL_FONT_CACHE_SIZE]; /* lazy font cache        */
    int          font_cache_count;     /* number of valid entries            */
    VIO_BOOL     is_visible;
    VIO_BOOL     redisplay_pending;
    int          extra_redraws_needed;   /* ticks to stay awake after swap */
    /* Fields expected by GS_window_struct (via OpenGL_graphics/windows.c) */
    int          init_x, init_y;
    int          border_width, border_height;
    VIO_BOOL     is_new;
}
WS_window_struct;

typedef  WS_window_struct *  WSwindow;

#include  <egl_window_prototypes.h>

#endif
