 
#include  <internal_volume_io.h>
#include  <gs_specific.h>
#include  <queue.h>

private  BOOLEAN  left_mouse_down = FALSE;
private  Gwindow  left_mouse_window = NULL;
private  BOOLEAN  middle_mouse_down = FALSE;
private  Gwindow  middle_mouse_window = NULL;
private  BOOLEAN  right_mouse_down = FALSE;
private  Gwindow  right_mouse_window = NULL;

private  struct
{
    BOOLEAN  key_is_down;
    Gwindow  window;
}
key_states[256];


private  int  current_mouse_x = -1;
private  int  current_mouse_y = -1;


private  void           set_current_input_window( Gwindow );
private  Gwindow        get_current_input_window( void );

#define  MIN_TIME_BETWEEN_EVENTS  0.01

typedef  struct
{
    Event_types    type;
    Gwindow        window;
    int            x_mouse;
    int            y_mouse;
    int            key_pressed;
} event_struct;

private  QUEUE_STRUCT( event_struct )    event_queue;

private  BOOLEAN  initialized = FALSE;

private  void  check_initialize_event_queue( void )
{
    int   i;

    if( !initialized )
    {
        initialized = TRUE;
        INITIALIZE_QUEUE( event_queue );

        for_less( i, 0, 256 )
        {
            key_states[i].key_is_down = FALSE;
            key_states[i].window = NULL;
        }
    }
}

public  void  delete_event_queue()
{
    if( initialized )
    {
        DELETE_QUEUE( event_queue );
    }
}

private  void  check_event_queue( void )
{
    event_struct   event, pre_event;

    check_initialize_event_queue();

    while( GS_get_event( &event.type, &event.window,
                         &event.x_mouse, &event.y_mouse, &event.key_pressed ) )
    {
        switch( event.type )
        {
        case KEY_UP_EVENT:
            if( event.key_pressed >= 0 && event.key_pressed < 256 )
            {
                if( key_states[event.key_pressed].key_is_down )
                {
                    key_states[event.key_pressed].key_is_down = FALSE;
                }
                else
                {
                    pre_event.type = KEY_DOWN_EVENT;
                    pre_event.window = event.window;
                    pre_event.key_pressed = event.key_pressed;
                    INSERT_IN_QUEUE( event_queue, pre_event );
                }
            }
            break;

        case KEY_DOWN_EVENT:
            if( event.key_pressed >= 0 && event.key_pressed < 256 )
            {
                if( !key_states[event.key_pressed].key_is_down )
                {
                    key_states[event.key_pressed].key_is_down = TRUE;
                    key_states[event.key_pressed].window = event.window;
                }
                else
                {
                    pre_event.type = KEY_UP_EVENT;
                    pre_event.window = key_states[event.key_pressed].window;
                    pre_event.key_pressed = event.key_pressed;
                    INSERT_IN_QUEUE( event_queue, pre_event );
                }
            }
            break;

        case LEFT_MOUSE_UP_EVENT:
            if( left_mouse_down )
            {
                left_mouse_down = FALSE;
            }
            else
            {
                pre_event = event;
                pre_event.type = LEFT_MOUSE_DOWN_EVENT;
                INSERT_IN_QUEUE( event_queue, pre_event );
            }
            break;

        case LEFT_MOUSE_DOWN_EVENT:
            if( !left_mouse_down )
            {
                left_mouse_down = TRUE;
                left_mouse_window = event.window;
            }
            else
            {
                pre_event = event;
                pre_event.type = LEFT_MOUSE_UP_EVENT;
                pre_event.window = left_mouse_window;
                INSERT_IN_QUEUE( event_queue, pre_event );
            }
            break;

        case MIDDLE_MOUSE_UP_EVENT:
            if( middle_mouse_down )
            {
                middle_mouse_down = FALSE;
            }
            else
            {
                pre_event = event;
                pre_event.type = MIDDLE_MOUSE_DOWN_EVENT;
                INSERT_IN_QUEUE( event_queue, pre_event );
            }
            break;

        case MIDDLE_MOUSE_DOWN_EVENT:
            if( !middle_mouse_down )
            {
                middle_mouse_down = TRUE;
                middle_mouse_window = event.window;
            }
            else
            {
                pre_event = event;
                pre_event.type = MIDDLE_MOUSE_UP_EVENT;
                pre_event.window = middle_mouse_window;
                INSERT_IN_QUEUE( event_queue, pre_event );
            }
            break;

        case RIGHT_MOUSE_UP_EVENT:
            if( right_mouse_down )
            {
                right_mouse_down = FALSE;
            }
            else
            {
                pre_event = event;
                pre_event.type = RIGHT_MOUSE_DOWN_EVENT;
                INSERT_IN_QUEUE( event_queue, pre_event );
            }
            break;

        case RIGHT_MOUSE_DOWN_EVENT:
            if( !right_mouse_down )
            {
                right_mouse_down = TRUE;
                right_mouse_window = event.window;
            }
            else
            {
                pre_event = event;
                pre_event.type = RIGHT_MOUSE_UP_EVENT;
                pre_event.window = right_mouse_window;
                INSERT_IN_QUEUE( event_queue, pre_event );
            }
            break;
        }

        INSERT_IN_QUEUE( event_queue, event );
    }
}

