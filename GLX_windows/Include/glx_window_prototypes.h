#ifndef  DEF_glx_window_prototypes
#define  DEF_glx_window_prototypes

public  Status  WS_create_window(
    STRING                 title,
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
    WS_window_struct       *window );

public  void  WS_delete_window(
    WS_window_struct  *window );

public  BOOLEAN  WS_window_has_overlay_planes(
    WS_window_struct  *window );

public  void  WS_set_current_window(
    WS_window_struct  *window,
    Bitplane_types    current_bitplanes );

public  int    WS_get_n_overlay_planes( void );

public  BOOLEAN  WS_get_event(
    Event_types          *event_type,
    Window_id            *window,
    event_info_struct    *info );

public  void  WS_get_window_position(
    WS_window_struct  *window,
    int               *x_pos,
    int               *y_pos );

public  void  WS_get_window_size(
    WS_window_struct  *window,
    int               *x_size,
    int               *y_size );

public  void  WS_set_colour_map_entry(
    WS_window_struct  *window,
    int               ind,
    Colour            colour );

public  void  WS_set_overlay_colour_map_entry(
    WS_window_struct  *window,
    int               ind,
    Colour            colour );

public  void  WS_swap_buffers(
    WS_window_struct  *window );

public  BOOLEAN  WS_get_font(
    Font_types       type,
    Real             size,
    WS_font_info     *font_info );

public  void  WS_build_font_in_window(
    WS_window_struct  *window,
    int               font_index,
    WS_font_info      *font_info );

public  void  WS_delete_font_in_window(
    WS_window_struct     *window,
    int                  font_index,
    WS_font_info         *font_info );

public  BOOLEAN  WS_set_font(
    WS_window_struct     *window,
    int                  font_index );

public  void  WS_delete_font(
    WS_font_info  *info );

public  Real  WS_get_character_height(
    WS_font_info  *font_info );

public  Real  WS_get_text_length(
    WS_font_info     *font_info,
    STRING           str );

public  void  WS_get_screen_size(
    int   *x_size, 
    int   *y_size  );

public  void create_fixed_font(
    GLuint fontOffset );

public  int  get_fixed_font_n_chars( void );

public  Real  get_fixed_font_height( void );

public  Real  get_fixed_font_width(
    char   ch );
#endif
