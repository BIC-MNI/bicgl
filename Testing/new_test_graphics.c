
#include  <internal_volume_io.h>
#include  <graphics.h>
#include  <GS_graphics.h>
#include  <WS_graphics.h>

#define  DEFAULT_WINDOW_X_SIZE    300
#define  DEFAULT_WINDOW_Y_SIZE    300

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
    BOOLEAN                colour_map_mode )
{
    int                rgba;
    unsigned  int      mode;
    int                used_size;
    Window_id          window_id;
    BOOLEAN            actual_colour_map_mode;

    mode = 0;

    if( colour_map_mode )
        mode |= GLUT_INDEX;
    else
        mode |= GLUT_RGB;

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
        print_error( "Could not open GLUT window in Display mode (%d,%d) for OpenGL\n",
                     colour_map_mode, TRUE );
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

    actual_colour_map_mode = !rgba;

    if( actual_colour_map_mode != colour_map_mode )
    {
        print_error( "Could not get requested colour_map_mode(%d), got(%d,%d)\n",
                     colour_map_mode,
                     !glutGet( (GLenum) GLUT_WINDOW_RGBA ),
                     glutGet( (GLenum) GLUT_WINDOW_COLORMAP_SIZE ) );

    }

    return;
}

private  void  G_create(
    STRING         title,
    int            x_pos,
    int            y_pos,
    int            width,
    int            height,
    BOOLEAN        colour_map_desired )
{
    create_GLUT_window( title, x_pos, y_pos, width, height,
                        colour_map_desired );
}

int main(
    int    argc,
    char   *argv[] )
{
#ifndef OLD
    Gwindow  first, second;

    glutInit( &argc, argv );

    G_create( "Test Window", 100, 600, 300, 300, FALSE );

    G_create( "Second Window", 500, 600, 200, 200, TRUE );
#else
    Gwindow  first, second;

    ALLOC( first, 1 );
    ALLOC( first->GS_window, 1 );
    first->GS_window->WS_window.window_id = create_GLUT_window( "Test Window", 100, 600, 300, 300, FALSE );

    ALLOC( second, 1 );
    ALLOC( second->GS_window, 1 );
    second->GS_window->WS_window.window_id = create_GLUT_window( "Second Window", 500, 600, 300, 300, FALSE );
#endif

    glutMainLoop();

    return( 0 );
}
