#include  <internal_volume_io.h>
#include  <x_windows.h>
#include  <X11/keysym.h>

private  void  bind_special_keys();

public  Display   *X_get_display()
{
    static  BOOLEAN  first = TRUE;
    static  Display  *display;

    if( first )
    {
        first = FALSE;
        display = XOpenDisplay( 0 );
    }

    return( display );
}

public  int  X_get_screen()
{
    static  BOOLEAN  first = TRUE;
    static  int      default_screen;

    if( first )
    {
        first = FALSE;
        default_screen = DefaultScreen( X_get_display() );
    }

    return( default_screen );
}

private   int   wait_for_window(
    Display *display,    /* ARGSUSED */
    XEvent  *event,
    char    *arg )
{
    Window   *window;

    window = (void *) arg;

    if( event->type == Expose && event->xexpose.window == *window )
        return( TRUE );
    else
        return( FALSE );
}

public  Status  X_create_window_with_visual(
    char             title[],
    int              initial_x_pos,
    int              initial_y_pos,
    int              initial_x_size,
    int              initial_y_size,
    BOOLEAN          colour_map_mode,  
    XVisualInfo      *visual,
    Colormap         cmap,
    X_window_struct  *window )
{
    int                      event_mask;
    XEvent                   event;
    XSetWindowAttributes     cwa;

    if( initial_x_pos < 0 )
        initial_x_pos = 0;

    if( initial_y_pos < 0 )
        initial_y_pos = 0;

    if( initial_x_size <= 0 )
        initial_x_size = 100;

    if( initial_y_size <= 0 )
        initial_y_size = 100;

    cwa.colormap = cmap;
    cwa.border_pixel = 0;
    cwa.event_mask = StructureNotifyMask | ExposureMask;
    window->window_id = XCreateWindow( X_get_display(),
                               RootWindow( X_get_display(),
                                           X_get_screen() ),
                               initial_x_pos, initial_y_pos,
                               initial_x_size, initial_y_size,
                               0, visual->depth, InputOutput, visual->visual,
                               CWColormap|CWBorderPixel, &cwa );

    XStoreName( X_get_display(), window->window_id, title );

    event_mask = KeyPressMask |
                 KeyReleaseMask |
                 PointerMotionMask |
                 ButtonPressMask |
                 ButtonReleaseMask |
                 ExposureMask |
                 StructureNotifyMask |
                 FocusChangeMask;
    XSelectInput( X_get_display(), window->window_id, event_mask );

    bind_special_keys();

    XMapWindow( X_get_display(), window->window_id );

    XIfEvent( X_get_display(), &event, wait_for_window,
              (char *) (&window->window_id) );

    window->visual = *visual;
    window->colour_map_mode = colour_map_mode;
    if( colour_map_mode )
    {
        cmap = XCreateColormap( X_get_display(), window->window_id,
                                visual->visual, AllocAll );
        XSetWindowColormap( X_get_display(), window->window_id, cmap );

        window->colour_map = cmap;
    }

    return( OK );
}

public  void  X_delete_window(
    X_window_struct  *window )
{
    if( window->colour_map_mode )
        XFreeColormap( X_get_display(), window->colour_map );

    XDestroyWindow( X_get_display(), window->window_id );
}

public  int  X_get_n_colours(
    X_window_struct          *window )
{
    return( window->visual.colormap_size );
}

public  void  X_set_colour_map_entry(
    X_window_struct          *window,
    int                      index,
    int                      r,
    int                      g,
    int                      b )
{
    XColor   def;

    if( !window->colour_map_mode )
    {
        return;
    }

    def.red = r << 8;
    def.green = g << 8;
    def.blue = b << 8;
    def.flags = DoRed | DoGreen | DoBlue;
    def.pixel = index;

    XStoreColor( X_get_display(), window->colour_map, &def );
}

public  void  X_get_window_geometry(
    X_window_struct          *window,
    int                      *x_position,
    int                      *y_position,
    int                      *x_size,
    int                      *y_size )
{
    int           x, y;
    unsigned int  width, height, border_width, depth;
    Window        root;

    if( XGetGeometry( X_get_display(), window->window_id,
                      &root, &x, &y, &width, &height, &border_width, &depth ) )
    {
        *x_position = x;
        *y_position = y;
        *x_size = width;
        *y_size = height;
    }
    else
    {
        *x_position = 0;
        *y_position = 0;
        *x_size = 0;
        *y_size = 0;
    }
}

/*--------------------------------- events ----------------------------- */

private  BOOLEAN  translate_key(
    XEvent   *x_event,
    int      *key )
{
    int             char_count;
    char            buffer[10];
    KeySym          keysym;
    XComposeStatus  compose;

    char_count = XLookupString( &x_event->xkey, buffer, 10, &keysym, &compose );

    *key = buffer[0];

    return( char_count >= 1 );
}

