#include "font_atlas.h"

/* font_atlas.h already pulled in stb_truetype.h's declarations (guarded by
 * its own __STB_INCLUDE_STB_TRUETYPE_H__ include guard). That guard does
 * NOT cover the implementation section below, so re-including here with
 * STB_TRUETYPE_IMPLEMENTATION defined emits the implementation exactly
 * once, in this translation unit only, without redefinition errors. */
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <stdlib.h>
#include <string.h>

#define FONT_ATLAS_MIN_DIM  512
#define FONT_ATLAS_MAX_DIM  2048
#define FONT_ATLAS_PADDING  1

static FontAtlas *try_build_at_size( const unsigned char *ttf_data,
                                     const stbtt_fontinfo *info,
                                     float pixel_height, int dim )
{
    FontAtlas *atlas = (FontAtlas *) calloc( 1, sizeof(FontAtlas) );
    if( !atlas )
        return NULL;

    unsigned char *pixels = (unsigned char *) calloc( 1, (size_t) dim * (size_t) dim );
    if( !pixels )
    {
        free( atlas );
        return NULL;
    }

    stbtt_pack_context pc;
    int ok = stbtt_PackBegin( &pc, pixels, dim, dim, 0, FONT_ATLAS_PADDING, NULL );
    if( !ok )
    {
        free( pixels );
        free( atlas );
        return NULL;
    }

    stbtt_PackSetOversampling( &pc, 1, 1 );

    int packed = stbtt_PackFontRange( &pc, ttf_data, 0, pixel_height,
                                       FONT_ATLAS_FIRST_CODEPOINT,
                                       FONT_ATLAS_N_GLYPHS, atlas->glyphs );
    stbtt_PackEnd( &pc );

    if( !packed )
    {
        free( pixels );
        free( atlas );
        return NULL;
    }

    atlas->atlas_pixels = pixels;
    atlas->atlas_w = dim;
    atlas->atlas_h = dim;
    atlas->pixel_height = pixel_height;

    stbtt_packedchar *ref = &atlas->glyphs['M' - FONT_ATLAS_FIRST_CODEPOINT];
    atlas->advance_width = ref->xadvance;

    int raw_ascent;
    stbtt_GetFontVMetrics( info, &raw_ascent, NULL, NULL );
    float scale = stbtt_ScaleForPixelHeight( info, pixel_height );
    atlas->ascent = (float) raw_ascent * scale;

    return atlas;
}

FontAtlas *font_atlas_build( const unsigned char *ttf_data, int ttf_data_len,
                             float pixel_height )
{
    if( !ttf_data || ttf_data_len <= 0 || pixel_height <= 0.0f )
        return NULL;

    int offset = stbtt_GetFontOffsetForIndex( ttf_data, 0 );
    if( offset < 0 )
        return NULL;

    stbtt_fontinfo info;
    if( !stbtt_InitFont( &info, ttf_data, offset ) )
        return NULL;

    int dim;
    for( dim = FONT_ATLAS_MIN_DIM; dim <= FONT_ATLAS_MAX_DIM; dim *= 2 )
    {
        FontAtlas *atlas = try_build_at_size( ttf_data, &info, pixel_height, dim );
        if( atlas )
            return atlas;
    }

    return NULL;
}

void font_atlas_free( FontAtlas *atlas )
{
    if( !atlas )
        return;
    free( atlas->atlas_pixels );
    free( atlas );
}

/* ------------------------------------------------------------------ */

typedef struct
{
    float      pixel_height;
    FontAtlas *atlas;
} FontAtlasCacheEntry;

struct FontAtlasCache
{
    const unsigned char *ttf_data;
    int                   ttf_data_len;
    int                   capacity;
    int                   count;
    FontAtlasCacheEntry  *entries; /* oldest at [0], newest appended */
};

FontAtlasCache *font_atlas_cache_create( const unsigned char *ttf_data,
                                          int ttf_data_len, int capacity )
{
    if( !ttf_data || ttf_data_len <= 0 || capacity <= 0 )
        return NULL;

    FontAtlasCache *cache = (FontAtlasCache *) calloc( 1, sizeof(FontAtlasCache) );
    if( !cache )
        return NULL;

    cache->entries = (FontAtlasCacheEntry *) calloc( (size_t) capacity, sizeof(FontAtlasCacheEntry) );
    if( !cache->entries )
    {
        free( cache );
        return NULL;
    }

    cache->ttf_data = ttf_data;
    cache->ttf_data_len = ttf_data_len;
    cache->capacity = capacity;
    cache->count = 0;

    return cache;
}

static int same_pixel_height( float a, float b )
{
    /* Exact comparison is fine here -- callers always pass a value derived
     * deterministically from dpi_scale * requested size, so a tiny epsilon
     * would only risk incorrectly coalescing genuinely distinct sizes. */
    return a == b;
}

FontAtlas *font_atlas_cache_get( FontAtlasCache *cache, float pixel_height )
{
    if( !cache )
        return NULL;

    int i;
    for( i = 0; i < cache->count; ++i )
    {
        if( same_pixel_height( cache->entries[i].pixel_height, pixel_height ) )
            return cache->entries[i].atlas;
    }

    FontAtlas *atlas = font_atlas_build( cache->ttf_data, cache->ttf_data_len, pixel_height );
    if( !atlas )
        return NULL;

    if( cache->count < cache->capacity )
    {
        cache->entries[cache->count].pixel_height = pixel_height;
        cache->entries[cache->count].atlas = atlas;
        cache->count++;
    }
    else
    {
        /* Evict oldest (index 0), shift the rest down, append at the end. */
        font_atlas_free( cache->entries[0].atlas );
        for( i = 0; i < cache->capacity - 1; ++i )
            cache->entries[i] = cache->entries[i + 1];
        cache->entries[cache->capacity - 1].pixel_height = pixel_height;
        cache->entries[cache->capacity - 1].atlas = atlas;
    }

    return atlas;
}

void font_atlas_cache_destroy( FontAtlasCache *cache )
{
    if( !cache )
        return;

    int i;
    for( i = 0; i < cache->count; ++i )
        font_atlas_free( cache->entries[i].atlas );

    free( cache->entries );
    free( cache );
}

int font_atlas_cache_size( const FontAtlasCache *cache )
{
    return cache ? cache->count : 0;
}
