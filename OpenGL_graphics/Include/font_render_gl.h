#ifndef DEF_FONT_RENDER_GL
#define DEF_FONT_RENDER_GL

#include <font_atlas.h>

/* Draws `s` using `atlas`, anchored at the window's current GL raster
 * position (as set by a prior glRasterPos / GS_set_raster_position call)
 * and tinted by the current GL colour (as set by a prior glColor /
 * GS_set_colour call) -- same implicit contract WS_draw_text has always
 * had. No-op if the current raster position is invalid (matching the
 * glBitmap-based renderer this replaces). Pure OpenGL 1.x/2.0 calls, no
 * backend-specific (GLX/EGL/GLUT) code -- shared verbatim by all three
 * bicgl windowing backends. */
void font_render_gl_draw_text( const FontAtlas *atlas, const char *s );

#endif /* DEF_FONT_RENDER_GL */
