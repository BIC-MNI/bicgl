 
#include  <internal_volume_io.h>
#include  <gs_specific.h>

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
    BOOLEAN             event_found;
    Window_id           window_id;
    event_info_struct   info;

    event_found = WS_get_event( type, &window_id, &info );
    *window = find_window_for_id( window_id );

    switch( *type )
    {
    case LEFT_MOUSE_UP_EVENT:
    case MIDDLE_MOUSE_UP_EVENT:
    case RIGHT_MOUSE_UP_EVENT:
    case LEFT_MOUSE_DOWN_EVENT:
    case MIDDLE_MOUSE_DOWN_EVENT:
    case RIGHT_MOUSE_DOWN_EVENT:
    case MOUSE_MOVEMENT_EVENT:
        *x_mouse = info.x_mouse;
        *y_mouse = (*window)->y_size - 1 - info.y_mouse;
        break;

    case KEY_UP_EVENT:
    case KEY_DOWN_EVENT:
        *key_pressed = info.key_pressed;
        break;

    case WINDOW_RESIZE_EVENT:
        if( *window != NULL )
        {
            (*window)->x_origin = info.x_position;
            (*window)->y_origin = info.y_position;
            (*window)->x_size = info.x_size;
            (*window)->y_size = info.y_size;
        }
        break;

    default:
        break;
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
    X_set_mouse_position( x_screen, y_screen );
}

public  BOOLEAN  WS_mouse_events_in_screen_coordinates()
{
    return( FALSE );
}