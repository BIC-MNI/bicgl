#ifndef  DEF_GLUT_WINDOWS
#define  DEF_GLUT_WINDOWS

#include  <graphics_base.h>
#include  <glut.h>

typedef  int  Window_id;

typedef  struct
{
    Window_id         window_id;
    Window_id         top_level_window_id;
    BOOLEAN           overlay_present;
    void              *current_font;
}
WS_window_struct;

typedef  WS_window_struct *  WSwindow;

#ifndef  public
#define       public   extern
#define       public_was_defined_here
#endif

#include  <glut_window_prototypes.h>

#ifdef  public_was_defined_here
#undef       public
#undef       public_was_defined_here
#endif

#endif
