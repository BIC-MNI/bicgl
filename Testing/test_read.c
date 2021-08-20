#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H  

#include  <graphics.h>

#define  LIGHT_INDEX         0

#define  PIXELS_X_ZOOM       2.0
#define  PIXELS_Y_ZOOM       2.0

static  void  check_pixels( window_struct *window );

void  copy_pixels();

int main(int argc,char **argv)
{
    VIO_Status            status;
    window_struct     *window, *event_window;
    text_struct       text;
    lines_struct      lines;
    polygons_struct   polygons;
    pixels_struct     pixels;
    static VIO_Surfprop   spr = { 0.2, 0.5, 0.5, 20.0, 1.0 };
    VIO_Point             point, centre_of_rotation;
    VIO_Vector            normal, light_direction;
    Event_types       event_type;
    VIO_BOOL           update_required, done;
    int               key_pressed;
    int               mouse_x, mouse_y, prev_mouse_x, prev_mouse_y;
    VIO_BOOL           mouse_in_window, prev_mouse_in_window;
    int               x_position, y_position, x_size, y_size;
    int               x_pixel, y_pixel;
    VIO_BOOL           in_rotation_mode;
    int               prev_rotation_mouse_x;
    VIO_Real              angle_in_degrees;
    int               i, j, pixels_x_size, pixels_y_size;
    VIO_Real              x, y;
    VIO_Transform         modeling_transform, rotation_transform;
    static VIO_Point      origin = { 0.0, 0.0, 2.0 };
    static VIO_Vector     up_direction = { 0.0, 1.0, 0.0 };
    static VIO_Vector     line_of_sight = { 0.0, 0.0, -1.0 };

    status = G_create_window( "Test Window", -1, -1, -1, -1, &window );

    G_set_background_colour( window, LIGHT_BLUE );

    G_set_zbuffer_state( window, FALSE );
    G_set_double_buffer_state( window, FALSE );

    G_set_3D_view( window, &origin, &line_of_sight, &up_direction,
                   0.01, 4.0, TRUE, 2.0, 2.0, 2.0 );

    fill_Point( point, -0.3, 0.3, 0.0 );
    G_transform_point( window, &point, MODEL_VIEW, &x_pixel, &y_pixel );

    print( "(%g,%g,%g) maps to %d %d in pixels\n",
            Point_x(point), Point_y(point), Point_z(point), x_pixel, y_pixel );

    /* ------- define text to be drawn (text.string filled in later ----- */

    fill_Point( point, 10.0, 10.0, 0.0 );
    initialize_text( &text, &point, make_Colour(255,255,0), SIZED_FONT, 14.0 );
    text.string[0] = (char) 0;

    /* ------------ define line to be drawn  ------------- */

    initialize_lines( &lines, make_Colour(255,255,0) );

    lines.n_points = 4;
    ALLOC( lines.points, 4 );
    fill_Point( lines.points[0], 0.0, 0.0, 0.0 );
    fill_Point( lines.points[1], 1.0, 1.0, 0.0 );
    fill_Point( lines.points[2], -0.3, 1.0, -1.0 );
    fill_Point( lines.points[3], 0.5, 0.4, 0.0 );

    lines.n_items = 3;
    ALLOC( lines.end_indices, lines.n_items );
    lines.end_indices[0] = 2;
    lines.end_indices[1] = 4;
    lines.end_indices[2] = 5;

    ALLOC( lines.indices, lines.end_indices[lines.n_items-1] );
    lines.indices[0] = 0;
    lines.indices[1] = 1;

    lines.indices[2] = 0;
    lines.indices[3] = 2;

    lines.indices[4] = 3;

    /* ------------ define pixels to be drawn  ------------- */

    pixels_x_size = 256;
    pixels_y_size = 256;

    x_position = 10;
    y_position = 10;
    initialize_pixels( &pixels, x_position, y_position,
                       pixels_x_size, pixels_y_size,
                       PIXELS_X_ZOOM, PIXELS_Y_ZOOM, RGB_PIXEL );

    for_less( i, 0, pixels_x_size )
    {
        for_less( j, 0, pixels_y_size )
        {
            PIXEL_RGB_COLOUR(pixels,i,j) = make_Colour( i % 256, j % 256, 0 );
        }
    }

    /* ------------ define polygons to be drawn  ------------- */

    initialize_polygons( &polygons, make_Colour(0,255,255), &spr );

    start_new_polygon( &polygons );

    fill_Point( point, -0.3, -0.3, 0.0 );
    fill_Vector( normal, 0.0, 0.0, 1.0 );
    add_point_to_polygon( &polygons, &point, &normal );
    fill_Point( point, 0.3, -0.3, 0.0 );
    add_point_to_polygon( &polygons, &point, &normal );
    fill_Point( point, 0.3, 0.3, 0.0 );
    add_point_to_polygon( &polygons, &point, &normal );
    fill_Point( point, -0.3, 0.3, 0.0 );
    add_point_to_polygon( &polygons, &point, &normal );

    /* ------------ define lights ----------------- */

    fill_Vector( light_direction, 1.0, 1.0, -1.0 );/* from over left shoulder */

    G_define_light( window, LIGHT_INDEX, DIRECTIONAL_LIGHT,
                    make_Colour(255,255,255),
                    &light_direction, (VIO_Point *) 0, 0.0, 0.0 );
    G_set_light_state( window, LIGHT_INDEX, TRUE );

    G_set_lighting_state( window, TRUE );

    /* --------------------------------------- */
    /* ------------ do main loop ------------- */
    /* --------------------------------------- */

    make_identity_transform( &modeling_transform );
    update_required = TRUE;

    in_rotation_mode = FALSE;

    prev_mouse_x = -1;
    prev_mouse_y = -1;
    prev_mouse_in_window = -100;

    print( "Hold down left button and move mouse to rotate\n" );
    print( "Hit middle mouse button to exit\n" );

    done = FALSE;

    do
    {
        do
        {
            event_type = G_get_event( &event_window, &key_pressed );

            if( event_window == window )
            {
                switch( event_type )
                {
                case KEY_DOWN_EVENT:
                    print( "Key pressed down: \"%c\"\n", key_pressed );
                    break;

                case KEY_UP_EVENT:
                    print( "Key released: \"%c\"\n", key_pressed );
                    break;

                case LEFT_MOUSE_DOWN_EVENT:
                    (void) G_get_mouse_position( window, &prev_rotation_mouse_x,
                                                 &mouse_y );
                    in_rotation_mode = TRUE;
                    break;

                case LEFT_MOUSE_UP_EVENT:
                    in_rotation_mode = FALSE;
                    update_required = TRUE;
                    break;

                case RIGHT_MOUSE_DOWN_EVENT:
                    break;

                case RIGHT_MOUSE_UP_EVENT:
                    break;

                case MIDDLE_MOUSE_DOWN_EVENT:
                    print( "Middle mouse DOWN\n" );
                    done = TRUE;
                    break;

                case MIDDLE_MOUSE_UP_EVENT:
                    print( "Middle mouse UP\n" );
                    break;

                case WINDOW_REDRAW_EVENT:
                    print( "Window needs to be redrawn.\n" );
                    update_required = TRUE;
                    break;

                case WINDOW_RESIZE_EVENT:
                    G_get_window_position( window, &x_position, &y_position );
                    G_get_window_size( window, &x_size, &y_size );
                    print( "Window resized, " );
                    print( " new position: %d %d   New size: %d %d\n",
                            x_position, y_position, x_size, y_size );
                    update_required = TRUE;
                    break;
                }
            }
        }                  /* break to do update when no events */
        while( event_type != NO_EVENT );

        /* check if in rotation mode and moved mouse horizontally */

        if( in_rotation_mode &&
            G_get_mouse_position( window, &mouse_x, &mouse_y ) &&
            mouse_x != prev_rotation_mouse_x )
        {
            angle_in_degrees = (prev_rotation_mouse_x - mouse_x);

            make_rotation_transform( angle_in_degrees * DEG_TO_RAD, Y,
                                     &rotation_transform );

            fill_Point( centre_of_rotation, 0.3, 0.0, 0.0 );
            make_transform_relative_to_point( &centre_of_rotation,
                                              &rotation_transform,
                                              &rotation_transform );
            concat_transforms( &modeling_transform, &modeling_transform,
                               &rotation_transform );
            G_set_modeling_transform( window, &modeling_transform );

            prev_rotation_mouse_x = mouse_x;

            update_required = TRUE;
        }

        mouse_in_window = G_get_mouse_position( window, &mouse_x, &mouse_y );

        if( mouse_in_window != prev_mouse_in_window ||
            mouse_in_window && (mouse_x != prev_mouse_x ||
                                mouse_y != prev_mouse_y) )
        {
            prev_mouse_in_window = mouse_in_window;

            if( mouse_in_window )
            {
                (void) G_get_mouse_position_0_to_1( window, &x, &y );
                (void) sprintf( text.string,
                                "Mouse: %4d,%4d pixels   %4.2f,%4.2f window",
                                mouse_x, mouse_y, x, y );
                prev_mouse_x = mouse_x;
                prev_mouse_y = mouse_y;
            }
            else
            {
                (void) sprintf( text.string, "Mouse:  out of window.\n" );
            }

            update_required = TRUE;
        }

        /* if one or more events caused an update, redraw the screen */

        if( update_required )
        {
            G_draw_pixels( window, &pixels );

            G_set_view_type( window, MODEL_VIEW );
            G_draw_polygons( window, &polygons );
            G_draw_lines( window, &lines );

            G_set_view_type( window, PIXEL_VIEW );
            G_draw_text( window, &text );

            G_update_window( window );

/*
   copy_pixels();
*/

   check_pixels( window );

            update_required = FALSE;
        }
    }
    while( !done );

    /* delete drawing objects and window (text does not need to be deleted */

    delete_lines( &lines );

    delete_polygons( &polygons );

    delete_pixels( &pixels );

    status = G_delete_window( window );

    return( status != VIO_OK );
}

static  void  check_pixels( window_struct *window )
{
    int            x_size, y_size;
    pixels_struct  pixels;

    G_get_window_size( window, &x_size, &y_size );

    initialize_pixels( &pixels, 100, 100, 100, 100, 1.0, 1.0,
                       RGB_PIXEL );

    G_set_bitplanes( window, NORMAL_PLANES );
/*
    copy_pixels();
*/

    G_read_pixels( window, 0, 99, 0, 99, pixels.data.pixels_rgb );
    G_draw_pixels( window, &pixels );
    G_update_window( window );

    delete_pixels( &pixels );
}
