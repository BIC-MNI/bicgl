#ifndef  DEF_GRAPHICS_SYSTEM_SPECIFIC
#define  DEF_GRAPHICS_SYSTEM_SPECIFIC

#include <def_gl.h>
#include <window_events.h>
#include <fmclient.h>

typedef  long  Window_id;

typedef  struct
{
    Font_types       type;
    Real             size;
    fmfonthandle     font_handle;
}  WS_font_info;

typedef  struct
{
    Window_id       window_id;
}
WS_window_struct;

typedef  struct
{
    long            unique_lmdef_id;
}
GS_window_struct;

#define  WS_WINDOW_DEFINED

#include <graphics.h>

#ifndef  public
#define       public
#define       public_was_defined_here
#endif

#include  <irisgl_graphics_prototypes.h>

#ifdef  public_was_defined_here
#undef       public
#undef       public_was_defined_here
#endif

#endif
