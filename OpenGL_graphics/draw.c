
#include  <internal_volume_io.h>
#include  <graphics.h>
#include  <random_order.h>

public  void  GS_set_point(
    Point  *point )
{
    glVertex3fv( point->coords );
}

public  void  GS_set_normal(
    Vector  *normal )
{
    glNormal3fv( normal->coords );
}

public  void  GS_set_colour(
    Colour  colour )
{
    void   *void_ptr;

    void_ptr = (void *) &colour;

    glColor4ubv( void_ptr );
}

public  void  GS_set_colour_index(
    Colour  colour )
{
    glIndexi( (GLint) colour );
}

public  void  GS_set_ambient_and_diffuse_mode(
    BOOLEAN  state )
{
    if( state )
    {
        glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
        glEnable( GL_COLOR_MATERIAL );
    }
    else
        glDisable( GL_COLOR_MATERIAL );
}

public  void  GS_initialize_surface_property(
    Gwindow        window )   /* ARGSUSED */
{
}

public  void  GS_set_surface_property(
    Gwindow        window,
    Colour         col,
    Surfprop       *surfprop )   /* ARGSUSED */
{
#ifndef  TWO_D_ONLY
    float  props[4];

    props[0] = Surfprop_a( *surfprop ) * get_Colour_r_0_1( col );
    props[1] = Surfprop_a( *surfprop ) * get_Colour_g_0_1( col );
    props[2] = Surfprop_a( *surfprop ) * get_Colour_b_0_1( col );
    props[3] = get_Colour_a_0_1( col );
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, props );

    props[0] = Surfprop_d( *surfprop ) * get_Colour_r_0_1( col );
    props[1] = Surfprop_d( *surfprop ) * get_Colour_g_0_1( col );
    props[2] = Surfprop_d( *surfprop ) * get_Colour_b_0_1( col );
    props[3] = get_Colour_a_0_1( col );
    glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, props );

    props[0] = Surfprop_s( *surfprop );
    props[1] = Surfprop_s( *surfprop );
    props[2] = Surfprop_s( *surfprop );
    props[3] = 1.0;
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, props );

    props[0] = Surfprop_se( *surfprop );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, props );
#endif
}

public  void  GS_set_line_width(
    Real  line_width )
{
    glLineWidth( (GLfloat) line_width );
}

public  void  GS_curve(
    float  geom[4][3] )  /* ARGSUSED */
{
}

public  void  GS_begin_point()
{
    glBegin( GL_POINTS );
}

public  void  GS_end_point()
{
    glEnd();
}

public  void  GS_begin_line()
{
    glBegin( GL_LINE_STRIP );
}

public  void  GS_end_line()
{
    glEnd();
}

public  void  GS_begin_closed_line()
{
    glBegin( GL_LINE_LOOP );
}

public  void  GS_end_closed_line()
{
    glEnd();
}

public  void  GS_begin_polygon()
{
    glBegin( GL_POLYGON );
}

public  void  GS_end_polygon()
{
    glEnd();
}

public  void  GS_begin_quad_strip()
{
    glBegin( GL_QUAD_STRIP );
}

public  void  GS_end_quad_strip()
{
    glEnd();
}

public  void  GS_set_raster_position(
    Real  x,
    Real  y,
    Real  z )
{
    glRasterPos3f( x, y, z );
}

public  void  GS_set_font(
    WS_font_info         *font_info )
{
    glListBase( font_info->listBase );
}

public  void  GS_draw_text(
    Font_types   type,
    char         string[] )    /* ARGSUSED */
{
    glCallLists( strlen( string ), GL_BYTE, string );
}

public  void  GS_set_pixel_zoom(
    Real  x_zoom,
    Real  y_zoom )
{
    glPixelZoom( (GLfloat) x_zoom, (GLfloat) y_zoom );
}

public  void  GS_draw_colour_map_pixels(
    Gwindow         window,
    pixels_struct   *pixels )    /* ARGSUSED */
{
    int       x, y, x_size, y_size;

    x = pixels->x_position;
    y = pixels->y_position;
    x_size = pixels->x_size;
    y_size = pixels->y_size;

    GS_set_raster_position( (Real) x, (Real) y, 0.0 );

    switch( pixels->pixel_type )
    {
    case COLOUR_INDEX_8BIT_PIXEL:
        glDrawPixels( x_size, y_size, GL_COLOR_INDEX,
                      GL_UNSIGNED_BYTE,
                      (void *) &PIXEL_COLOUR_INDEX_8(*pixels,0,0) );
        break;

    case COLOUR_INDEX_16BIT_PIXEL:
        glDrawPixels( x_size, y_size, GL_COLOR_INDEX,
                      GL_UNSIGNED_SHORT,
                      (void *) &PIXEL_COLOUR_INDEX_16(*pixels,0,0) );
        break;
    default:
        break;
    }
}

public  void  GS_draw_rgb_pixels(
    Gwindow         window,
    pixels_struct   *pixels )    /* ARGSUSED */
{
    int       x, y, x_size, y_size, n_pixels;
    void      *void_ptr;

    x = pixels->x_position;
    y = pixels->y_position;
    x_size = pixels->x_size;
    y_size = pixels->y_size;

    GS_set_raster_position( (Real) x, (Real) y, 0.0 );

    void_ptr = (void *) &PIXEL_RGB_COLOUR(*pixels,0,0);
    glDrawPixels( x_size, y_size, GL_RGBA, GL_UNSIGNED_BYTE, void_ptr );
}

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
    glReadBuffer( GL_FRONT );

    if( window->colour_map_state )
    {
        glReadPixels( x_min, y_min, x_max - x_min + 1, y_max - y_min + 1,
                      GL_COLOR_INDEX, GL_UNSIGNED_INT, pixels );
    }
    else
    {
        glReadPixels( x_min, y_min, x_max - x_min + 1, y_max - y_min + 1,
                      GL_RGBA, GL_UNSIGNED_BYTE, pixels );
    }
#endif
}
