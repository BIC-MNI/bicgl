#include  <internal_volume_io.h>
#include  <graphics.h>

private  Gwindow  lookup_window_for_window_id(
    Window_id  window_id )
{
    int       i, n_windows;
    Gwindow   window;

    window = get_current_window();

    if( window != NULL &&
        GS_get_window_id( window->GS_window ) == window_id )
        return( window );

    n_windows = get_n_graphics_windows();

    for_less( i, 0, n_windows )
    {
        window = get_nth_graphics_window( i );

        if( GS_get_window_id( window->GS_window ) == window_id )
            break;
    }

    if( i >= n_windows )
        return( NULL );
    else
        return( window );
}

private  void  global_update_function( void )
{
    Window_id   window_id;
    Gwindow     window;

    window_id = GS_get_current_window_id();

    window = lookup_window_for_window_id( window_id );

    if( window == NULL )
        handle_internal_error( "global_update_function" );

    (*window->update_callback)( window, window->update_callback_data );
}

public  void  initialize_callbacks_for_window(
    Gwindow                 window )
{
    GS_set_update_function( global_update_function );
}

public  void  G_set_update_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data )
{
    window->update_callback = func;
    window->update_callback_data = func_data;
}

public  void  G_main_loop( void )
{
    GS_event_loop();
}
