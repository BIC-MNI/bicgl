#include  <internal_volume_io.h>
#include  "../../GLUT/include/glut.h"

private int  test_glut_window(
    BOOLEAN   colour_map_mode,
    BOOLEAN   double_buffer_flag,
    BOOLEAN   depth_buffer_flag )
{
    int                rgba, doub, depth;
    unsigned  int      mode;
    int                window_id;
    char               title[1000];

    mode = 0;

    if( colour_map_mode )
        mode |= GLUT_INDEX;
    else
        mode |= GLUT_RGB;

    if( double_buffer_flag )
        mode |= GLUT_DOUBLE;
    else
        mode |= GLUT_SINGLE;

    if( depth_buffer_flag )
        mode |= GLUT_DEPTH;
    
    glutInitDisplayMode( mode );

    if( !glutGet( (GLenum) GLUT_DISPLAY_MODE_POSSIBLE ) )
    {
        (void) printf( "Test window: Could not open GLUT window in Display mode (%d,%d,%d) for OpenGL\n",
                     colour_map_mode, double_buffer_flag, depth_buffer_flag );
        return( -1 );
    }

    (void) sprintf( title, "Title: %d %d %d",
                    colour_map_mode, double_buffer_flag, depth_buffer_flag );

    window_id = glutCreateWindow( title );

    if( window_id < 1 )
    {
        (void) printf( "Test window: Could not open GLUT window for OpenGL\n" );
        return( -1 );
    }

    rgba = glutGet((GLenum) GLUT_WINDOW_RGBA);
    doub = glutGet((GLenum) GLUT_WINDOW_DOUBLEBUFFER);
    depth = glutGet((GLenum) GLUT_WINDOW_DEPTH_SIZE);

    if( rgba != !colour_map_mode ||
        doub != double_buffer_flag ||
        (depth > 0) != depth_buffer_flag )
    {
        (void) printf( "Test window: Could not get requested mode( %d %d %d ), got( %d %d %d )\n",
                     colour_map_mode, double_buffer_flag, depth_buffer_flag,
                     !rgba, doub, (depth > 0 ) );
    }

    return( window_id );
}

private  void  test_glut_windows( void )
{
    int  window_id;

    window_id = test_glut_window( FALSE, FALSE, FALSE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( FALSE, FALSE, TRUE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( FALSE, TRUE, FALSE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( FALSE, TRUE, TRUE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( TRUE, FALSE, FALSE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( TRUE, FALSE, TRUE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( TRUE, TRUE, FALSE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( TRUE, TRUE, TRUE );
    glutDestroyWindow( window_id );
}

int main(
    int    argc,
    char   *argv[] )
{
    Status            status;

    glutInit( &argc, argv );

    print( "\n" );
    test_glut_windows();

    print( "\n" );
    test_glut_windows();

    print( "\n" );
    test_glut_windows();

    print( "\n" );
    test_glut_windows();

    print( "\n" );
    test_glut_windows();

    return( 0 );
}
