
#include  <internal_volume_io.h>
#include  <graphics.h>
#include  <GS_graphics.h>
#include  <WS_graphics.h>

private  void  display_function( void )
{
    print( "Display callback\n" );
}

private  void  create_GLUT_window(
    STRING                 title,
    int                    initial_x_pos,
    int                    initial_y_pos,
    int                    initial_x_size,
    int                    initial_y_size,
    BOOLEAN                rgb_mode )
{
    int                rgba;
    unsigned  int      mode;
    int                used_size;
    int                window_id;

    mode = 0;

    if( rgb_mode )
        mode |= GLUT_RGB;
    else
        mode |= GLUT_INDEX;

    mode |= GLUT_DOUBLE;
    mode |= GLUT_DEPTH;
    
    glutInitWindowPosition( initial_x_pos, initial_y_pos );

    glutInitWindowSize( initial_x_size, initial_y_size );

    glutInitDisplayMode( mode );

    if( !glutGet( (GLenum) GLUT_DISPLAY_MODE_POSSIBLE ) )
    {
        print_error( "Double buffer mode unavailable, trying single buffer\n" );
        mode -= GLUT_DOUBLE;
        mode |= GLUT_SINGLE;
        glutInitDisplayMode( mode );
    }

    if( !glutGet( (GLenum) GLUT_DISPLAY_MODE_POSSIBLE ) )
    {
        print_error( "Could not open GLUT window in rgb mode=%d for OpenGL\n",
                     rgb_mode );
        return;
    }

    window_id = glutCreateWindow( title );

    if( window_id < 1 )
    {
        (void) printf( "Could not open GLUT window for OpenGL\n" );
        return;
    }

    rgba = glutGet((GLenum) GLUT_WINDOW_RGBA);

    glutUseLayer( (GLenum) GLUT_NORMAL );

    glutPopWindow();

    glutDisplayFunc( display_function );

    if( rgba && !rgb_mode || !rgba && rgb_mode )
    {
        print_error( "Could not get requested rgb_mode(%d), got(%d,%d)\n",
                     rgb_mode,
                     glutGet( (GLenum) GLUT_WINDOW_RGBA ),
                     glutGet( (GLenum) GLUT_WINDOW_COLORMAP_SIZE ) );

    }

    return;
}

int main(
    int    argc,
    char   *argv[] )
{
    glutInit( &argc, argv );

    create_GLUT_window( "Test Window", 100, 600, 300, 300, FALSE );
    create_GLUT_window( "Second Window", 500, 600, 200, 200, TRUE );

    glutMainLoop();

    return( 0 );
}
