
#include  <internal_volume_io.h>
#include  <gs_specific.h>

public  void  GS_set_point(
    Point  *point )
{
    v3f( point->coords );
}

public  void  GS_set_normal(
    Vector  *normal )
{
    n3f( normal->coords );
}

public  void  GS_set_colour(
    Colour  colour )
{
    cpack( colour );
}

public  void  GS_set_colour_index(
    Colour  colour )
{
    color( (Colorindex) colour );
}

public  void  GS_set_ambient_and_diffuse_mode(
    BOOLEAN  state )
{
    if( state )
        lmcolor( LMC_AD );
    else
        lmcolor( LMC_COLOR );
}

#ifndef  TWO_D_ONLY
private  long  get_unique_lmdef_id( void )
{
    static  long   unique_lmdef_id = 1;

    unique_lmdef_id += 8;
    if( unique_lmdef_id > 60000 )
        unique_lmdef_id = 1;

    return( unique_lmdef_id );
}
#endif

public  void  GS_initialize_surface_property(
    Gwindow        window )
{
#ifndef  TWO_D_ONLY
    static  Surfprop  spr = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    window->GS_window->unique_lmdef_id = get_unique_lmdef_id();
    GS_set_surface_property( window, WHITE, &spr );
#endif
}

public  void  GS_set_surface_property(
    Gwindow        window,
    Colour         col,
    Surfprop       *surfprop )
{
#ifndef  TWO_D_ONLY
    static  float  props[] = { (float) AMBIENT, 0.0f, 0.0f, 0.0f,
                               (float) DIFFUSE, 0.0f, 0.0f, 0.0f,
                               (float) SPECULAR, 0.0f, 0.0f, 0.0f,
                               (float) SHININESS, 0.0f,
                               (float) ALPHA, 0.0f,
                               (float) LMNULL };

    props[1] = (float) ((Real) Surfprop_a( *surfprop )*get_Colour_r_0_1( col ));
    props[2] = (float) ((Real) Surfprop_a( *surfprop )*get_Colour_g_0_1( col ));
    props[3] = (float) ((Real) Surfprop_a( *surfprop )*get_Colour_b_0_1( col ));

    props[5] = (float) ((Real) Surfprop_d( *surfprop )*get_Colour_r_0_1( col ));
    props[6] = (float) ((Real) Surfprop_d( *surfprop )*get_Colour_g_0_1( col ));
    props[7] = (float) ((Real) Surfprop_d( *surfprop )*get_Colour_b_0_1( col ));

    props[9] = Surfprop_s( *surfprop );
    props[10] = Surfprop_s( *surfprop );
    props[11] = Surfprop_s( *surfprop );

    props[13] = Surfprop_se( *surfprop );

    props[15] = Surfprop_t( *surfprop );

    lmdef( DEFMATERIAL, (short) window->GS_window->unique_lmdef_id,
           SIZEOF_STATIC_ARRAY(props), props );
#endif
}

public  void  GS_set_line_width(
    Real  line_width )
{
    linewidth( (short) line_width );
}

public  void  GS_curve(
    float  geom[4][3] )
{
    crv( geom );
}

public  void  GS_begin_point( void )
{
    bgnpoint();
}

public  void  GS_end_point( void )
{
    endpoint();
}

public  void  GS_begin_line( void )
{
    bgnline();
}

public  void  GS_end_line( void )
{
    endline();
}

public  void  GS_begin_closed_line( void )
{
    bgnclosedline();
}

public  void  GS_end_closed_line( void )
{
    endclosedline();
}

public  void  GS_begin_polygon( void )
{
    bgnpolygon();
}

public  void  GS_end_polygon( void )
{
    endpolygon();
}
public  void  GS_begin_quad_strip( void )
{
    bgnqstrip();
}

public  void  GS_end_quad_strip( void )
{
    endqstrip();
}

public  void  GS_set_raster_position(
    Real  x,
    Real  y,
    Real  z )
{
    cmov( (Coord) x, (Coord) y, (Coord) z );
}

public  void  GS_set_pixel_zoom(
    Real  x_zoom,
    Real  y_zoom )
{
    rectzoom( (float) x_zoom, (float) y_zoom );
}

