#ifndef  DEF_x_window_prototypes
#define  DEF_x_window_prototypes

public  Display   *X_get_display( void );

public  int  X_get_screen( void );

public  Status  X_create_window_with_visual(
    STRING           title,
    int              initial_x_pos,
    int              initial_y_pos,
    int              initial_x_size,
    int              initial_y_size,
    BOOLEAN          colour_map_mode,  
    XVisualInfo      *visual,
    Colormap         cmap,
    X_window_struct  *window );

public  Status  X_create_overlay_window(
    X_window_struct  *window,
    XVisualInfo      *visual,
    X_window_struct  *overlay_window );

public  void  X_delete_window(
    X_window_struct  *window );

public  int  X_get_n_colours(
    X_window_struct          *window );

public  void  X_set_colour_map_entry(
    X_window_struct          *window,
    int                      ind,
    int                      r,
    int                      g,
    int                      b );

public  void  X_get_window_geometry(
    X_window_struct          *window,
    int                      *x_position,
    int                      *y_position,
    int                      *x_size,
    int                      *y_size );

public  void  X_get_screen_size(
    int    *x_size,
    int    *y_size );

public  BOOLEAN  X_get_event(
    Event_types          *event_type,
    Window_id            *window,
    event_info_struct    *info );

public  BOOLEAN  X_get_font_name(
    Font_types       type,
    int              size,
    STRING           *font_name );

public  void  X_set_mouse_position(
    int                      x,
    int                      y );
#endif
