#include  <internal_volume_io.h>
#include  <graphics.h>

static  BOOLEAN    left_button_state = FALSE;
static  BOOLEAN    middle_button_state = FALSE;
static  BOOLEAN    right_button_state = FALSE;
static  int        keyboard_modifiers = 0;

public  BOOLEAN  G_get_left_mouse_button( void )
{
    return( left_button_state );
}

public  BOOLEAN  G_get_middle_mouse_button( void )
{
    return( middle_button_state );
}

public  BOOLEAN  G_get_right_mouse_button( void )
{
    return( right_button_state );
}

public  BOOLEAN  G_get_shift_key_state( void )
{
    return( (keyboard_modifiers & SHIFT_KEY_BIT) != 0 );
}

public  BOOLEAN  G_get_ctrl_key_state( void )
{
    return( (keyboard_modifiers & CTRL_KEY_BIT) != 0 );
}

public  BOOLEAN  G_get_alt_key_state( void )
{
    return( (keyboard_modifiers & ALT_KEY_BIT) != 0 );
}

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

private  Gwindow  get_event_window(
    Window_id   window_id )
{
    Gwindow     window;

    window = lookup_window_for_window_id( window_id );

    if( window == NULL )
        handle_internal_error( "get_event_window" );

    return( window );
}

private  Gwindow  get_key_or_mouse_event_window(
    Window_id   window_id,
    int         x,
    int         y,
    int         modifier )
{
    Gwindow     window;

    window = get_event_window( window_id );

    if( window != NULL )
    {
        window->x_mouse_pos = x;
        window->y_mouse_pos = y;
        keyboard_modifiers = modifier;
    }

    return( window );
}

private  void  global_update_function(
    Window_id  window_id )
{
    Gwindow     window;

    window = get_event_window( window_id );

    if( window->update_callback != NULL )
        (*window->update_callback)( window, window->update_data );
}

private  void  global_update_overlay_function(
    Window_id  window_id )
{
    Gwindow     window;

    window = get_event_window( window_id );

    if( window->update_overlay_callback != NULL )
        (*window->update_overlay_callback)( window,
                                            window->update_overlay_data );
}

private  void  global_resize_function(
    Window_id  window_id,
    int        x,
    int        y,
    int        x_size,
    int        y_size )
{
    Gwindow     window;

    window = get_event_window( window_id );

    window->x_origin = x;
    window->y_origin = y;
    window->x_size = x_size;
    window->y_size = y_size;

    window_was_resized( window );

    if( window->resize_callback != NULL )
        (*window->resize_callback)( window, x, y, x_size, y_size,
                                    window->resize_data );
}

private  void  global_key_down_function(
    Window_id  window_id,
    int        key,
    int        x,
    int        y,
    int        modifier )
{
    Gwindow     window;

    window = get_key_or_mouse_event_window( window_id, x, y, modifier );

    if( window->key_down_callback != NULL )
        (*window->key_down_callback)( window, key, window->key_down_data );
}

private  void  global_key_up_function(
    Window_id  window_id,
    int        key,
    int        x,
    int        y,
    int        modifier )
{
    Gwindow     window;

    window = get_key_or_mouse_event_window( window_id, x, y, modifier );

    if( window->key_up_callback != NULL )
        (*window->key_up_callback)( window, key, window->key_up_data );
}

private  void  global_mouse_movement_function(
    Window_id  window_id,
    int        x,
    int        y )
{
    Gwindow     window;

    window = get_event_window( window_id );

    window->x_mouse_pos = x;
    window->y_mouse_pos = y;

    if( window->mouse_movement_callback != NULL )
        (*window->mouse_movement_callback)( window, x, y,
                                            window->mouse_movement_data );
}

private  void  global_left_mouse_down_function(
    Window_id  window_id,
    int        x,
    int        y,
    int        modifier )
{
    Gwindow     window;

    window = get_key_or_mouse_event_window( window_id, x, y, modifier );

    left_button_state = TRUE;

    if( window->left_mouse_down_callback != NULL )
        (*window->left_mouse_down_callback)( window, x, y,
                                             window->left_mouse_down_data );
}

