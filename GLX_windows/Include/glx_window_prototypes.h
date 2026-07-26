#ifndef  DEF_glx_window_prototypes
#define  DEF_glx_window_prototypes

  VIO_Status  WS_create_window(
    VIO_STR                 title,
    int                    initial_x_pos,
    int                    initial_y_pos,
    int                    initial_x_size,
    int                    initial_y_size,
    VIO_BOOL                colour_map_mode,
    VIO_BOOL                double_buffer_flag,
    VIO_BOOL                depth_buffer_flag,
    int                    n_overlay_planes,
    VIO_BOOL                *actual_colour_map_mode,
    VIO_BOOL                *actual_double_buffer_flag,
    VIO_BOOL                *actual_depth_buffer_flag,
    int                    *actual_n_overlay_planes,
    WS_window_struct       *window );

  void  WS_delete_window(
    WS_window_struct  *window );

  VIO_BOOL  WS_window_has_overlay_planes(
    WS_window_struct  *window );

  void  WS_set_current_window(
    WS_window_struct  *window,
    Bitplane_types    current_bitplanes );

  int    WS_get_n_overlay_planes( void );

  VIO_BOOL  WS_get_event(
    Event_types          *event_type,
    Window_id            *window,
    event_info_struct    *info );

  void  WS_get_window_position(
    WS_window_struct  *window,
    int               *x_pos,
    int               *y_pos );

  void  WS_get_window_size(
    WS_window_struct  *window,
    int               *x_size,
    int               *y_size );

  int   WS_get_window_content_scale( void );

  void  WS_set_colour_map_entry(
    WS_window_struct  *window,
    int               ind,
    VIO_Colour            colour );

  void  WS_set_overlay_colour_map_entry(
    WS_window_struct  *window,
    int               ind,
    VIO_Colour            colour );

  void  WS_swap_buffers(
    WS_window_struct  *window );

  void  WS_draw_text(
    Font_types   type,
    VIO_Real     size,
    VIO_STR      string );

  VIO_Real  WS_get_character_height(
    Font_types   type,
    VIO_Real     size );

  VIO_Real  WS_get_text_length(
    VIO_STR      str,
    Font_types   type,
    VIO_Real     size );

  void  WS_get_screen_size(
    int   *x_size,
    int   *y_size  );

  VIO_BOOL  WS_is_native_wayland( void );

#endif
