
#include  <internal_volume_io.h>
#include  <graphics.h>

#define  LIGHT_INDEX         0

#define  PIXELS_X_ZOOM       1.0
#define  PIXELS_Y_ZOOM       1.0

#define  N_FONTS             10
#define  N_FONTS_TO_DRAW      2

#define  ROTATE_CONTINUOUSLY
#undef  ROTATE_CONTINUOUSLY

typedef struct
{
    int              prev_rotation_mouse_x;
    BOOLEAN          in_rotation_mode;
    pixels_struct    pixels;
    lines_struct     lines;
    polygons_struct  polygons;
    Transform        modeling_transform;

}main_struct;

private  void  update(
    Gwindow    window,
    void       *update_data )
{
    main_struct   *info;

    info = (main_struct *) update_data;

    G_clear_window( window );

    G_set_zbuffer_state( window, OFF );
    G_set_lighting_state( window, OFF );
    G_set_view_type( window, PIXEL_VIEW );
    G_draw_pixels( window, &info->pixels );

    G_set_zbuffer_state( window, ON );
    G_set_lighting_state( window, ON );
    G_set_view_type( window, MODEL_VIEW );
    G_draw_polygons( window, &info->polygons );
    G_set_lighting_state( window, OFF );
    G_draw_lines( window, &info->lines );

/*
    G_set_lighting_state( window, OFF );
    G_set_view_type( window, PIXEL_VIEW );
    G_draw_lines( window, &info->lines_2d );
    G_draw_lines( window, &info->single_point );
    G_draw_text( window, &info->text );

    for_less( i, 0, N_FONTS_TO_DRAW )
        G_draw_text( window, &info->font_examples[i] );
*/

    G_update_window( window );
}

private  void  left_mouse_down(
    Gwindow    window,
    int        x,
    int        y,
    void       *update_data )
{
    main_struct   *info;

    info = (main_struct *) update_data;

    info->prev_rotation_mouse_x = x;
    info->in_rotation_mode = TRUE;
}

private  void  update_rotation(
    Gwindow       window,
    main_struct   *info,
    int           x )
{
    Real        angle_in_degrees;
    Transform   rotation_transform;
    Point       centre_of_rotation;

    angle_in_degrees = (Real) (info->prev_rotation_mouse_x - x);

    make_rotation_transform( angle_in_degrees * DEG_TO_RAD, Y,
                             &rotation_transform );

    fill_Point( centre_of_rotation, 0.3, 0.0, 0.0 );
    make_transform_relative_to_point( &centre_of_rotation,
                                      &rotation_transform,
                                      &rotation_transform );
    concat_transforms( &info->modeling_transform, &info->modeling_transform,
                       &rotation_transform );
    G_set_modeling_transform( window, &info->modeling_transform );

    info->prev_rotation_mouse_x = x;

    G_set_update_flag( window );
}

private  void  left_mouse_up(
    Gwindow    window,
    int        x,
    int        y,
    void       *update_data )
{
    main_struct   *info;

    info = (main_struct *) update_data;

    update_rotation( window, info, x );
    info->in_rotation_mode = FALSE;
}

private  void  mouse_movement(
    Gwindow    window,
    int        x,
    int        y,
    void       *update_data )
{
    main_struct   *info;

    info = (main_struct *) update_data;

    if( info->in_rotation_mode )
        update_rotation( window, info, x );
}

