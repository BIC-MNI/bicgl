/*
 * EGL-backend stub for WS_windows.h.
 * stored_font.c includes <WS_windows.h>; when compiled for the EGL
 * backend the include path puts EGL_windows/Include/ first, so this
 * file is found instead of GLX_windows/Include/WS_windows.h.
 * stored_font.c only needs volume_io types and OpenGL GL/gl.h —
 * it does NOT use the WS_window_struct directly.
 */

#ifndef  DEF_EGL_WS_WINDOWS
#define  DEF_EGL_WS_WINDOWS

#include  <volume_io.h>

/* OpenGL function prototypes (no GLX, no EGL here) */
#include  <GL/gl.h>

/* Pull in the EGL-backend WSwindow definition */
#include  <WS_graphics.h>

#endif
