#ifndef  DEF_GRAPHICS_SYSTEM_SPECIFIC
#define  DEF_GRAPHICS_SYSTEM_SPECIFIC

#include <def_gl.h>
#include <window_events.h>
#include <fmclient.h>

#define  ACCESS_PIXEL( array, x, y, size_x ) \
         (array)[(x) + (size_x) * (y)]

typedef  long  Window_id;

typedef  enum { VIEWING_MATRIX = MVIEWING, PROJECTION_MATRIX = MPROJECTION }
              Matrix_modes;

typedef  enum { LESS_OR_EQUAL = ZF_LEQUAL } Depth_functions;

typedef  fmfonthandle   GS_font;

typedef  struct
{
    Font_types  type;
    Real        size;
    GS_font     font_handle;
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

#define  N_LIGHTS  8

#include <graphics_info.h>

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