private  Real   next_timer_event = 0.0;
private  Real   timer_interval = 0.2;

public  void  G_set_timer_event_interval(
    Real interval )
{
    timer_interval = interval;
}

private  Event_types  prev_type = (Event_types) -1;
private  Real         next_check_event_time = 0.0;
private  BOOLEAN      prev_was_timer = FALSE;

private  Event_types  get_event(
    Gwindow        *window,
    int            *x_mouse,
    int            *y_mouse,
    int            *key_pressed )
{
    BOOLEAN        found;
    Real           current_time;
    Event_types    type;
    event_struct   event;

    type = NO_EVENT;

    if( prev_type == NO_EVENT )
    {
        current_time = current_realtime_seconds();
        if( current_time < next_check_event_time )
            sleep_program( next_check_event_time - current_time );
    }

    check_event_queue();

    if( !IS_QUEUE_EMPTY( event_queue ) )
    {
        found = FALSE;

        if( !prev_was_timer && prev_type != NO_EVENT && timer_interval > 0.0 )
        {
            current_time = current_realtime_seconds();
    
            if( current_time > next_timer_event )
            {
                type = NO_EVENT;
                found = TRUE;
                prev_was_timer = TRUE;
            }
        }

        if( !found )
        {
            REMOVE_FROM_QUEUE( event_queue, event );
            type = event.type;
            *window = event.window;
            *x_mouse = event.x_mouse;
            *y_mouse = event.y_mouse;
            *key_pressed = event.key_pressed;

            if( (prev_type == NO_EVENT || prev_was_timer)
                && timer_interval > 0.0 )
            {
                next_timer_event = current_realtime_seconds() + timer_interval;
            }

            prev_type = type;
            prev_was_timer = FALSE;
        }
    }
    else
    {
        next_check_event_time = current_realtime_seconds() +
                                MIN_TIME_BETWEEN_EVENTS;
        prev_type = NO_EVENT;
    }

    if( type == WINDOW_RESIZE_EVENT && *window != NULL )
        window_was_resized( *window );

    return( type );
}

private  BOOLEAN  mouse_movement_events_enabled = FALSE;

public  void  set_mouse_movement_events( BOOLEAN  state )
{
    mouse_movement_events_enabled = state;
}

private  void  record_current_mouse_pos(
    int       x_mouse,
    int       y_mouse )
{
    if( get_current_input_window() != NULL &&
        WS_mouse_events_in_screen_coordinates() )
    {
        x_mouse -= get_current_input_window()->x_origin;
        y_mouse -= get_current_input_window()->y_origin;
    }

    current_mouse_x = x_mouse;
    current_mouse_y = y_mouse;
}

public  BOOLEAN  G_events_pending()
{
    check_event_queue();

    return( !IS_QUEUE_EMPTY( event_queue ) );
}

