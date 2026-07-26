/* Automated, headless unit tests for the platform-agnostic font_atlas
 * module (no GL context / display required). Run via CTest; pass = exit
 * code 0. See AGENTS.md / the font-rendering plan for context. */

#include <font_atlas.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FONT_ATLAS_TEST_TTF_PATH
#error "FONT_ATLAS_TEST_TTF_PATH must be defined at compile time"
#endif

static unsigned char *read_file( const char *path, long *len_out )
{
    FILE *f = fopen( path, "rb" );
    if( !f )
        return NULL;

    fseek( f, 0, SEEK_END );
    long len = ftell( f );
    fseek( f, 0, SEEK_SET );

    unsigned char *buf = (unsigned char *) malloc( (size_t) len );
    if( fread( buf, 1, (size_t) len, f ) != (size_t) len )
    {
        fclose( f );
        free( buf );
        return NULL;
    }
    fclose( f );
    *len_out = len;
    return buf;
}

static void test_build_basic( const unsigned char *ttf, int ttf_len )
{
    FontAtlas *atlas = font_atlas_build( ttf, ttf_len, 16.0f );
    assert( atlas != NULL );
    assert( atlas->atlas_w > 0 );
    assert( atlas->atlas_h > 0 );
    assert( atlas->pixel_height == 16.0f );
    assert( atlas->atlas_pixels != NULL );
    font_atlas_free( atlas );
    printf( "test_build_basic: OK\n" );
}

static void test_glyph_metrics_sane( const unsigned char *ttf, int ttf_len )
{
    FontAtlas *atlas = font_atlas_build( ttf, ttf_len, 16.0f );
    assert( atlas != NULL );

    stbtt_packedchar *ga = &atlas->glyphs['A' - FONT_ATLAS_FIRST_CODEPOINT];
    assert( ga->x1 > ga->x0 );
    assert( ga->y1 > ga->y0 );

    stbtt_packedchar *gdot = &atlas->glyphs['.' - FONT_ATLAS_FIRST_CODEPOINT];
    assert( gdot->x1 > gdot->x0 );
    assert( gdot->y1 > gdot->y0 );

    font_atlas_free( atlas );
    printf( "test_glyph_metrics_sane: OK\n" );
}

static void test_fixed_advance_width( const unsigned char *ttf, int ttf_len )
{
    FontAtlas *atlas = font_atlas_build( ttf, ttf_len, 16.0f );
    assert( atlas != NULL );

    stbtt_packedchar *gm = &atlas->glyphs['M' - FONT_ATLAS_FIRST_CODEPOINT];
    assert( atlas->advance_width == gm->xadvance );
    assert( atlas->advance_width > 0.0f );

    font_atlas_free( atlas );
    printf( "test_fixed_advance_width: OK\n" );
}

static void test_ascent_is_sane( const unsigned char *ttf, int ttf_len )
{
    FontAtlas *atlas = font_atlas_build( ttf, ttf_len, 16.0f );
    assert( atlas != NULL );
    /* Ascent must be positive and no larger than the whole baked size. */
    assert( atlas->ascent > 0.0f );
    assert( atlas->ascent <= atlas->pixel_height );
    font_atlas_free( atlas );
    printf( "test_ascent_is_sane: OK\n" );
}

static void test_build_failure_paths( void )
{
    assert( font_atlas_build( NULL, 0, 16.0f ) == NULL );

    unsigned char garbage[64];
    memset( garbage, 0xFF, sizeof(garbage) );
    assert( font_atlas_build( garbage, sizeof(garbage), 16.0f ) == NULL );

    printf( "test_build_failure_paths: OK\n" );
}

static void test_free_null_is_noop( void )
{
    font_atlas_free( NULL );
    printf( "test_free_null_is_noop: OK\n" );
}

static void test_large_pixel_height_grows_atlas( const unsigned char *ttf, int ttf_len )
{
    /* Large enough that a single 512x512 bake can't hold all 95 glyphs. */
    FontAtlas *atlas = font_atlas_build( ttf, ttf_len, 200.0f );
    assert( atlas != NULL );
    assert( atlas->atlas_w > 512 || atlas->atlas_h > 512 );

    font_atlas_free( atlas );
    printf( "test_large_pixel_height_grows_atlas: OK\n" );
}

static void test_cache_same_pixel_height_reuses( const unsigned char *ttf, int ttf_len )
{
    FontAtlasCache *cache = font_atlas_cache_create( ttf, ttf_len, 8 );
    assert( cache != NULL );

    FontAtlas *a1 = font_atlas_cache_get( cache, 16.0f );
    FontAtlas *a2 = font_atlas_cache_get( cache, 16.0f );
    assert( a1 != NULL );
    assert( a1 == a2 );

    font_atlas_cache_destroy( cache );
    printf( "test_cache_same_pixel_height_reuses: OK\n" );
}

static void test_cache_different_pixel_height_rebuilds( const unsigned char *ttf, int ttf_len )
{
    FontAtlasCache *cache = font_atlas_cache_create( ttf, ttf_len, 8 );
    assert( cache != NULL );

    FontAtlas *a1 = font_atlas_cache_get( cache, 16.0f );
    FontAtlas *a2 = font_atlas_cache_get( cache, 32.0f );
    assert( a1 != NULL && a2 != NULL );
    assert( a1 != a2 );
    assert( a2->pixel_height == 32.0f );

    font_atlas_cache_destroy( cache );
    printf( "test_cache_different_pixel_height_rebuilds: OK\n" );
}

static void test_cache_eviction( const unsigned char *ttf, int ttf_len )
{
    /* Pointer identity is not a reliable way to prove an atlas was evicted
     * and rebuilt rather than reused -- freed memory can be immediately
     * reused by malloc for the replacement atlas, at the same address.
     * Instead, check the actual invariant eviction exists to guarantee:
     * the cache never grows past its capacity, however many distinct
     * sizes are requested. */
    const int capacity = 4;
    FontAtlasCache *cache = font_atlas_cache_create( ttf, ttf_len, capacity );
    assert( cache != NULL );

    int i;
    for( i = 0; i < capacity + 2; ++i )
    {
        FontAtlas *a = font_atlas_cache_get( cache, 10.0f + (float) i );
        assert( a != NULL );
        assert( font_atlas_cache_size( cache ) <= capacity );
    }
    assert( font_atlas_cache_size( cache ) == capacity );

    font_atlas_cache_destroy( cache );
    printf( "test_cache_eviction: OK\n" );
}

int main( void )
{
    long ttf_len = 0;
    unsigned char *ttf = read_file( FONT_ATLAS_TEST_TTF_PATH, &ttf_len );
    if( !ttf )
    {
        fprintf( stderr, "could not read test ttf at %s\n", FONT_ATLAS_TEST_TTF_PATH );
        return 1;
    }

    test_build_basic( ttf, (int) ttf_len );
    test_glyph_metrics_sane( ttf, (int) ttf_len );
    test_fixed_advance_width( ttf, (int) ttf_len );
    test_ascent_is_sane( ttf, (int) ttf_len );
    test_build_failure_paths();
    test_free_null_is_noop();
    test_large_pixel_height_grows_atlas( ttf, (int) ttf_len );
    test_cache_same_pixel_height_reuses( ttf, (int) ttf_len );
    test_cache_different_pixel_height_rebuilds( ttf, (int) ttf_len );
    test_cache_eviction( ttf, (int) ttf_len );

    free( ttf );
    printf( "all font_atlas tests passed\n" );
    return 0;
}