int main(
    int    argc,
    char   *argv[] )
{
    main_struct       info;
    Status            status;
    BOOLEAN           stereo_flag;
    Gwindow           window;
    text_struct       text;
    static Font_types font_types[N_FONTS] = {
                          FIXED_FONT,
                          SIZED_FONT,
                          SIZED_FONT,
                          SIZED_FONT,
                          SIZED_FONT,
                          SIZED_FONT,
                          SIZED_FONT,
                          SIZED_FONT,
                          SIZED_FONT,
                          SIZED_FONT };
    static Real       font_sizes[N_FONTS] = { 0.0,
                           6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0 };
    text_struct       font_examples[N_FONTS];
    lines_struct      lines_2d, single_point;
    static Surfprop   spr = { 0.2f, 0.5f, 0.5f, 20.0f, 1.0f };
    Point             point, centre_of_rotation;
    Vector            normal, light_direction;
    BOOLEAN           done;
    int               key_pressed;
    int               mouse_x, mouse_y, prev_mouse_x, prev_mouse_y;
    BOOLEAN           mouse_in_window, prev_mouse_in_window;
    int               x_position, y_position, x_size, y_size;
    int               x_pixel, y_pixel;
    BOOLEAN           in_rotation_mode;
    int               prev_rotation_mouse_x;
    Real              angle_in_degrees;
    int               i, j, pixels_x_size, pixels_y_size;
    Real              x, y, eye_separation;
    static Point      origin = { 0.0f, 0.0f, 2.0f };
    static Vector     up_direction = { 0.0f, 1.0f, 0.0f };
    static Vector     line_of_sight = { 0.0f, 0.0f, -1.0f };
    int               n_iters = 1;

    info.in_rotation_mode = FALSE;

    stereo_flag = (argc > 1);

    if( !stereo_flag || sscanf( argv[1], "%lf", &eye_separation ) != 1 )
        eye_separation = 0.0;

    status = G_create_window( "Test Window",
                              100, 600, 300, 300,
                              FALSE, TRUE, FALSE, 0, &window );

    G_set_transparency_state( window, ON );

    if( status != OK )
        return( 1 );

    if( stereo_flag )
        G_set_background_colour( window, WHITE );

    G_set_3D_view( window, &origin, &line_of_sight, &up_direction,
                   0.01, 4.0, ON, 2.0, stereo_flag, eye_separation, 2.0, 2.0 );

    fill_Point( point, -0.3, 0.3, 0.0 );
    G_transform_point( window, &point, MODEL_VIEW, &x_pixel, &y_pixel );

    print( "(%g,%g,%g) maps to %d %d in pixels\n",
            Point_x(point), Point_y(point), Point_z(point), x_pixel, y_pixel );

    /* ------- define text to be drawn (text.string filled in later ----- */

    fill_Point( point, 10.0, 10.0, 0.0 );
    initialize_text( &text, &point, make_Colour(255,0,255), SIZED_FONT, 14.0 );
    text.string = alloc_string( 2000 );

    for_less( i, 0, N_FONTS )
    {
        fill_Point( point, 10.0, 30.0 + (Real) i * 20.0, 0.0 );
        initialize_text( &font_examples[i], &point, make_Colour(0,255,255),
                         font_types[i], font_sizes[i] );

        if( font_types[i] == FIXED_FONT )
            font_examples[i].string = create_string( "Fixed font: " );
        else
        {
            font_examples[i].string = alloc_string( 1000 );
            (void) sprintf( font_examples[i].string, "Sized font %g points: ",
                            font_sizes[i] );
        }

        concat_to_string( &font_examples[i].string,
                          "abcdefghijklmnopqrstuvwxyz" );
    }

    /* ------------ define line to be drawn  ------------- */

    initialize_lines( &info.lines, make_Colour(0,255,0) );

    info.lines.n_points = 4;
    ALLOC( info.lines.points, 4 );
    fill_Point( info.lines.points[0], 0.0, 0.0, 0.0 );
    fill_Point( info.lines.points[1], 1.0, 1.0, 0.0 );
    fill_Point( info.lines.points[2], -0.3, 1.0, -1.0 );
    fill_Point( info.lines.points[3], 0.5, 0.4, 0.0 );

    info.lines.n_items = 3;
    ALLOC( info.lines.end_indices, info.lines.n_items );
    info.lines.end_indices[0] = 2;
    info.lines.end_indices[1] = 4;
    info.lines.end_indices[2] = 5;

    ALLOC( info.lines.indices, info.lines.end_indices[info.lines.n_items-1] );
    info.lines.indices[0] = 0;
    info.lines.indices[1] = 1;

    info.lines.indices[2] = 0;
    info.lines.indices[3] = 2;

    info.lines.indices[4] = 3;

    /* --- initialize single point */

    initialize_lines( &single_point, make_Colour(255,255,255) );

    single_point.n_points = 1;
    ALLOC( single_point.points, 4 );
    fill_Point( single_point.points[0], 0.0, 0.0, 0.0 );
    single_point.n_items = 1;
    ALLOC( single_point.end_indices, single_point.n_items );
    single_point.end_indices[0] = 1;
    ALLOC( single_point.indices,
           single_point.end_indices[single_point.n_items-1] );
    single_point.indices[0] = 0;

    /* ------------ define 2d line to be drawn  ------------- */

    initialize_lines( &lines_2d, make_Colour(0,255,150) );

    G_get_window_size( window, &x_size, &y_size );

    lines_2d.n_points = 4;
    ALLOC( lines_2d.points, 4 );
    fill_Point( lines_2d.points[0], 5.0, 5.0, 0.0 );
    fill_Point( lines_2d.points[1], (Real) x_size - 5.0, 5.0, 0.0 );
    fill_Point( lines_2d.points[2], (Real) x_size - 5.0, (Real) y_size - 5.0,
                                    0.0 );
    fill_Point( lines_2d.points[3], 5.0, (Real) y_size - 5.0, 0.0 );

    lines_2d.n_items = 1;
    ALLOC( lines_2d.end_indices, lines_2d.n_items );
    lines_2d.end_indices[0] = 5;

    ALLOC( lines_2d.indices, lines_2d.end_indices[lines_2d.n_items-1] );
    lines_2d.indices[0] = 0;
    lines_2d.indices[1] = 1;
    lines_2d.indices[2] = 2;
    lines_2d.indices[3] = 3;
    lines_2d.indices[4] = 0;

    /* ------------ define pixels to be drawn  ------------- */

    pixels_x_size = 250;
    pixels_y_size = 250;

    x_position = 10;
    y_position = 10;
    initialize_pixels( &info.pixels, x_position, y_position,
                       pixels_x_size, pixels_y_size,
                       PIXELS_X_ZOOM, PIXELS_Y_ZOOM, RGB_PIXEL );

    for_less( i, 0, pixels_x_size )
    {
        for_less( j, 0, pixels_y_size )
        {
            PIXEL_RGB_COLOUR(info.pixels,i,j) = make_Colour_0_1(
                      (Real) i / (Real) (pixels_x_size-1),
                      (Real) j / (Real) (pixels_y_size-1), 0.0 );
        }
    }

    /* ------------ define polygons to be drawn  ------------- */

    initialize_polygons( &info.polygons, make_Colour(0,255,255), &spr );

    start_new_polygon( &info.polygons );

    fill_Point( point, -0.3, -0.3, 0.0 );
    fill_Vector( normal, 0.0, 0.0, 1.0 );
    add_point_to_polygon( &info.polygons, &point, &normal );
    fill_Point( point, 0.3, -0.3, 0.0 );
    add_point_to_polygon( &info.polygons, &point, &normal );
    fill_Point( point, 0.3, 0.3, 0.0 );
    add_point_to_polygon( &info.polygons, &point, &normal );
    fill_Point( point, -0.3, 0.3, 0.0 );
    add_point_to_polygon( &info.polygons, &point, &normal );


    /* ------------ define lights ----------------- */

    fill_Vector( light_direction, 1.0, 1.0, -1.0 );/* from over left shoulder */

    G_define_light( window, LIGHT_INDEX, DIRECTIONAL_LIGHT,
                    make_Colour(255,255,255),
                    &light_direction, (Point *) 0, 0.0, 0.0 );
    G_set_light_state( window, LIGHT_INDEX, ON );

    /* --------------------------------------- */
    /* ------------ do main loop ------------- */
    /* --------------------------------------- */

    make_identity_transform( &info.modeling_transform );

    in_rotation_mode = FALSE;

    prev_mouse_x = -1;
    prev_mouse_y = -1;
    prev_mouse_in_window = -100;

    print( "Hold down left button and move mouse to rotate\n" );
    print( "Hit ESC key to exit\n" );

    G_set_update_function( window, update, (void *) &info );
    G_set_left_mouse_down_function( window, left_mouse_down, (void *) &info );
    G_set_left_mouse_up_function( window, left_mouse_up, (void *) &info );
    G_set_mouse_movement_function( window, mouse_movement, (void *) &info );

    G_main_loop();

    done = FALSE;

#ifdef old
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
                    print( "Key pressed down: \"%c\" (%d)\n", key_pressed,
                            key_pressed );
                    if( key_pressed == '' )
                        done = TRUE;
                    break;

                case KEY_UP_EVENT:
                    print( "Key released: \"%c\"   (%d)\n", key_pressed,
                            key_pressed );
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
                    (void) G_get_mouse_position_0_to_1( window, &x, &y );
                    if( x < 0.5 )
                        x = x * 4.0 - 0.5;
                    else
                        x = (Real) x_size - 2.5 + 4.0 * (x - 0.5);

                    if( y < 0.5 )
                        y = y * 4.0 - 0.5;
                    else
                        y = (Real) y_size - 2.5 + 4.0 * (y - 0.5);

                    fill_Point( single_point.points[0], x, y, 0.0 );
                    update_required = TRUE;
                    break;

                case RIGHT_MOUSE_UP_EVENT:
                    break;

                case MIDDLE_MOUSE_DOWN_EVENT:
                    print( "Middle mouse DOWN\n" );
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
                    fill_Point( lines_2d.points[0], 5.0, 5.0, 0.0 );
                    fill_Point( lines_2d.points[1], (Real) x_size - 5.0, 5.0, 0.0 );
                    fill_Point( lines_2d.points[2], (Real) x_size - 5.0, (Real) y_size - 5.0,
                                                    0.0 );
                    fill_Point( lines_2d.points[3], 5.0, (Real) y_size - 5.0, 0.0 );
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

#ifdef  ROTATE_CONTINUOUSLY
        {
            angle_in_degrees = 10.0;
#else
        if( in_rotation_mode &&
            G_get_mouse_position( window, &mouse_x, &mouse_y ) &&
            mouse_x != prev_rotation_mouse_x )
        {
            angle_in_degrees = (Real) (prev_rotation_mouse_x - mouse_x);
#endif

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
            G_clear_window( window );

            G_set_zbuffer_state( window, OFF );
            G_set_lighting_state( window, OFF );
            G_set_view_type( window, PIXEL_VIEW );
            for_less( i, 0, n_iters )
                G_draw_pixels( window, &pixels );

            G_set_zbuffer_state( window, ON );
            G_set_lighting_state( window, ON );
            G_set_view_type( window, MODEL_VIEW );
            G_draw_polygons( window, &polygons );
            G_set_lighting_state( window, OFF );
            G_draw_lines( window, &lines );

            G_set_lighting_state( window, OFF );
            G_set_view_type( window, PIXEL_VIEW );
            G_draw_lines( window, &lines_2d );
            G_draw_lines( window, &single_point );
            G_draw_text( window, &text );

            for_less( i, 0, N_FONTS_TO_DRAW )
                G_draw_text( window, &font_examples[i] );

            G_update_window( window );
            update_required = FALSE;
        }
    }
    while( !done );
#endif

    /* delete drawing objects and window (text does not need to be deleted */

    delete_lines( &info.lines );
    delete_lines( &lines_2d );
    delete_lines( &single_point );

    delete_polygons( &info.polygons );

    delete_pixels( &info.pixels );

    delete_text( &text );

    for_less( i, 0, N_FONTS )
        delete_text( &font_examples[i] );

    status = G_delete_window( window );

    G_terminate();

    output_alloc_to_file( NULL );

    return( status != OK );
}