public  void  GS_draw_colour_map_pixels(
    Gwindow         window,
    pixels_struct   *pixels )
{
    int   x, y, x_size, y_size;

    x = pixels->x_position + window->x_viewport_min;
    y = pixels->y_position + window->y_viewport_min;
    x_size = pixels->x_size;
    y_size = pixels->y_size;

    switch( pixels->pixel_type )
    {
    case COLOUR_INDEX_8BIT_PIXEL:
        pixmode( PM_SIZE, 8 );
        lrectwrite( (Screencoord) x, (Screencoord) y,
                    (Screencoord) (x + x_size - 1),
                    (Screencoord) (y + y_size - 1),
                    (unsigned long *) ((void *)
                     (pixels->data.pixels_8bit_colour_index)) );
        break;

    case COLOUR_INDEX_16BIT_PIXEL:
        rectwrite( (Screencoord) x, (Screencoord) y,
                   (Screencoord) (x + x_size - 1),
                   (Screencoord) (y + y_size - 1),
                   (Colorindex *)
                    (pixels->data.pixels_16bit_colour_index) );
        break;
    default:
        break;
    }
}

public  void  GS_draw_rgb_pixels(
    Gwindow         window,
    pixels_struct   *pixels )
{
    int   x, y, x_size, y_size;

    x = pixels->x_position + window->x_viewport_min;
    y = pixels->y_position + window->y_viewport_min;
    x_size = pixels->x_size;
    y_size = pixels->y_size;

    lrectwrite( (Screencoord) x, (Screencoord) y,
                (Screencoord) (x + x_size - 1),
                (Screencoord) (y + y_size - 1),
                pixels->data.pixels_rgb );
}

/* ARGSUSED */

public  void  GS_read_pixels(
    Gwindow         window,
    int             x_min,
    int             x_max,
    int             y_min,
    int             y_max,
    Colour          pixels[] )
{
#ifdef  TWO_D_ONLY
    int   i, n_pixels;

    n_pixels = (x_max - x_min + 1) * (y_max - y_min + 1);

    for_less( i, 0, n_pixels )
        pixels[i] = BLACK;
#else
    int   i, n_pixels;

    readsource( SRC_FRONT );

    (void) lrectread( (Screencoord) x_min, (Screencoord) y_min,
                      (Screencoord) x_max, (Screencoord) y_max, pixels );

    n_pixels = (x_max - x_min + 1) * (y_max - y_min + 1);
    for_less( i, 0, n_pixels )
        pixels[i] |= BLACK;
#endif
}

private  const  STRING  font_name = "Helvetica";

private  fmfonthandle   base_font_handle;

private  BOOLEAN  fonts_present( void )
{
    return( base_font_handle != (fmfonthandle) 0 );
}

/* ARGSUSED */

public  void  WS_build_font_in_window(
    WS_window_struct     *window,
    int                  font_index,
    WS_font_info         *font_info )
{
}

/* ARGSUSED */

public  void  WS_delete_font_in_window(
    WS_window_struct     *window,
    int                  font_index,
    WS_font_info         *font_info )
{
}

/* ARGSUSED */

public  void  WS_delete_font(
    WS_font_info  *info )
{
}

public  BOOLEAN  WS_get_font(
    Font_types    type,
    Real          size,
    WS_font_info  *info )
{
    static  BOOLEAN  first = TRUE;
    BOOLEAN          found;

    if( first )
    {
        first = FALSE;
        fminit();

        base_font_handle = fmfindfont( font_name );
    }

    info->type = type;
    info->size = size;

    if( type == FIXED_FONT )
    {
        found = TRUE;
    }
    else if( fonts_present() )
    {
        info->font_handle = fmscalefont( base_font_handle, size );
        found = TRUE;
    }
    else
    {
        found = FALSE;
    }

    return( found );
}

public  Real  WS_get_text_length(
    WS_font_info    *font_info,
    STRING          str )
{
    Real   width;

    if( font_info->type == SIZED_FONT )
        width = (Real) fmgetstrwidth( font_info->font_handle, str );
    else
        width = (Real) strwidth( str );

    return( width );
}

public  Real  WS_get_character_height(
    WS_font_info    *font_info )
{
    Real   height;

    if( font_info->type == SIZED_FONT )
        height = font_info->size;
    else
        height = 18.0;

    return( height );
}

/* ARGSUSED */

public  BOOLEAN  GS_set_font(
    Gwindow            window,
    int                font_index,
    WS_font_info       *font )
{
    if( font->type != FIXED_FONT )
        fmsetfont( font->font_handle );

    return( TRUE );
}

public  void   GS_draw_text(
    Font_types   type,
    STRING       string )
{
    if( type == SIZED_FONT )
        fmprstr( string );
    else
        charstr( string );
}

