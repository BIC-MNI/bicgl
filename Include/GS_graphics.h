#ifndef  DEF_GRAPHICS_SYSTEM_SPECIFIC
#define  DEF_GRAPHICS_SYSTEM_SPECIFIC

#include  <graphics_base.h>

/*---  This is a placeholder to hide the specifics of GS_windows from API,
       not elegant, but I'll get back to this */

typedef  struct
{
    int  empty;
}
GS_window_struct;

typedef  GS_window_struct   *GSwindow;

#endif