private  void  global_left_mouse_up_function(
    Window_id  window_id,
    int        x,
    int        y,
    int        modifier )
{
    Gwindow     window;

    window = get_key_or_mouse_event_window( window_id, x, y, modifier );

    left_button_state = FALSE;

    if( window->left_mouse_up_callback != NULL )
        (*window->left_mouse_up_callback)( window, x, y,
                                           window->left_mouse_up_data );
}

private  void  global_middle_mouse_down_function(
    Window_id  window_id,
    int        x,
    int        y,
    int        modifier )
{
    Gwindow     window;

    window = get_key_or_mouse_event_window( window_id, x, y, modifier );

    middle_button_state = TRUE;

    if( window->middle_mouse_down_callback != NULL )
        (*window->middle_mouse_down_callback)( window, x, y,
                                               window->middle_mouse_down_data );
}

private  void  global_middle_mouse_up_function(
    Window_id  window_id,
    int        x,
    int        y,
    int        modifier )
{
    Gwindow     window;

    window = get_key_or_mouse_event_window( window_id, x, y, modifier );

    middle_button_state = FALSE;

    if( window->middle_mouse_up_callback != NULL )
        (*window->middle_mouse_up_callback)( window, x, y,
                                             window->middle_mouse_up_data );
}

private  void  global_right_mouse_down_function(
    Window_id  window_id,
    int        x,
    int        y,
    int        modifier )
{
    Gwindow     window;

    window = get_key_or_mouse_event_window( window_id, x, y, modifier );

    right_button_state = TRUE;

    if( window->right_mouse_down_callback != NULL )
        (*window->right_mouse_down_callback)( window, x, y,
                                              window->right_mouse_down_data );
}

private  void  global_right_mouse_up_function(
    Window_id  window_id,
    int        x,
    int        y,
    int        modifier )
{
    Gwindow     window;

    window = get_key_or_mouse_event_window( window_id, x, y, modifier );

    right_button_state = FALSE;

    if( window->right_mouse_up_callback != NULL )
        (*window->right_mouse_up_callback)( window, x, y,
                                            window->right_mouse_up_data );
}

private  void  global_iconify_function(
    Window_id  window_id )
{
    Gwindow     window;

    window = get_event_window( window_id );

    if( window->iconify_callback != NULL )
        (*window->iconify_callback)( window, window->iconify_data );
}

private  void  global_deiconify_function(
    Window_id  window_id )
{
    Gwindow     window;

    window = get_event_window( window_id );

    if( window->deiconify_callback != NULL )
        (*window->deiconify_callback)( window, window->deiconify_data );
}

private  void  global_enter_function(
    Window_id  window_id )
{
    Gwindow     window;

    window = get_event_window( window_id );

    if( window->enter_callback != NULL )
        (*window->enter_callback)( window, window->enter_data );
}

private  void  global_leave_function(
    Window_id  window_id )
{
    Gwindow     window;

    window = get_event_window( window_id );

    if( window->leave_callback != NULL )
        (*window->leave_callback)( window, window->leave_data );
}

private  void  global_quit_function(
    Window_id  window_id )
{
    Gwindow     window;

    window = get_event_window( window_id );

    if( window->quit_callback != NULL )
        (*window->quit_callback)( window, window->quit_data );
}

private  void  initialize_callbacks( void )
{
    GS_set_update_function( global_update_function );
    GS_set_update_overlay_function( global_update_overlay_function );
    GS_set_resize_function( global_resize_function );
    GS_set_key_down_function( global_key_down_function );
    GS_set_key_up_function( global_key_up_function );
    GS_set_mouse_movement_function( global_mouse_movement_function );
    GS_set_left_mouse_down_function( global_left_mouse_down_function );
    GS_set_left_mouse_up_function( global_left_mouse_up_function );
    GS_set_middle_mouse_down_function( global_middle_mouse_down_function );
    GS_set_middle_mouse_up_function( global_middle_mouse_up_function );
    GS_set_right_mouse_down_function( global_right_mouse_down_function );
    GS_set_right_mouse_up_function( global_right_mouse_up_function );
    GS_set_iconify_function( global_iconify_function );
    GS_set_deiconify_function( global_deiconify_function );
    GS_set_enter_function( global_enter_function );
    GS_set_leave_function( global_leave_function );
    GS_set_quit_function( global_quit_function );
}

