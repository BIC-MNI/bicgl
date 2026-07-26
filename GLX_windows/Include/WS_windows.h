#ifndef  DEF_GLX_WINDOWS
#define  DEF_GLX_WINDOWS

#include  <graphics_base.h>

#include  <volume_io.h>
#include  <x_windows.h>

#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
#include <OpenGL/glx.h>
#else
#include  <GL/glx.h>
#endif

typedef  struct
{
    X_window_struct   x_window;
    GLXContext        graphics_context;
    VIO_BOOL           overlay_present;
    X_window_struct   overlay_window;
    GLXContext        overlay_context;
}
WS_window_struct;

#include  <glx_window_prototypes.h>

#ifdef  public_was_defined_here
#undef       
#undef       public_was_defined_here
#endif

#endif
