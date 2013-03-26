#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H  

#include  <volume_io.h>
#include  <GS_graphics.h>

  void  GS_set_point(
    VIO_Point  *point )
{
    glVertex3fv( point->coords );
}

  void  GS_set_normal(
    VIO_Vector  *normal )
{
    glNormal3fv( normal->coords );
}

  void  GS_set_colour(
    VIO_Colour  colour )
{
    glColor4ubv( (GLubyte*) &colour );
}

  void  GS_set_colour_index(
    VIO_Colour  colour )
{
    glIndexi( (GLint) colour );
}

  void  GS_set_ambient_and_diffuse_mode(
    VIO_BOOL  state )
{
    if( state )
    {
        glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
        glEnable( GL_COLOR_MATERIAL );
    }
    else
        glDisable( GL_COLOR_MATERIAL );
}

/* ARGSUSED */

  void  GS_initialize_surface_property(
    GSwindow        window )
{
}

/* ARGSUSED */

  void  GS_set_surface_property(
    GSwindow       window,
    VIO_Colour         col,
    VIO_Surfprop       *surfprop )
{
    float  props[4];
    VIO_Real   a, d, s;

    a = (VIO_Real) Surfprop_a( *surfprop );
    d = (VIO_Real) Surfprop_d( *surfprop );
    s = (VIO_Real) Surfprop_s( *surfprop );

    props[0] = (float) (a * get_Colour_r_0_1( col ));
    props[1] = (float) (a * get_Colour_g_0_1( col ));
    props[2] = (float) (a * get_Colour_b_0_1( col ));
    props[3] = (float) get_Colour_a_0_1( col );
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, props );

    props[0] = (float) (d * get_Colour_r_0_1( col ));
    props[1] = (float) (d * get_Colour_g_0_1( col ));
    props[2] = (float) (d * get_Colour_b_0_1( col ));
    props[3] = (float) get_Colour_a_0_1( col );
    glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, props );

    props[0] = (float) s;
    props[1] = (float) s;
    props[2] = (float) s;
    props[3] = 1.0f;
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, props );

    props[0] = Surfprop_se( *surfprop );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, props );
}

  void  GS_set_line_width(
    VIO_Real  line_width )
{
    glLineWidth( (GLfloat) line_width );
}

/* ARGSUSED */

  void  GS_curve(
    float  geom[4][3] )
{
}

  void  GS_begin_point( void )
{
    glBegin( GL_POINTS );
}

  void  GS_end_point( void )
{
    glEnd();
}

  void  GS_begin_line( void )
{
    glBegin( GL_LINE_STRIP );
}

  void  GS_end_line( void )
{
    glEnd();
}

  void  GS_begin_closed_line( void )
{
    glBegin( GL_LINE_LOOP );
}

  void  GS_end_closed_line( void )
{
    glEnd();
}

  void  GS_begin_polygon( void )
{
    glBegin( GL_POLYGON );
}

  void  GS_end_polygon( void )
{
    glEnd();
}

  void  GS_begin_quad_strip( void )
{
    glBegin( GL_QUAD_STRIP );
}

  void  GS_end_quad_strip( void )
{
    glEnd();
}

  void  GS_set_raster_position(
    VIO_Real  x,
    VIO_Real  y,
    VIO_Real  z )
{
    glRasterPos3d( (double) x, (double) y, (double) z );
}

  void  GS_set_pixel_zoom(
    VIO_Real  x_zoom,
    VIO_Real  y_zoom )
{
    glPixelZoom( (GLfloat) x_zoom, (GLfloat) y_zoom );
}

/* ARGSUSED */

  void  GS_draw_colour_map_pixels(
    int             x_viewport_min,
    int             y_viewport_min,
    pixels_struct   *pixels )
{
    int       x, y, x_size, y_size;

    x = pixels->x_position;
    y = pixels->y_position;
    x_size = pixels->x_size;
    y_size = pixels->y_size;

    GS_set_raster_position( (VIO_Real) x, (VIO_Real) y, 0.0 );

    switch( pixels->pixel_type )
    {
    case COLOUR_INDEX_8BIT_PIXEL:
        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
        glDrawPixels( x_size, y_size, GL_COLOR_INDEX,
                      GL_UNSIGNED_BYTE,
                      (void *) &PIXEL_COLOUR_INDEX_8(*pixels,0,0) );
        break;

    case COLOUR_INDEX_16BIT_PIXEL:
        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
        glDrawPixels( x_size, y_size, GL_COLOR_INDEX,
                      GL_UNSIGNED_SHORT,
                      (void *) &PIXEL_COLOUR_INDEX_16(*pixels,0,0) );
        break;
    default:
        break;
    }
}

/* ARGSUSED */

  void  GS_draw_rgb_pixels(
    int             x_viewport_min,
    int             y_viewport_min,
    pixels_struct   *pixels )
{
    int       x, y, x_size, y_size;
    void      *void_ptr;

    x = pixels->x_position;
    y = pixels->y_position;
    x_size = pixels->x_size;
    y_size = pixels->y_size;

    GS_set_raster_position( (VIO_Real) x, (VIO_Real) y, 0.0 );

    void_ptr = (void *) &PIXEL_RGB_COLOUR(*pixels,0,0);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glDrawPixels( x_size, y_size, GL_RGBA, GL_UNSIGNED_BYTE, void_ptr );
}

  void  GS_read_pixels(
    VIO_BOOL         colour_map_state,
    int             x_min,
    int             x_max,
    int             y_min,
    int             y_max,
    VIO_Colour          pixels[] )
{
    glReadBuffer( GL_FRONT );

    if( colour_map_state )
    {
        glReadPixels( x_min, y_min, x_max - x_min + 1, y_max - y_min + 1,
                      GL_COLOR_INDEX, GL_UNSIGNED_INT, pixels );
    }
    else
    {
        glReadPixels( x_min, y_min, x_max - x_min + 1, y_max - y_min + 1,
                      GL_RGBA, GL_UNSIGNED_BYTE, pixels );
    }
}

  VIO_Real  GS_get_character_height(
    Font_types       type,
    VIO_Real             size )
{
    return( WS_get_character_height( type, size ) );
}

/* ARGSUSED */

  void  GS_draw_text(
    Font_types   type,
    VIO_Real         size,
    VIO_STR       string )
{
    WS_draw_text( type, size, string );
}

  VIO_Real  GS_get_text_length(
    VIO_STR           str,
    Font_types       type,
    VIO_Real             size )
{
    return( WS_get_text_length( str, type, size ) );
}
