#include  <internal_volume_io.h>
#include  <WS_windows.h>
#include  <GL/glx.h>

private  BOOLEAN  GLX_supported()
{
    static  BOOLEAN  first = TRUE;
    static  BOOLEAN  supported = FALSE;
    int              error, event;

    if( first )
    {
        first = FALSE;
        supported = glXQueryExtension( X_get_display(), &error, &event );
    }

    return( supported );
}

public  Status  WS_create_window(
    char                   title[],
    char                   initial_x_pos,
    char                   initial_y_pos,
    char                   initial_x_size,
    char                   initial_y_size,
    BOOLEAN                colour_map_mode,
    BOOLEAN                double_buffer_flag,
    BOOLEAN                depth_buffer_flag,
    int                    n_overlay_planes,
    BOOLEAN                *actual_colour_map_mode,
    BOOLEAN                *actual_double_buffer_flag,
    BOOLEAN                *actual_depth_buffer_flag,
    int                    *actual_n_overlay_planes,
    WS_window_struct       *window )
{
    int                      i, n_config, nret;
    Status                   status;
    long                     glxlink_ret;
    GLXconfig                *glxConfig, *retconfig, element;
    Colormap                 cmap;
    XVisualInfo              *visual, templ;

    if( !GLX_supported() )
    {
        return( ERROR );
    }

    if( n_overlay_planes > 0 )
    {
        print( "WS_create_window():  Sorry, cannot handle overlay planes.\n" );
    }
    *actual_n_overlay_planes = 0;

    n_config = 0;

    if( !colour_map_mode )
    {
        element.buffer = GLX_NORMAL;
        element.mode = GLX_RGB;
        element.arg = TRUE;
        ADD_ELEMENT_TO_ARRAY( glxConfig, n_config, element, DEFAULT_CHUNK_SIZE);
    }

    if( double_buffer_flag )
    {
        element.buffer = GLX_NORMAL;
        element.mode = GLX_DOUBLE;
        element.arg = TRUE;
        ADD_ELEMENT_TO_ARRAY( glxConfig, n_config, element, DEFAULT_CHUNK_SIZE);
    }

    if( depth_buffer_flag )
    {
        element.buffer = GLX_NORMAL;
        element.mode = GLX_ZSIZE;
        element.arg = GLX_NOCONFIG;
        ADD_ELEMENT_TO_ARRAY( glxConfig, n_config, element, DEFAULT_CHUNK_SIZE);
    }

    element.buffer = 0;
    element.mode = 0;
    element.arg = 0;
    ADD_ELEMENT_TO_ARRAY( glxConfig, n_config, element, DEFAULT_CHUNK_SIZE );

/*
    retconfig = GLXgetconfig( X_get_display(), X_get_screen(),
                              glxConfig );
*/

    if( n_config > 0 )
        FREE( glxConfig );

    if( retconfig == NULL )
    {
        print( "Cannot get glxconfig of desired configuration\n" );
        status = ERROR;
    }

    if( status == OK )
    {
        *actual_colour_map_mode = colour_map_mode;
        *actual_double_buffer_flag = double_buffer_flag;
        *actual_depth_buffer_flag = depth_buffer_flag;

        cmap = DefaultColormap( X_get_display(), X_get_screen() );

        i = 0;
        while( retconfig[i].buffer != 0 )
        {
            switch( retconfig[i].mode )
            {
            case GLX_DOUBLE:
                if( retconfig[i].buffer == GLX_NORMAL && retconfig[i].arg == 0 )
                    double_buffer_flag = FALSE;
                break;

            case GLX_RGB:
                if( retconfig[i].buffer == GLX_NORMAL && retconfig[i].arg == 0 )
                    colour_map_mode = TRUE;
                break;

            case GLX_ZSIZE:
                if( retconfig[i].buffer == GLX_NORMAL && retconfig[i].arg == 0 )
                    depth_buffer_flag = FALSE;
                break;

            case GLX_COLORMAP:
                if( retconfig[i].buffer == GLX_NORMAL )
                    cmap = retconfig[i].arg;
                break;

            case GLX_VISUAL:
                if( retconfig[i].buffer == GLX_NORMAL )
                {
                    templ.visualid = retconfig[i].arg;
                    templ.screen = X_get_screen();
                    visual = XGetVisualInfo( X_get_display(),
                                              VisualScreenMask|VisualIDMask,
                                              &templ, &nret );
                }
                break;

            }

            ++i;
        }

        status = X_create_window_with_visual( title,
                                  initial_x_pos, initial_y_pos,
                                  initial_x_size, initial_y_size, 
                                  colour_map_mode, visual, cmap,
                                  &window->x_window );
    }

    if( status == OK )
    {
        i = 0;
        while( retconfig[i].buffer != 0 )
        {
            if( retconfig[i].buffer == GLX_NORMAL &&
                retconfig[i].mode == GLX_WINDOW )
            {
                retconfig[i].arg = (int) window->x_window.window_id;
            }
            ++i;
        }

/*
        glxlink_ret = GLXlink( X_get_display(), retconfig );
*/

        if( glxlink_ret < 0 )
        {
            print( "GLXlink returned %d\n", glxlink_ret );
            status = ERROR;
        }
    }

    if( retconfig != NULL )
        free( retconfig );

    return( status );
}

public  void  WS_delete_window(
    WS_window_struct  *window )
{
/*
    GLXunlink( X_get_display(), window->x_window.window_id );
*/
    X_delete_window( &window->x_window );
}

public  void  WS_make_window_current(
    WS_window_struct  *window )
{
/*
    GLXwinset( X_get_display(), window->x_window.window_id );
*/
}

public  BOOLEAN  WS_get_event(
    Event_types          *event_type,
    Window_id            *window,
    event_info_struct    *info )
{
    return( X_get_event( event_type, window, info ) );
}

public  void  WS_get_window_position(
    WS_window_struct  *window,
    int               *x_pos,
    int               *y_pos )
{
    int    x_size, y_size;

    X_get_window_geometry( &window->x_window, x_pos, y_pos, &x_size, &y_size );
}

public  void  WS_get_window_size(
    WS_window_struct  *window,
    int               *x_size,
    int               *y_size )
{
    int    x_pos, y_pos;

    X_get_window_geometry( &window->x_window, &x_pos, &y_pos, x_size, y_size );
}

public  void  WS_set_colour_map_entry(
    WS_window_struct  *window,
    int               index,
    Colour            colour )
{
    X_set_colour_map_entry( &window->x_window, index,
                            get_Colour_r(colour),
                            get_Colour_g(colour),
                            get_Colour_b(colour) );
}

public  void  WS_swap_buffers(
    WS_window_struct  *window )
{
    glXSwapBuffers( X_get_display(), window->x_window.window_id );
}
