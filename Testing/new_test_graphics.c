
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

private  Window_id  create_GLUT_window(
    STRING                 title,
    int                    initial_x_pos,
    int                    initial_y_pos,
    int                    initial_x_size,
    int                    initial_y_size,
    BOOLEAN                colour_map_mode,
    BOOLEAN                double_buffer_flag,
    BOOLEAN                depth_buffer_flag,
    int                    n_overlay_planes,
    BOOLEAN                *actual_colour_map_mode_ptr,
    BOOLEAN                *actual_double_buffer_flag_ptr,
    BOOLEAN                *actual_depth_buffer_flag_ptr,
    int                    *actual_n_overlay_planes_ptr )
{
    int                rgba, doub, depth;
    unsigned  int      mode;
    int                used_size;
    Window_id          window_id;
    BOOLEAN            actual_colour_map_mode;
    BOOLEAN            actual_double_buffer_flag;
    BOOLEAN            actual_depth_buffer_flag;
    int                actual_n_overlay_planes;

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
    
    if( initial_x_pos >= 0 && initial_y_pos >= 0 )
    {
        if( initial_y_size <= 0 )
            used_size = glutGet( (GLenum) GLUT_INIT_WINDOW_HEIGHT );
        else
            used_size = initial_y_size;

        glutInitWindowPosition( initial_x_pos,
                                initial_y_pos + used_size - 1 );
    }
    else
        glutInitWindowPosition( -1, -1 );

    if( initial_x_size > 0 && initial_y_size > 0 )
        glutInitWindowSize( initial_x_size, initial_y_size );
    else
        glutInitWindowSize( DEFAULT_WINDOW_X_SIZE, DEFAULT_WINDOW_Y_SIZE );

    glutInitDisplayMode( mode );

    if( !glutGet( (GLenum) GLUT_DISPLAY_MODE_POSSIBLE ) &&
        double_buffer_flag )
    {
        print_error( "Double buffer mode unavailable, trying single buffer\n" );
        mode -= GLUT_DOUBLE;
        mode |= GLUT_SINGLE;
        glutInitDisplayMode( mode );
    }

    if( !glutGet( (GLenum) GLUT_DISPLAY_MODE_POSSIBLE ) )
    {
        print_error( "Could not open GLUT window in Display mode (%d,%d) for OpenGL\n",
                     colour_map_mode, double_buffer_flag );
        return( -1 );
    }

    window_id = glutCreateWindow( title );

    if( window_id < 1 )
    {
        print_error( "Could not open GLUT window for OpenGL\n" );
        return( -1 );
    }

    rgba = glutGet((GLenum) GLUT_WINDOW_RGBA);
    doub = glutGet((GLenum) GLUT_WINDOW_DOUBLEBUFFER);
    depth = glutGet((GLenum) GLUT_WINDOW_DEPTH_SIZE);

    glutUseLayer( (GLenum) GLUT_NORMAL );

    glutPopWindow();

    actual_n_overlay_planes = 0;
    glutDisplayFunc( display_function );

    actual_colour_map_mode = !rgba;
    actual_double_buffer_flag = doub;
    actual_depth_buffer_flag = (depth > 0);

    if( actual_colour_map_mode != colour_map_mode )
    {
        print_error( "Could not get requested colour_map_mode(%d), got(%d,%d)\n",
                     colour_map_mode,
                     !glutGet( (GLenum) GLUT_WINDOW_RGBA ),
                     glutGet( (GLenum) GLUT_WINDOW_COLORMAP_SIZE ) );

/*
        glutDestroyWindow( window_id );
        window_id = -1;
*/
    }

    if( actual_double_buffer_flag && !double_buffer_flag )
    {

        /*--- print_error( "For some reason got double buffer window, when requesting single buffer.\n" );*/

        /*--- testing has shown that it actually does get a single
              buffer mode, but reports double, so there may be a bug
              in GLUT, and we just assign the double buffering to FALSE */
        actual_double_buffer_flag = FALSE;
    }
    else if( !actual_double_buffer_flag && double_buffer_flag )
    {
        print_error( "Could not get requested double buffer window\n" );
    }

#ifdef  USING_X11
    if( window_id >= 1 && actual_colour_map_mode )
    {
        int   n_colours_to_copy;

        n_colours_to_copy = glutGet( (GLenum) GLUT_WINDOW_COLORMAP_SIZE );

        copy_X_colours( n_colours_to_copy );
    }
#endif

    if( actual_colour_map_mode_ptr != NULL )
        *actual_colour_map_mode_ptr = actual_colour_map_mode;

    if( actual_double_buffer_flag_ptr != NULL )
        *actual_double_buffer_flag_ptr = actual_double_buffer_flag;

    if( actual_depth_buffer_flag_ptr != NULL )
        *actual_depth_buffer_flag_ptr = actual_depth_buffer_flag;

    if( actual_n_overlay_planes_ptr != NULL )
        *actual_n_overlay_planes_ptr = actual_n_overlay_planes;

    return( window_id );
}


