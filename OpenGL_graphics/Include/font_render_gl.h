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

/* Restores the effective pixel density the pre-stb_truetype renderer
 * used to produce on Linux/X11 (deleted in 45a2e81), applied only where
 * that old inflation actually used to apply -- non-Retina/non-HiDPI
 * displays. macOS never exercised that X11 code path (no X11 there) and
 * its dpi_scale-driven sizing already looks correct, so it's left alone.
 * 1.5x, not the ~1.39x the old X11-Helvetica-at-100dpi lookup implied:
 * this is the value two independent manual eyeball corrections (Display's
 * Colour_bar_text_size and Register's button/label/entry/slider font
 * sizes, both 10.0 -> 15.0, reverted once this constant existed) actually
 * converged on -- trust the empirical result over the archaeology. */
#define FONT_RENDER_GL_LEGACY_POINT_SCALE 1.5f

/* Converts a Font_types "size" value (as passed by Display's and
 * Register's global_variables.h font-size constants) plus the window's
 * current dpi_scale into the final pixel height to bake/look up in the
 * font atlas. dpi_scale should be 1.0f for backends with no HiDPI
 * concept (GLX, GLUT). */
float font_render_gl_pixel_height( float size, float dpi_scale );

#endif /* DEF_FONT_RENDER_GL */