public  void  G_set_update_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data )
{
    window->update_callback = func;
    window->update_data = func_data;
}

public  void  G_set_update_overlay_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data )
{
    window->update_overlay_callback = func;
    window->update_overlay_data = func_data;
}

public  void  G_set_resize_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, int, int, void * ),
    void                    *func_data )
{
    window->resize_callback = func;
    window->resize_data = func_data;
}

public  void  G_set_key_down_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, void * ),
    void                    *func_data )
{
    window->key_down_callback = func;
    window->key_down_data = func_data;
}

public  void  G_set_key_up_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, void * ),
    void                    *func_data )
{
    window->key_up_callback = func;
    window->key_up_data = func_data;
}

public  void  G_set_mouse_movement_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data )
{
    window->mouse_movement_callback = func;
    window->mouse_movement_data = func_data;
}

public  void  G_set_left_mouse_down_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data )
{
    window->left_mouse_down_callback = func;
    window->left_mouse_down_data = func_data;
}

public  void  G_set_left_mouse_up_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data )
{
    window->left_mouse_up_callback = func;
    window->left_mouse_up_data = func_data;
}

public  void  G_set_middle_mouse_down_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data )
{
    window->middle_mouse_down_callback = func;
    window->middle_mouse_down_data = func_data;
}

public  void  G_set_middle_mouse_up_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data )
{
    window->middle_mouse_up_callback = func;
    window->middle_mouse_up_data = func_data;
}

public  void  G_set_right_mouse_down_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data )
{
    window->right_mouse_down_callback = func;
    window->right_mouse_down_data = func_data;
}

public  void  G_set_right_mouse_up_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data )
{
    window->right_mouse_up_callback = func;
    window->right_mouse_up_data = func_data;
}

public  void  G_set_iconify_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data )
{
    window->iconify_callback = func;
    window->iconify_data = func_data;
}

public  void  G_set_deiconify_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data )
{
    window->deiconify_callback = func;
    window->deiconify_data = func_data;
}

public  void  G_set_window_enter_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data )
{
    window->enter_callback = func;
    window->enter_data = func_data;
}

public  void  G_set_window_leave_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data )
{
    window->leave_callback = func;
    window->leave_data = func_data;
}

public  void  G_set_window_quit_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data )
{
    window->quit_callback = func;
    window->quit_data = func_data;
}

public  void  initialize_callbacks_for_window(
    Gwindow                 window )
{
    window->update_callback = NULL;
    window->update_overlay_callback = NULL;
    window->resize_callback = NULL;
    window->key_down_callback = NULL;
    window->key_up_callback = NULL;
    window->mouse_movement_callback = NULL;
    window->left_mouse_down_callback = NULL;
    window->left_mouse_up_callback = NULL;
    window->middle_mouse_down_callback = NULL;
    window->middle_mouse_up_callback = NULL;
    window->right_mouse_down_callback = NULL;
    window->right_mouse_up_callback = NULL;
    window->iconify_callback = NULL;
    window->deiconify_callback = NULL;
    window->enter_callback = NULL;
    window->leave_callback = NULL;
    window->quit_callback = NULL;
}

public  void  G_set_update_flag(
    Gwindow  window )
{
    GS_set_update_flag( window->GS_window );
}

public  void  G_add_timer_function(
    Real          seconds,
    void          (*func) ( void * ),
    void          *data )
{
    GS_add_timer_function( seconds, func, data );
}

public  void  G_add_idle_function(
    void  (*func) ( void * ),
    void          *data )
{
    GS_add_idle_function( func, data );
}

public  void  G_remove_idle_function(
    void  (*func) ( void * ),
    void          *data )
{
    GS_remove_idle_function( func, data );
}

public  void  G_main_loop( void )
{
    initialize_callbacks();

    GS_event_loop();
}
