#include  <def_mni.h>

int  main(
    int   argc,
    char  *argv[] )
{
    Status         status;
    volume_struct  volume;
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

    return( status != OK );
}