public  Event_types  G_get_event(
    Gwindow        *window,
    int            *key_pressed )
{
    BOOLEAN        found;
    int            x_mouse, y_mouse;
    Event_types    type;

    found = FALSE;

    while( !found )
    {
        type = get_event( window, &x_mouse, &y_mouse, key_pressed );

        switch( type )
        {
        case MOUSE_MOVEMENT_EVENT:
            if( mouse_movement_events_enabled )
                found = TRUE;
            *window = get_current_input_window();
            record_current_mouse_pos( x_mouse, y_mouse );
            break;

        case WINDOW_ENTER_EVENT:
            set_current_input_window( *window );
            found = TRUE;
            break;

        case WINDOW_LEAVE_EVENT:
            *window = get_current_input_window();
            set_current_input_window( (Gwindow) NULL );
            found = TRUE;
            break;

        case KEY_DOWN_EVENT:
        case KEY_UP_EVENT:
            *window = get_current_input_window();
            found = TRUE;
            break;

        case LEFT_MOUSE_DOWN_EVENT:
            *window = get_current_input_window();
            record_current_mouse_pos( x_mouse, y_mouse );
            found = TRUE;
            break;

        case LEFT_MOUSE_UP_EVENT:
            *window = get_current_input_window();
            record_current_mouse_pos( x_mouse, y_mouse );
            found = TRUE;
            break;

        case MIDDLE_MOUSE_DOWN_EVENT:
            *window = get_current_input_window();
            record_current_mouse_pos( x_mouse, y_mouse );
            found = TRUE;
            break;

        case MIDDLE_MOUSE_UP_EVENT:
            *window = get_current_input_window();
            record_current_mouse_pos( x_mouse, y_mouse );
            found = TRUE;
            break;

        case RIGHT_MOUSE_DOWN_EVENT:
            *window = get_current_input_window();
            record_current_mouse_pos( x_mouse, y_mouse );
            found = TRUE;
            break;

        case RIGHT_MOUSE_UP_EVENT:
            *window = get_current_input_window();
            record_current_mouse_pos( x_mouse, y_mouse );
            found = TRUE;
            break;

        default:
            found = TRUE;
            break;
        }
    }

    return( type );
}

private  Gwindow  current_input_window = NULL;

private  Gwindow  get_current_input_window( void )
{
    return( current_input_window );
}

private  void  set_current_input_window(
    Gwindow  window )
{
    current_input_window = window;
}

public  BOOLEAN  G_is_mouse_in_window(
    Gwindow window )
{
    return( current_input_window == window && window != (Gwindow) NULL );
}

public  void  G_get_mouse_screen_position(
    int            *x_screen_pos,
    int            *y_screen_pos )
{
    int       x_mouse, y_mouse;
    Gwindow   window;

    window = get_current_input_window();

    if( window != NULL && G_get_mouse_position( window, &x_mouse, &y_mouse ) )
    {
        *x_screen_pos = x_mouse + window->x_origin;
        *y_screen_pos = y_mouse + window->y_origin;
    }
    else
    {
        *x_screen_pos = 0;
        *y_screen_pos = 0;
    }
}

public  BOOLEAN  G_get_mouse_position(
    Gwindow        window,
    int            *x_pixel_pos,
    int            *y_pixel_pos )
{
    BOOLEAN        in_window;

    in_window = G_is_mouse_in_window( window );

    if( window != (Gwindow) NULL )
    {
        *x_pixel_pos = current_mouse_x;
        *y_pixel_pos = current_mouse_y;
    }
    
    return( in_window );
}

public  BOOLEAN  G_get_mouse_position_0_to_1(
    Gwindow        window,
    Real           *x_pos,
    Real           *y_pos )
{
    int            x_pixel, y_pixel;
    BOOLEAN        in_window;

    in_window = G_get_mouse_position( window, &x_pixel, &y_pixel );

    if( window != (Gwindow) NULL )
    {
        *x_pos = (Real) (x_pixel + 0.5) / (Real) window->x_size;
        *y_pos = (Real) (y_pixel + 0.5) / (Real) window->y_size;
    }
    
    return( in_window );
}

public  void  G_set_mouse_position(
    int   x_screen,
    int   y_screen )
{
    GS_set_mouse_position( x_screen, y_screen );
}

