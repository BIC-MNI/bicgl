#ifndef  DEF_GRAPHICS_SYSTEM_SPECIFIC
#define  DEF_GRAPHICS_SYSTEM_SPECIFIC

#include <bicpl.h>
#include <WS_graphics.h>
#include <GL/gl.h>

typedef  struct
{
    WS_window_struct   WS_window;
}
GS_window_struct;

typedef  GS_window_struct   *GSwindow;

#ifndef  public
#define       public
#define       public_was_defined_here
#endif

#include  <opengl_graphics_prototypes.h>

#ifdef  public_was_defined_here
#undef       public
#undef       public_was_defined_here
#endif

#endif
