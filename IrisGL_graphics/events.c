 
#include  <internal_volume_io.h>
#include  <graphics.h>
#include  <queue.h>
#include  <gl/device.h>

private    BOOLEAN    left_mouse_down = FALSE;
private    BOOLEAN    middle_mouse_down = FALSE;
private    BOOLEAN    right_mouse_down = FALSE;

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
    Gwindow        *window,
    int            *x_mouse,
    int            *y_mouse,
    int            *key_pressed )
{
    BOOLEAN       event_found;
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
                *window = (Gwindow) NULL;
            }
            else
            {
                *type = WINDOW_ENTER_EVENT;
                *window = find_window_for_id( (Window_id) val );
            }
            event_found = TRUE;
            break;

        case WINQUIT:
        case WINSHUT:
            *window = find_window_for_id( (Window_id) val );
            *type = WINDOW_QUIT_EVENT;
            event_found = TRUE;
            break;

        case DRAWOVERLAY:
            *window = find_window_for_id( (Window_id) val );
            *type = REDRAW_OVERLAY_EVENT;
            event_found = TRUE;
            break;

        case DEPTHCHANGE:
            *window = find_window_for_id( (Window_id) val );
            if( *window != (Gwindow) NULL )
                clear_overlay_planes();
            *type = REDRAW_OVERLAY_EVENT;
            event_found = TRUE;
            break;

        case WINTHAW:
            *window = find_window_for_id( (Window_id) val );
            *type = WINDOW_DEICONIZED_EVENT;
            event_found = TRUE;
            break;

        case WINFREEZE:
            *window = find_window_for_id( (Window_id) val );
            if( *window != (Gwindow) NULL )
                clear_overlay_planes();
            *type = WINDOW_ICONIZED_EVENT;
            event_found = TRUE;
            break;

        case REDRAW:
            *window = find_window_for_id( (Window_id) val );
            if( *window != (Gwindow) NULL )
            {
                clear_overlay_planes();
                getsize( &x_pos, &y_pos );
                (*window)->x_origin = x_pos;
                (*window)->y_origin = y_pos;

                getsize( &x_size, &y_size );
                if( (*window)->x_size != x_size || (*window)->y_size != y_size )
                {
                    *type = WINDOW_RESIZE_EVENT;
                    (*window)->x_size = x_size;
                    (*window)->y_size = y_size;
                }
                else
                    *type = WINDOW_REDRAW_EVENT;
                event_found = TRUE;
            }
            break;

        case LEFTARROWKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            if( key_pressed != (int *) 0 )
                *key_pressed = LEFT_ARROW_KEY;
            break;

        case RIGHTARROWKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            if( key_pressed != (int *) 0 )
                *key_pressed = RIGHT_ARROW_KEY;
            break;

        case DOWNARROWKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            if( key_pressed != (int *) 0 )
                *key_pressed = DOWN_ARROW_KEY;
            break;

        case UPARROWKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            if( key_pressed != (int *) 0 )
                *key_pressed = UP_ARROW_KEY;
            break;

        case LEFTSHIFTKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            if( key_pressed != (int *) 0 )
                *key_pressed = LEFT_SHIFT_KEY;
            break;

        case RIGHTSHIFTKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            if( key_pressed != (int *) 0 )
                *key_pressed = RIGHT_SHIFT_KEY;
            break;

        case LEFTCTRLKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            if( key_pressed != (int *) 0 )
                *key_pressed = LEFT_CTRL_KEY;
            break;

        case RIGHTCTRLKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            if( key_pressed != (int *) 0 )
                *key_pressed = RIGHT_CTRL_KEY;
            break;

        case LEFTALTKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            if( key_pressed != (int *) 0 )
                *key_pressed = LEFT_ALT_KEY;
            break;

        case RIGHTALTKEY:
            *type = get_key_up_or_down_event( val );
            event_found = TRUE;
            if( key_pressed != (int *) 0 )
                *key_pressed = RIGHT_ALT_KEY;
            break;

        case KEYBD:
            *type = KEY_DOWN_EVENT;
            event_found = TRUE;
            if( key_pressed != (int *) 0 )
                *key_pressed = val;
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
            if( val && !left_mouse_down )
            {
                *type = LEFT_MOUSE_DOWN_EVENT;
                left_mouse_down = TRUE;
                event_found = TRUE;
            }
            else if( !val && left_mouse_down )
            {
                *type = LEFT_MOUSE_UP_EVENT;
                left_mouse_down = FALSE;
                event_found = TRUE;
            }
            break;

        case MIDDLEMOUSE:
            if( val && !middle_mouse_down )
            {
                *type = MIDDLE_MOUSE_DOWN_EVENT;
                middle_mouse_down = TRUE;
                event_found = TRUE;
            }
            else if( !val && middle_mouse_down )
            {
                *type = MIDDLE_MOUSE_UP_EVENT;
                middle_mouse_down = FALSE;
                event_found = TRUE;
            }
            break;

        case RIGHTMOUSE:
            if( val && !right_mouse_down )
            {
                *type = RIGHT_MOUSE_DOWN_EVENT;
                right_mouse_down = TRUE;
                event_found = TRUE;
            }
            else if( !val && right_mouse_down )
            {
                *type = RIGHT_MOUSE_UP_EVENT;
                right_mouse_down = FALSE;
                event_found = TRUE;
            }
            break;

        case QFULL:
            print( "Error: GL event queue full.\n" );
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
    setvaluator( MOUSEX, x_screen, 0, 10000 );
    setvaluator( MOUSEY, y_screen, 0, 10000 );
}

public  BOOLEAN  WS_mouse_events_in_screen_coordinates()
{
    return( TRUE );
}