private  Status  WS_create(
    STRING                 title,
    int                    initial_x_pos,
    int                    initial_y_pos,
    int                    initial_x_size,
    int                    initial_y_size,
    BOOLEAN                colour_map_mode,
    BOOLEAN                double_buffer_flag,
    BOOLEAN                depth_buffer_flag,
    int                    n_overlay_planes,
    BOOLEAN                *actual_colour_map_mode,
    BOOLEAN                *actual_double_buffer_flag,
    BOOLEAN                *actual_depth_buffer_flag,
    int                    *actual_n_overlay_planes,
    WSwindow               window )
{
    window->window_id = create_GLUT_window( title, 
                                            initial_x_pos,
                                            initial_y_pos,
                                            initial_x_size,
                                            initial_y_size,
                                            colour_map_mode,
                                            double_buffer_flag,
                                            depth_buffer_flag,
                                            n_overlay_planes,
                                            actual_colour_map_mode,
                                            actual_double_buffer_flag,
                                            actual_depth_buffer_flag,
                                            actual_n_overlay_planes );

    return( OK );
}
private  Status  GS_create(
    GSwindow       window,
    STRING         title,
    int            x_pos,
    int            y_pos,
    int            width,
    int            height,
    BOOLEAN        colour_map_desired,
    BOOLEAN        double_buffer_desired,
    BOOLEAN        depth_buffer_desired,
    int            n_overlay_planes_desired,
    BOOLEAN        *actual_colour_map_flag,
    BOOLEAN        *actual_double_buffer_flag,
    BOOLEAN        *actual_depth_buffer_flag,
    int            *actual_n_overlay_planes )
{
    Status       status;

    status = WS_create( title, x_pos, y_pos, width, height,
                               colour_map_desired, double_buffer_desired,
                               depth_buffer_desired,
                               0, actual_colour_map_flag,
                               actual_double_buffer_flag,
                               actual_depth_buffer_flag,
                               actual_n_overlay_planes, &window->WS_window);

    return( status );
}

private  Status  G_create(
    STRING         title,
    int            x_pos,
    int            y_pos,
    int            width,
    int            height,
    BOOLEAN        colour_map_desired,
    BOOLEAN        double_buffer_desired,
    BOOLEAN        depth_buffer_desired,
    int            n_overlay_planes_desired,
    Gwindow        *window )
{
    Status   status;
    BOOLEAN  actual_colour_map_flag;
    BOOLEAN  actual_double_buffer_flag;
    BOOLEAN  actual_depth_buffer_flag;
    int      actual_n_overlay_planes;

    ALLOC( *window, 1 );

    ALLOC( (*window)->GS_window, 1 );

    status = GS_create( (*window)->GS_window,
                               title, x_pos, y_pos, width, height,
                               colour_map_desired,
                               double_buffer_desired,
                               depth_buffer_desired,
                               0,
                               &actual_colour_map_flag,
                               &actual_double_buffer_flag,
                               &actual_depth_buffer_flag,
                               &actual_n_overlay_planes );

    return( status );
}

int main(
    int    argc,
    char   *argv[] )
{
#ifndef OLD
    Gwindow  first, second;

    glutInit( &argc, argv );

    (void) G_create( "Test Window",
                              100, 600, 300, 300,
                              FALSE, TRUE, TRUE, 0,
                              &first );

    (void) G_create( "Second Window",
                            500, 600, 200, 200,
                            TRUE, TRUE, TRUE, 0, &second );
#else
    Gwindow  first, second;
    int      dummy;

    ALLOC( first, 1 );
    ALLOC( first->GS_window, 1 );
    ALLOC( second, 1 );
    ALLOC( second->GS_window, 1 );
    (void) GS_create_window( first->GS_window, "First Window",
                              100, 600, 300, 300,
                              FALSE, TRUE, TRUE, 0,
                              &dummy, &dummy, &dummy, &dummy );

    (void) GS_create_window( second->GS_window, "Second Window",
                              500, 600, 300, 300,
                              FALSE, TRUE, TRUE, 0,
                              &dummy, &dummy, &dummy, &dummy );
#endif

    glutMainLoop();

    return( 0 );
}
