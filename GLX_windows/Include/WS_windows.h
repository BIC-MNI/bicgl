#ifndef  DEF_GLX_WINDOWS
#define  DEF_GLX_WINDOWS

#include  <volume_io.h>
#include  <x_windows.h>
#include  <GL/glx.h>

typedef  GLint  WS_font;

typedef  struct
{
    X_window_struct   x_window;
    GLXContext        graphics_context;
}
WS_window_struct;

typedef  struct
{
    GLint         listBase;
    XFontStruct   *x_font_info;
}  WS_font_info;

#ifndef  public
#define       public   extern
#define       public_was_defined_here
#endif

#include  <glx_window_prototypes.h>

#ifdef  public_was_defined_here
#undef       public
#undef       public_was_defined_here
#endif

#endif
