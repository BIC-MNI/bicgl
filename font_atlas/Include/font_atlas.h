#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H

#include "stb_truetype.h"

#define FONT_ATLAS_N_GLYPHS 95   /* ASCII 32..126 */
#define FONT_ATLAS_FIRST_CODEPOINT 32

typedef struct {
    unsigned char    *atlas_pixels;  /* single channel, atlas_w*atlas_h, owned */
    int               atlas_w, atlas_h;
    stbtt_packedchar  glyphs[FONT_ATLAS_N_GLYPHS];
    float             pixel_height;  /* the size this atlas was baked at */
    float             advance_width; /* fixed cell width, from reference glyph */
    float             ascent;        /* baseline-to-top, scaled to pixel_height */
} FontAtlas;

/* Builds an atlas from an in-memory .ttf buffer. Returns NULL on any
 * failure (bad buffer, no such font, or packing fails even after growing
 * the atlas up to a fixed cap). Never partially initializes the result. */
FontAtlas *font_atlas_build( const unsigned char *ttf_data, int ttf_data_len,
                             float pixel_height );

void font_atlas_free( FontAtlas *atlas );

/* Small cache keyed by pixel_height, so repeated requests at the same
 * size (e.g. during a monitor-drag content-scale storm) reuse the same
 * atlas instead of rebuilding every call. Oldest-entry eviction once
 * capacity is exceeded. */
typedef struct FontAtlasCache FontAtlasCache;

FontAtlasCache *font_atlas_cache_create( const unsigned char *ttf_data,
                                          int ttf_data_len, int capacity );

/* Returns a cached or newly-built atlas for pixel_height. Returned pointer
 * is owned by the cache -- do not free it directly, it is released by
 * font_atlas_cache_destroy() or eviction. Returns NULL if the atlas could
 * not be built. */
FontAtlas *font_atlas_cache_get( FontAtlasCache *cache, float pixel_height );

void font_atlas_cache_destroy( FontAtlasCache *cache );

/* Number of atlases currently held (<= the cache's capacity). Exposed
 * mainly for testing the eviction invariant -- pointer identity is not a
 * reliable way to prove eviction happened, since freed memory can be
 * immediately reused by malloc for the replacement atlas. */
int font_atlas_cache_size( const FontAtlasCache *cache );

#endif /* FONT_ATLAS_H */
