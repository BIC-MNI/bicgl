
#include  <internal_volume_io.h>
#include  <GS_graphics.h>

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

private  long  get_unique_lmdef_id( void )
{
    static  long   unique_lmdef_id = 1;

    unique_lmdef_id += 8;
    if( unique_lmdef_id > 60000 )
        unique_lmdef_id = 1;

    return( unique_lmdef_id );
}

public  void  GS_initialize_surface_property(
    GSwindow        window )
{
    static  Surfprop  spr = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    window->unique_lmdef_id = get_unique_lmdef_id();
    GS_set_surface_property( window, WHITE, &spr );
}

public  void  GS_set_surface_property(
    GSwindow       window,
    Colour         col,
    Surfprop       *surfprop )
{
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

    lmdef( DEFMATERIAL, (short) window->unique_lmdef_id,
           SIZEOF_STATIC_ARRAY(props), props );
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
    int             x_viewport_min,
    int             y_viewport_min,
    pixels_struct   *pixels )
{
    int   x, y, x_size, y_size;

    x = pixels->x_position + x_viewport_min;
    y = pixels->y_position + y_viewport_min;
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
    int             x_viewport_min,
    int             y_viewport_min,
    pixels_struct   *pixels )
{
    int   x, y, x_size, y_size;

    x = pixels->x_position + x_viewport_min;
    y = pixels->y_position + y_viewport_min;
    x_size = pixels->x_size;
    y_size = pixels->y_size;

    lrectwrite( (Screencoord) x, (Screencoord) y,
                (Screencoord) (x + x_size - 1),
                (Screencoord) (y + y_size - 1),
                pixels->data.pixels_rgb );
}

/* ARGSUSED */

public  void  GS_read_pixels(
    BOOLEAN         colour_map_state,
    int             x_min,
    int             x_max,
    int             y_min,
    int             y_max,
    Colour          pixels[] )
{
    int   i, n_pixels;

    readsource( SRC_FRONT );

    if( colour_map_state )
    {
        (void) rectread( (Screencoord) x_min, (Screencoord) y_min,
                         (Screencoord) x_max, (Screencoord) y_max,
                         (Colorindex *) pixels );
    }
    else
    {
        (void) lrectread( (Screencoord) x_min, (Screencoord) y_min,
                          (Screencoord) x_max, (Screencoord) y_max, pixels );

        n_pixels = (x_max - x_min + 1) * (y_max - y_min + 1);
        for_less( i, 0, n_pixels )
            pixels[i] |= BLACK;
    }
}

private  const  STRING  font_name = "Helvetica";

private  fmfonthandle   base_font_handle = 0;

private  struct
{
    Real           size;
    fmfonthandle   font;
}
*fonts = NULL;

private  int  n_fonts = 0;
private  int  current_font_index = 0;

private  BOOLEAN  fonts_present( void )
{
    return( base_font_handle != (fmfonthandle) 0 );
}

private  fmfonthandle  lookup_font(
    Font_types    font_type,
    Real          size )
{
    static  BOOLEAN  first = TRUE;

    if( first )
    {
        first = FALSE;
        fminit();

        base_font_handle = fmfindfont( font_name );
    }

    if( font_type == FIXED_FONT || !fonts_present() )
        return( (fmfonthandle) 0 );

    if( current_font_index <= 0 || current_font_index >= n_fonts ||
        fonts[current_font_index].size != size )
    {
        for_less( current_font_index, 0, n_fonts )
        {
            if( fonts[current_font_index].size == size )
                break;
        }
    }

    if( current_font_index >= n_fonts )
    {
        SET_ARRAY_SIZE( fonts, n_fonts, n_fonts+1, 1 );
        fonts[current_font_index].size = size;
        fonts[current_font_index].font = fmscalefont( base_font_handle, size );
    }

    return( fonts[current_font_index].font );
}

public  Real  GS_get_text_length(
    STRING          str,
    Font_types      font_type,
    Real            font_size )
{
    fmfonthandle   font;
    Real           width;

    font = lookup_font( font_type, font_size );

    if( font != 0 )
        width = (Real) fmgetstrwidth( font, str );
    else
        width = (Real) strwidth( str );

    return( width );
}

public  Real  GS_get_character_height(
    Font_types      font_type,
    Real            font_size )
{
    int            i;
    fmfonthandle   font;
    Real           height;
    static struct
    {
        int   declared_size;
        int   actual_size;
    }              some_corrections[] = 
    {
        { 6, 7 },
        { 7, 8 },
        { 8, 8 },
        { 9, 9 },
        { 10, 11 },
        { 11, 11 },
        { 12, 12 },
        { 13, 12 },
        { 14, 14 },
        { 15, 14 },
        { 18, 19 },
        { 24, 24 }
    };

    font = lookup_font( font_type, font_size );

    if( font != 0 )
    {
        for_less( i, 0, SIZEOF_STATIC_ARRAY( some_corrections ) )
        {
            if( some_corrections[i].declared_size == (int) font_size )
                break;
        }

        if( i < SIZEOF_STATIC_ARRAY( some_corrections ) )
            height = (Real) some_corrections[i].actual_size;
        else
            height = font_size;
    }
    else
        height = 10.0;

    return( height );
}

public  void   GS_draw_text(
    Font_types   font_type,
    Real         font_size,
    STRING       string )
{
    fmfonthandle   font;

    if( font_type == SIZED_FONT )
    {
        font = lookup_font( font_type, font_size );
        fmsetfont( font );
        fmprstr( string );
    }
    else
        charstr( string );
}

