 
#include  <internal_volume_io.h>
#include  <gs_specific.h>
#include  <queue.h>
#include  <gl/device.h>

/* ----------------------------- MNI Header -----------------------------------
@NAME       : initialize_window_events
@INPUT      : window
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Initializes and enables window events for the given window.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

/* ARGSUSED */

public  void  initialize_window_events(
     Gwindow        window )
{
    qdevice( KEYBD );

    qdevice( LEFTARROWKEY );
    qdevice( RIGHTARROWKEY );
    qdevice( DOWNARROWKEY );
    qdevice( UPARROWKEY );
    qdevice( LEFTSHIFTKEY );
    qdevice( RIGHTSHIFTKEY );
    qdevice( LEFTALTKEY );
    qdevice( RIGHTALTKEY );
    qdevice( LEFTCTRLKEY );
    qdevice( RIGHTCTRLKEY );

    qdevice( REDRAW );
    qdevice( DEPTHCHANGE );
    qdevice( DRAWOVERLAY );
    qdevice( INPUTCHANGE );

    qdevice( MOUSEX );
    qdevice( MOUSEY );
    qdevice( LEFTMOUSE );
    qdevice( MIDDLEMOUSE );
    qdevice( RIGHTMOUSE );

    qdevice( WINFREEZE );
    qdevice( WINTHAW );
    qdevice( WINQUIT );
    qdevice( WINSHUT );

    qdevice( QFULL );
}

private  Event_types  get_key_up_or_down_event(
    short   val )
{
    if( val )
        return( KEY_DOWN_EVENT );
    else
        return( KEY_UP_EVENT );
}

private  int  current_mouse_x = -1;
private  int  current_mouse_y = -1;

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_get_event
@INPUT      : 
@OUTPUT     : type
              window
              x_mouse
              y_mouse
              key_pressed
