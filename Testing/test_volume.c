#include  <def_mni.h>

int  main(
    int   argc,
    char  *argv[] )
{
    Status         status;
    int            n_alloced, x_size, y_size, i;
    Real           intensity;
    pixels_struct  pixels;
    volume_struct  volume;
    window_struct  *window;
    Real           scale;
    Colour         *rgb_map;
    char           *filename;

    initialize_argument_processing( argc, argv );
    (void) get_string_argument( "/nil/david/big_data/avg.mnc", &filename );

    status = input_volume( filename, &volume );

    if( status == OK )
    {
        print( "Volume %s: %d by %d by %d\n",
                filename, volume.sizes[X], volume.sizes[Y], volume.sizes[Z] );
        print( "Thickness: %g %g %g\n",
                volume.thickness[X], volume.thickness[Y], volume.thickness[Z] );
    }

    status = G_create_window( "Volume Browser", -1, -1, -1, -1, &window );

    G_get_window_size( window, &x_size, &y_size );

    n_alloced = 0;

    ALLOC( rgb_map, volume.max_value+1 );

    for_less( i, 0, volume.max_value+1 )
    {
        intensity = (Real) i / (Real) volume.max_value;
        rgb_map[i] = make_Colour_0_1( intensity, intensity, intensity );
    }

    scale = (Real) x_size / (Real) volume.sizes[X];
    if( (Real) y_size / (Real) volume.sizes[Y] < scale )
        scale = (Real) y_size / (Real) volume.sizes[Y];

    create_volume_slice( &volume, (Real) volume.sizes[Z] / 2.0,
                         0.0, 0.0, scale, scale,
                         (volume_struct *) NULL, 0.0, 0.0, 0.0, 0.0, 0.0,
                         X, Y, x_size, y_size, RGB_PIXEL, FALSE,
                         (unsigned short **) NULL,
                         &rgb_map, &n_alloced, &pixels );

    G_set_view_type( window, PIXEL_VIEW );
    G_draw_pixels( window, &pixels );
    G_update_window( window );

    print( "Hit return: " );
    (void) getchar();

    return( status != OK );
}
