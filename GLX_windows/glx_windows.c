#include  <internal_volume_io.h>
#include  <WS_windows.h>

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
    WS_window_struct       *window )
{
    int                      *attrib, n_attrib, flag;
    Status                   status;
    Colormap                 cmap;
    XVisualInfo              *visual;

    if( !GLX_supported() )
    {
        return( ERROR );
    }

    n_attrib = 0;

    if( n_overlay_planes > 0 )
    {
        print( "WS_create_window(): cannot handle overlay planes yet.\n" );
    }

    if( colour_map_mode )
    {
/*
        ADD_ELEMENT_TO_ARRAY( attrib, n_attrib, GLX_BUFFER_SIZE,
                              DEFAULT_CHUNK_SIZE);
        ADD_ELEMENT_TO_ARRAY( attrib, n_attrib, 4096, DEFAULT_CHUNK_SIZE);
*/
    }
    else
    {
        ADD_ELEMENT_TO_ARRAY( attrib, n_attrib, GLX_RGBA, DEFAULT_CHUNK_SIZE);
    }

    if( double_buffer_flag )
    {
        ADD_ELEMENT_TO_ARRAY( attrib, n_attrib, GLX_DOUBLEBUFFER,
                              DEFAULT_CHUNK_SIZE);
    }

    if( depth_buffer_flag )
    {
        ADD_ELEMENT_TO_ARRAY( attrib, n_attrib, GLX_DEPTH_SIZE,
                              DEFAULT_CHUNK_SIZE);
        ADD_ELEMENT_TO_ARRAY( attrib, n_attrib, 1, DEFAULT_CHUNK_SIZE);
    }

    ADD_ELEMENT_TO_ARRAY( attrib, n_attrib, None, DEFAULT_CHUNK_SIZE);

    visual = glXChooseVisual( X_get_display(), X_get_screen(), attrib );

    FREE( attrib );

    if( visual == NULL )
    {
        print( "Cannot find matching visual.\n" );
        return( ERROR );
    }

    glXGetConfig( X_get_display(), visual, GLX_RGBA, &flag );
    *actual_colour_map_mode = (flag == GL_FALSE);

    glXGetConfig( X_get_display(), visual, GLX_DOUBLEBUFFER, &flag );
    *actual_double_buffer_flag = (flag == GL_TRUE);

    glXGetConfig( X_get_display(), visual, GLX_DEPTH_SIZE, &flag );
    *actual_depth_buffer_flag = (flag > 0);

    *actual_n_overlay_planes = 0;

/*
    cmap = DefaultColormap( X_get_display(), X_get_screen() );
*/
    cmap = XCreateColormap( X_get_display(), RootWindow(X_get_display(),
                                                        X_get_screen()),
                            visual->visual, AllocNone );


    status = X_create_window_with_visual( title,
                              initial_x_pos, initial_y_pos,
                              initial_x_size, initial_y_size, 
                              colour_map_mode, visual, cmap,
                              &window->x_window );

    if( status == OK )
    {
        window->graphics_context = glXCreateContext( X_get_display(),
                                                     visual, NULL, TRUE );
    }

    return( status );
}

public  void  WS_delete_window(
    WS_window_struct  *window )
{
    glXDestroyContext( X_get_display(), window->graphics_context );
    X_delete_window( &window->x_window );
}

public  void  WS_make_window_current(
    WS_window_struct  *window )
{
    glXMakeCurrent( X_get_display(), window->x_window.window_id,
                    window->graphics_context );
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
