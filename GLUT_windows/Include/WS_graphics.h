#ifndef  DEF_GLUT_WINDOWS
#define  DEF_GLUT_WINDOWS

#include  <graphics_base.h>

#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
#include <GLUT/glut.h>
#else
#include  <GL/glut.h>
#endif

typedef  int  Window_id;

typedef  struct
{
    Window_id    window_id;
    VIO_STR      title;
    VIO_BOOL     overlay_present;
    VIO_BOOL     is_visible;
    int          n_colours;
    VIO_SCHAR    *colour_map_entry_set;
    VIO_Colour   *colour_map;
}
WS_window_struct;

typedef  WS_window_struct *  WSwindow;

#include  <glut_window_prototypes.h>

#endif
