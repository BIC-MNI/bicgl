#include  <def_graphics.h>

main()
{
    Status            status;
    window_struct     *window;
    text_struct       text;
    lines_struct      lines;
    polygons_struct   polygons;
    pixels_struct     pixels;
    Point             point;
    int               i, j, pixels_x_size, pixels_y_size, x_position, y_position;

    status = G_create_window( "Test Window", -1, -1, -1, -1, &window );

    /* ------- define text to be drawn (text.string filled in later ----- */

    fill_Point( point, 10.0, 10.0, 0.0 );
    initialize_text( &text, &point, make_Colour(255,0,255), SIZED_FONT, 14.0 );
    text.string[0] = (char) 0;

    /* ------------ define line to be drawn  ------------- */

    initialize_lines( &lines, make_Colour(255,255,0) );

    lines.n_points = 4;
    ALLOC( lines.points, 4 );
    fill_Point( lines.points[0], 0.0, 0.0, 0.0 );
    fill_Point( lines.points[1], 100.0, 100.0, 0.0 );
    fill_Point( lines.points[2], 30.0, 100.0, 0.0 );
    fill_Point( lines.points[3], 100.0, 30.0, 0.0 );

    lines.n_items = 3;
    ALLOC( lines.end_indices, lines.n_items );
    lines.end_indices[0] = 2;
    lines.end_indices[1] = 4;
    lines.end_indices[2] = 6;

    ALLOC( lines.indices, lines.end_indices[lines.n_items-1] );
    lines.indices[0] = 0;
    lines.indices[1] = 1;

    lines.indices[2] = 0;
    lines.indices[3] = 2;

    lines.indices[4] = 0;
    lines.indices[5] = 3;

    /* ------------ define pixels to be drawn  ------------- */

    pixels_x_size = 256;
    pixels_y_size = 256;

    x_position = 10;
    y_position = 10;
    initialize_pixels( &pixels, x_position, y_position,
                       pixels_x_size, pixels_y_size, 1.0, 1.0, RGB_PIXEL );

    for_less( i, 0, pixels_x_size )
    {
        for_less( j, 0, pixels_y_size )
        {
            PIXEL_RGB_COLOUR(pixels,i,j) = make_Colour( i % 256, j % 256, 0 );
        }
    }

    /* ------------ define polygons to be drawn  ------------- */

    initialize_polygons( &polygons, make_Colour(0,255,255), (Surfprop *) NULL );

    fill_Point( point, 10.0, 10.0, 0.0 );
    add_point_to_polygon( &polygons, &point, (Vector *) 0 );
    fill_Point( point, 90.0, 10.0, 0.0 );
    add_point_to_polygon( &polygons, &point, (Vector *) 0 );
    fill_Point( point, 90.0, 90.0, 0.0 );
    add_point_to_polygon( &polygons, &point, (Vector *) 0 );
    fill_Point( point, 10.0, 90.0, 0.0 );
    add_point_to_polygon( &polygons, &point, (Vector *) 0 );

    G_set_viewport( window, 100, 300, 100, 300 );

/*
    G_draw_pixels( window, &pixels );
*/

    G_set_view_type( window, PIXEL_VIEW );
/*
    ortho2( -0.5, 200.5, -0.5, 200.5 );
    viewport( 100, 300, 100, 300 );
*/
    G_draw_polygons( window, &polygons );
    G_draw_lines( window, &lines );

    G_set_view_type( window, PIXEL_VIEW );
    G_draw_text( window, &text );

    G_update_window( window );

    print( "Hit return: " );
    (void) getchar();

    /* delete drawing objects and window (text does not need to be deleted */

    delete_lines( &lines );

    delete_polygons( &polygons );

    delete_pixels( &pixels );

    status = G_delete_window( window );

    return( status != OK );
}