@RETURNS    : TRUE if an event is found
@DESCRIPTION: Gets a mouse, keyboard, or window event, if any.
              Does NOT block, waiting for an event.  Returns FALSE if no
              event is available.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  BOOLEAN  GS_get_event(
    Event_types    *type,
    Window_id      *window_id,
    int            *x_mouse,
    int            *y_mouse,
    int            *key_pressed )
{
    BOOLEAN       event_found;
    Gwindow       event_window;
    long          dev;
    short         val;
    long          x_size, y_size, x_pos, y_pos;

    event_found = FALSE;

    while( !event_found && qtest() )
    {
        dev = qread( &val );

        switch( dev )
        {
        case INPUTCHANGE:
            if( val == 0 )
            {
                *type = WINDOW_LEAVE_EVENT;
                *window_id = 0;
            }
            else
            {
                *type = WINDOW_ENTER_EVENT;
                *window_id = (Window_id) val;
            }
            event_found = TRUE;
            break;

        case WINQUIT:
        case WINSHUT:
            *window_id = (Window_id) val;
            *type = WINDOW_QUIT_EVENT;
            event_found = TRUE;
            break;

        case DRAWOVERLAY:
            *window_id = (Window_id) val;
            *type = REDRAW_OVERLAY_EVENT;
            event_found = TRUE;
            break;

        case DEPTHCHANGE:
            *window_id = (Window_id) val;
            event_window = find_window_for_id( *window_id );
            if( event_window != (Gwindow) NULL )
                clear_overlay_planes();
            *type = REDRAW_OVERLAY_EVENT;
            event_found = TRUE;
            break;

        case WINTHAW:
            *window_id = (Window_id) val;
            *type = WINDOW_DEICONIZED_EVENT;
            event_found = TRUE;
            break;

        case WINFREEZE:
            *window_id = (Window_id) val;
            event_window = find_window_for_id( *window_id );
            if( event_window != (Gwindow) NULL )
                clear_overlay_planes();
            *type = WINDOW_ICONIZED_EVENT;
            event_found = TRUE;
            break;

        case REDRAW:
            *window_id = (Window_id) val;
            event_window = find_window_for_id( *window_id );
            if( event_window != (Gwindow) NULL )
            {
                set_current_window( event_window );
                clear_overlay_planes();
                getorigin( &x_pos, &y_pos );
                event_window->x_origin = (int) x_pos;
                event_window->y_origin = (int) y_pos;

                getsize( &x_size, &y_size );
                if( event_window->x_size != (int) x_size ||
                    event_window->y_size != (int) y_size )
                {
                    *type = WINDOW_RESIZE_EVENT;
                    event_window->x_size = (int) x_size;
                    event_window->y_size = (int) y_size;
                }
                else
                    *type = WINDOW_REDRAW_EVENT;
                event_found = TRUE;
            }
            break;

        case LEFTARROWKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            *key_pressed = LEFT_ARROW_KEY;
            break;

        case RIGHTARROWKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            *key_pressed = RIGHT_ARROW_KEY;
            break;

        case DOWNARROWKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            *key_pressed = DOWN_ARROW_KEY;
            break;

        case UPARROWKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            *key_pressed = UP_ARROW_KEY;
            break;

        case LEFTSHIFTKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            *key_pressed = LEFT_SHIFT_KEY;
            break;

        case RIGHTSHIFTKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            *key_pressed = RIGHT_SHIFT_KEY;
            break;

        case LEFTCTRLKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            *key_pressed = LEFT_CTRL_KEY;
            break;

        case RIGHTCTRLKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            *key_pressed = RIGHT_CTRL_KEY;
            break;

        case LEFTALTKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            *key_pressed = LEFT_ALT_KEY;
            break;

        case RIGHTALTKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            *key_pressed = RIGHT_ALT_KEY;
            break;

        case KEYBD:
            *type = KEY_DOWN_EVENT;
            event_found = TRUE;
            *key_pressed = (int) val;
            break;

        case MOUSEX:
        case MOUSEY:
            if( dev == MOUSEX )
                current_mouse_x = (int) val;
            else
                current_mouse_y = (int) val;

            *x_mouse = current_mouse_x;
            *y_mouse = current_mouse_y;
            *type = MOUSE_MOVEMENT_EVENT;
            event_found = TRUE;
            break;

        case LEFTMOUSE:
            if( val )
            {
                *type = LEFT_MOUSE_DOWN_EVENT;
                event_found = TRUE;
            }
            else
            {
                *type = LEFT_MOUSE_UP_EVENT;
                event_found = TRUE;
            }
            *x_mouse = current_mouse_x;
            *y_mouse = current_mouse_y;
            break;

        case MIDDLEMOUSE:
            if( val )
            {
                *type = MIDDLE_MOUSE_DOWN_EVENT;
                event_found = TRUE;
            }
            else
            {
                *type = MIDDLE_MOUSE_UP_EVENT;
                event_found = TRUE;
            }
            *x_mouse = current_mouse_x;
            *y_mouse = current_mouse_y;
            break;

        case RIGHTMOUSE:
            if( val )
            {
                *type = RIGHT_MOUSE_DOWN_EVENT;
                event_found = TRUE;
            }
            else
            {
                *type = RIGHT_MOUSE_UP_EVENT;
                event_found = TRUE;
            }
            *x_mouse = current_mouse_x;
            *y_mouse = current_mouse_y;
            break;

        case QFULL:
            print_error( "Error: GL event queue full.\n" );
            break;

        default:
            break;
        }
    }

    return( event_found );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_set_mouse_position
@INPUT      : x_screen
              y_screen
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Sets the mouse position relative to the full screen.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  void  GS_set_mouse_position(
    int   x_screen,
    int   y_screen )
{
    setvaluator( MOUSEX, (short) x_screen, 0, 10000 );
    setvaluator( MOUSEY, (short) y_screen, 0, 10000 );
}

public  BOOLEAN  WS_mouse_events_in_screen_coordinates( void )
{
    return( TRUE );
}
