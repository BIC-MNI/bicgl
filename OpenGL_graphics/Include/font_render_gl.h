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

/* FIXED_FONT never honored "size" historically (X11 "fixed" font /
 * GLUT_BITMAP_8_BY_13, flat ~13px) -- used for compact status/
 * coordinate readouts where preserving that old compact size matters
 * more than nominal "size" fidelity. Scaled only by real display
 * density, never by the SIZED_FONT reference-density floor below
 * (FIXED_FONT never went through the Helvetica-style lookup either). */
#define FONT_RENDER_GL_FIXED_FONT_PIXELS 13.0f

/* SIZED_FONT constants were tuned assuming rendering at Retina-like
 * pixel density (~2x) -- validated true for macOS, where dpi_scale is
 * already ~2.0 on Retina displays. Backends with no real HiDPI
 * detection (GLX, GLUT, and GLFW on Linux/X11, which this codebase can
 * only ever observe at dpi_scale==1.0) must not fall below that
 * reference density merely because they can't detect it. A floor, not
 * a conditional multiplier: genuinely denser displays keep scaling up
 * via their own real dpi_scale past this reference, they just never
 * scale down below it. */
#define FONT_RENDER_GL_REFERENCE_DPI_SCALE 2.0f

/* Converts a Font_types "size" value (as passed by Display's and
 * Register's global_variables.h font-size constants) plus the window's
 * current dpi_scale into the final pixel height to bake/look up in the
 * font atlas. dpi_scale should be 1.0f for backends with no HiDPI
 * concept (GLX, GLUT). is_fixed_font should be nonzero iff the caller's
 * Font_types is FIXED_FONT -- passed as a plain flag rather than the
 * Font_types enum itself so this otherwise dependency-light module
 * doesn't need to include bicpl's obj_defs.h. */
float font_render_gl_pixel_height( int is_fixed_font, float size, float dpi_scale );

#endif /* DEF_FONT_RENDER_GL */
