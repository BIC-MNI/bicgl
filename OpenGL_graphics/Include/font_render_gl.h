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

/* Converts a Font_types "size" value (as passed by Display's and
 * Register's global_variables.h font-size constants) plus the window's
 * current dpi_scale into the final pixel height to bake/look up in the
 * font atlas: pixel_height = size * dpi_scale, nothing else.
 *
 * Deliberately NOT floored, boosted, or branched by Font_types (an
 * earlier version of this function tried both a reference-density floor
 * for SIZED_FONT and a flat ignore-"size" constant for FIXED_FONT,
 * reconstructed from the pre-stb_truetype renderer's old X11/GLUT font
 * lookups). Both were wrong in practice: Display's and Register's own
 * scale_ui_geometry() (Display/main/main.c, Register/User_interface/
 * main/initialize.c) scale surrounding UI geometry -- button/panel/
 * colour-bar dimensions -- by this exact same real dpi_scale, with no
 * floor of their own. Flooring only the font size (not the geometry it
 * has to fit inside) desynced the two on any display where dpi_scale
 * doesn't already meet the floor (i.e. every non-Retina/non-HiDPI
 * display) -- e.g. Register's Button_height staying at its base size
 * while button text rendered at 2x that, visibly overflowing. Font size
 * must track the same dpi_scale as its container, always, with no
 * exceptions -- if a particular Font_types/size combination still looks
 * too small or too large after that, the fix is to edit that specific
 * named size constant (as already done successfully for
 * Colour_bar_text_size and Register's button/label/entry/slider sizes),
 * not to reintroduce a formula-level correction here. */
float font_render_gl_pixel_height( float size, float dpi_scale );

#endif /* DEF_FONT_RENDER_GL */