public  BOOLEAN  X_get_event(
    Event_types          *event_type,
    Window_id            *window,
    event_info_struct    *info )
{
    XEvent   x_event;

    *event_type = NO_EVENT;
    *window = 0;
    
    while( *event_type == NO_EVENT && XPending( X_get_display() ) )
    {
        XNextEvent( X_get_display(), &x_event );
    
        switch( x_event.type )
        {
        case ButtonPress:
            *window = x_event.xbutton.window;
            info->x_mouse = x_event.xbutton.x;
            info->y_mouse = x_event.xbutton.y;
            switch( x_event.xbutton.button )
            {
            case Button1:
                *event_type = LEFT_MOUSE_DOWN_EVENT;  break;
            case Button2:
                *event_type = MIDDLE_MOUSE_DOWN_EVENT;  break;
            case Button3:
                *event_type = RIGHT_MOUSE_DOWN_EVENT;  break;
            }
            break;
    
        case ButtonRelease:
            *window = x_event.xbutton.window;
            info->x_mouse = x_event.xbutton.x;
            info->y_mouse = x_event.xbutton.y;
            switch( x_event.xbutton.button )
            {
            case Button1:
                *event_type = LEFT_MOUSE_UP_EVENT;  break;
            case Button2:
                *event_type = MIDDLE_MOUSE_UP_EVENT;  break;
            case Button3:
                *event_type = RIGHT_MOUSE_UP_EVENT;  break;
            }
            break;
    
        case ConfigureNotify:
            *event_type = WINDOW_RESIZE_EVENT;
            *window = x_event.xconfigure.window;
            info->x_position = x_event.xconfigure.x;
            info->y_position = x_event.xconfigure.y;
            info->x_size = x_event.xconfigure.width;
            info->y_size = x_event.xconfigure.height;
            break;
    
        case DestroyNotify:
            *event_type = WINDOW_QUIT_EVENT;
            *window = x_event.xdestroywindow.window;
            break;
    
        case EnterNotify:
            *event_type = WINDOW_ENTER_EVENT;
            *window = x_event.xcrossing.window;
            break;
    
        case LeaveNotify:
            *event_type = WINDOW_LEAVE_EVENT;
            *window = x_event.xcrossing.window;
            break;
    
        case FocusIn:
            *event_type = WINDOW_ENTER_EVENT;
            *window = x_event.xfocus.window;
            break;
    
        case FocusOut:
            *event_type = WINDOW_LEAVE_EVENT;
            *window = x_event.xfocus.window;
            break;
    
        case Expose:
            *event_type = WINDOW_REDRAW_EVENT;
            *window = x_event.xexpose.window;
            break;
    
        case KeyPress:
            if( translate_key( &x_event, &info->key_pressed ) )
            {
                *event_type = KEY_DOWN_EVENT;
                *window = x_event.xkey.window;
            }
            break;
    
        case KeyRelease:
            if( translate_key( &x_event, &info->key_pressed ) )
            {
                *event_type = KEY_UP_EVENT;
                *window = x_event.xkey.window;
            }
            break;
    
        case MotionNotify:
            *event_type = MOUSE_MOVEMENT_EVENT;
            *window = x_event.xmotion.window;
            info->x_mouse = x_event.xmotion.x;
            info->y_mouse = x_event.xmotion.y;
            break;
    
        case MapNotify:
            *event_type = WINDOW_DEICONIZED_EVENT;
            *window = x_event.xmap.window;
            break;
    
        case UnmapNotify:
            *event_type = WINDOW_ICONIZED_EVENT;
            *window = x_event.xunmap.window;
            break;
    
        default:
            *event_type = NO_EVENT;
            break;
        }
    }

    return( *event_type != NO_EVENT );
}

private  void  bind_key(
    KeySym   keysym,
    int      output_key_value )
{
    char  string[1];

    string[0] = (char) output_key_value;

    XRebindKeysym( X_get_display(), keysym, NULL, 0, string, 1 );
}

private  void  bind_special_keys()
{
    int      i;
    static   BOOLEAN  first = TRUE;
    static  struct  { KeySym keysym;  int  character; }  table[] =
         {
             { XK_Left,       LEFT_ARROW_KEY },
             { XK_Right,      RIGHT_ARROW_KEY },
             { XK_Down,       DOWN_ARROW_KEY },
             { XK_Up,         UP_ARROW_KEY },
             { XK_Shift_L,    LEFT_SHIFT_KEY },
             { XK_Shift_R,    RIGHT_SHIFT_KEY },
             { XK_Control_L,  LEFT_CTRL_KEY },
             { XK_Control_R,  RIGHT_CTRL_KEY },
             { XK_Alt_L,      LEFT_ALT_KEY },
             { XK_Alt_R,      RIGHT_ALT_KEY }
         };

    if( !first )
        return;

    first = FALSE;

    for_less( i, 0, SIZEOF_STATIC_ARRAY(table) )
        bind_key( table[i].keysym, table[i].character );
}

private  char  font_named_str[] = "*-helvetica-medium-r-normal--*-%d-*";

#define  MAX_NAMES   10
#define  MAX_FONT_SIZE_ERROR   5

public  BOOLEAN  X_get_font(
    Font_types       type,
    int              size,
    Font             *x_font )
{
    char     **names;
    int      n_returned, offset;
    STRING   pattern;

    offset = 0;

    do
    {
        if( type == FIXED_FONT )
            (void) strcpy( pattern, "fixed" );
        else
            (void) sprintf( pattern, font_named_str, 10 * (size + offset) );
        names = XListFonts( X_get_display(), pattern, MAX_NAMES, &n_returned );

        if( offset <= 0 )
            offset = -offset + 1;
        else
            offset = -offset;
    }
    while( n_returned == 0 &&
           (type != FIXED_FONT || offset <= MAX_FONT_SIZE_ERROR) );

    if( n_returned >= 1 )
    {
        *x_font = XLoadFont( X_get_display(), names[0] );
        XFreeFontNames( names );
    }

    return( n_returned >= 1 );
}

public  void  X_set_mouse_position(
    int                      x,
    int                      y )
{
    XWarpPointer( X_get_display(), None, None, 0, 0, 0, 0, x, y );
}
