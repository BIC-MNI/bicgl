#ifndef  DEF_graphics_prototypes
#define  DEF_graphics_prototypes

  void  initialize_display_interrupts(
    Gwindow        window );

  void  G_set_drawing_interrupt_state(
    Gwindow          window,
    VIO_BOOL         state );

  VIO_BOOL  G_get_drawing_interrupt_state(
    Gwindow          window );

  void  G_set_drawing_interrupt_check_n_objects(
    Gwindow          window,
    int             interval );

  void  G_set_drawing_interrupt_time(
    Gwindow          window,
    VIO_Real             interval );

  void  G_start_interrupt_test(
    Gwindow          window );

  VIO_BOOL  G_get_interrupt_occurred(
    Gwindow          window );

  void  G_clear_drawing_interrupt_flag(
    Gwindow          window );

  void  set_continuation_flag(
    Gwindow         window,
    VIO_BOOL         state );

  void  initialize_surface_property(
    Gwindow        window );

  VIO_BOOL  view_is_stereo(
    Gwindow        window );

  void  G_draw_polygons(
    Gwindow         window,
    polygons_struct *polygons );

  void  G_draw_quadmesh(
    Gwindow         window,
    quadmesh_struct *quadmesh );

  void  G_draw_lines(
    Gwindow         window,
    lines_struct    *lines );

  VIO_Real  G_get_text_height(
    Font_types       type,
    VIO_Real             size );

  VIO_Real  G_get_text_length(
    VIO_STR           str,
    Font_types       type,
    VIO_Real             size );

  void  G_draw_text(
    Gwindow         window,
    text_struct     *text );

  void  G_draw_marker(
    Gwindow         window,
    marker_struct   *marker );

  void  G_draw_pixels(
    Gwindow         window,
    pixels_struct   *pixels );

  void  G_read_pixels(
    Gwindow         window,
    int             x_min,
    int             x_max,
    int             y_min,
    int             y_max,
    VIO_Colour          pixels[] );

  void  draw_object(
    Gwindow         window,
    object_struct   *object );

  VIO_BOOL  G_get_left_mouse_button( void );

  VIO_BOOL  G_get_middle_mouse_button( void );

  VIO_BOOL  G_get_right_mouse_button( void );

  VIO_BOOL  G_get_shift_key_state( void );

  VIO_BOOL  G_get_ctrl_key_state( void );

  VIO_BOOL  G_get_alt_key_state( void );

  void  G_set_update_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data );

  void  G_set_update_overlay_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data );

  void  G_set_resize_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, int, int, void * ),
    void                    *func_data );

  void  G_set_key_down_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, void * ),
    void                    *func_data );

  void  G_set_key_up_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, void * ),
    void                    *func_data );

  void  G_set_mouse_movement_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data );

  void  G_set_left_mouse_down_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data );

  void  G_set_left_mouse_up_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data );

  void  G_set_middle_mouse_down_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data );

  void  G_set_middle_mouse_up_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data );

  void  G_set_right_mouse_down_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data );

  void  G_set_right_mouse_up_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data );

  void  G_set_scroll_down_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data );

  void  G_set_scroll_up_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, int, int, void * ),
    void                    *func_data );

  void  G_set_iconify_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data );

  void  G_set_deiconify_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data );

  void  G_set_window_enter_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data );

  void  G_set_window_leave_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data );

  void  G_set_window_quit_function(
    Gwindow                 window,
    void                    (*func) ( Gwindow, void * ),
    void                    *func_data );

  void  initialize_callbacks_for_window(
    Gwindow                 window );

  void  G_set_update_flag(
    Gwindow  window );

  void  G_add_timer_function(
    VIO_Real          seconds,
    void          (*func) ( void * ),
    void          *data );

  void  G_add_idle_function(
    void  (*func) ( void * ),
    void          *data );

  void  G_remove_idle_function(
    void  (*func) ( void * ),
    void          *data );

  void  G_main_loop( void );

  void  G_set_mouse_position(
    int       x_screen,
    int       y_screen );

  VIO_BOOL  G_is_mouse_in_window(
    Gwindow window );

  void  G_get_mouse_screen_position(
    int            *x_screen_pos,
    int            *y_screen_pos );

  VIO_BOOL  G_get_mouse_position(
    Gwindow        window,
    int            *x_pixel_pos,
    int            *y_pixel_pos );

  VIO_BOOL  G_get_mouse_position_0_to_1(
    Gwindow        window,
    VIO_Real           *x_pos,
    VIO_Real           *y_pos );

  void  G_set_default_update_min_interval(
    VIO_Real   seconds );

  void  G_set_window_update_min_interval(
    Gwindow   window,
    VIO_Real      seconds );

  void  initialize_graphics_struct(
    graphics_struct   *graphics );

  void  delete_graphics_struct(
    graphics_struct   *graphics );

  void  set_graphics_viewport(
    graphics_struct   *graphics,
    int               viewport_index,
    int               x_min,
    int               x_max,
    int               y_min,
    int               y_max );

  void  get_graphics_viewport(
    graphics_struct   *graphics,
    int               viewport_index,
    int               *x_min,
    int               *x_max,
    int               *y_min,
    int               *y_max );

  void  set_graphics_viewport_background(
    graphics_struct   *graphics,
    int               viewport_index,
    VIO_Colour            background_colour,
    int               background_colour_index );

  void  add_object_to_viewport(
    graphics_struct   *graphics,
    int               viewport_index,
    Bitplane_types    bitplane,
    object_struct     *object );

  void  remove_object_from_viewport(
    graphics_struct   *graphics,
    int               viewport_index,
    Bitplane_types    bitplane,
    object_struct     *object );

  void  set_viewport_update_flag( 
    graphics_struct   *graphics,
    int               viewport_index,
    Bitplane_types    bitplane );

  VIO_BOOL  get_viewport_update_flag( 
    graphics_struct   *graphics,
    int               viewport_index,
    Bitplane_types    bitplane,
    int               buffer );

  void  set_bitplanes_clear_flag( 
    graphics_struct   *graphics,
    Bitplane_types    bitplane );

  void  set_viewport_bitplane_objects_visibility(
    graphics_struct   *graphics,
    int               viewport_index,
    Bitplane_types    bitplane,
    VIO_BOOL           visibility );

  void  set_viewport_objects_visibility(
    graphics_struct   *graphics,
    int               viewport_index,
    VIO_BOOL           visibility );

  VIO_BOOL  redraw_out_of_date_viewports(
    graphics_struct   *graphics,
    Gwindow           window,
    int               current_buffer );

  void   initialize_graphics_lights(
    Gwindow  window );

  void  G_set_ambient_light(
    Gwindow        window,
    VIO_Colour         colour );

  void  G_define_light(
    Gwindow         window,
    int             light_index,
    Light_types     type,
    VIO_Colour          colour,
    VIO_Vector          *direction,
    VIO_Point           *position,
    VIO_Real            spot_exponent,
    VIO_Real            spot_angle );

  void  G_set_light_state(
    Gwindow         window,
    int             light_index,
    VIO_BOOL         state );

  Random_mask_type  get_random_order_mask(
    int  size );

  void  G_set_shaded_state(
    Gwindow      window,
    VIO_BOOL      state );

  void  G_set_shading_type(
    Gwindow        window,
    Shading_types  type );

  VIO_BOOL  G_get_lighting_state(
    Gwindow      window );

  void  G_set_lighting_state(
    Gwindow      window,
    VIO_BOOL      state );

  void  G_backface_culling_state(
    Gwindow     window,
    VIO_BOOL     state );

  void  G_set_render_lines_as_curves_state(
    Gwindow     window,
    VIO_BOOL     state );

  void  G_set_n_curve_segments(
    Gwindow  window,
    int      n_segments );

  void  G_set_markers_labels_visibility(
    Gwindow    window,
    VIO_BOOL    state );

  void  initialize_window_view(
    Gwindow   window );

  void  set_view_for_eye(
    Gwindow         window,
    int             which_eye );

  void  G_set_view_type(
    Gwindow         window,
    View_types      view_type );

  void  G_set_3D_view(
    Gwindow        window,
    VIO_Point          *origin,
    VIO_Vector         *line_of_sight,
    VIO_Vector         *up_direction,
    VIO_Real           front_clip_distance,
    VIO_Real           back_clip_distance,
    VIO_BOOL        perspective_flag,
    VIO_Real           perspective_distance,
    VIO_BOOL        stereo_flag,
    VIO_Real           eye_separation,
    VIO_Real           window_width,
    VIO_Real           window_height );

  void  G_set_modeling_transform(
    Gwindow         window,
    VIO_Transform       *transform );

  void  G_transform_point(
    Gwindow         window,
    VIO_Point           *point,
    View_types      view_type,
    int             *x_pixel,
    int             *y_pixel );

  void  update_transforms(
    Gwindow         window );

  void  G_push_transform(
    Gwindow      window,
    VIO_Transform    *transform );

  void  G_pop_transform(
    Gwindow    window );

  void  window_was_resized(
    Gwindow    window );

  void  G_set_viewport(
    Gwindow        window,
    int            x_min,
    int            x_max,
    int            y_min,
    int            y_max );

  int  get_n_graphics_windows( void );

  Gwindow  get_nth_graphics_window(
    int   i );

  void  set_current_window( Gwindow   window );

  Gwindow  get_current_window( void );

  VIO_Status  G_create_window(
    VIO_STR        title,
    int            x_pos,
    int            y_pos,
    int            width,
    int            height,
    VIO_BOOL       colour_map_desired,
    VIO_BOOL       double_buffer_desired,
    VIO_BOOL       depth_buffer_desired,
    int            n_overlay_planes_desired,
    Gwindow        *window );

  VIO_Status  G_create_child_window(
    VIO_STR        title,
    int            x_pos,
    int            y_pos,
    int            width,
    int            height,
    VIO_BOOL       colour_map_desired,
    VIO_BOOL       double_buffer_desired,
    VIO_BOOL       depth_buffer_desired,
    int            n_overlay_planes_desired,
    Gwindow        *window,
    Gwindow        parent);

  VIO_Status  G_delete_window(
    Gwindow   window );

  void  G_set_window_title(
    Gwindow   window,
    VIO_STR    title );

  void  G_terminate( void );

  VIO_BOOL  G_is_double_buffer_supported( void );

  VIO_BOOL  G_get_double_buffer_state(
    Gwindow        window );

  void  G_set_double_buffer_state(
    Gwindow        window,
    VIO_BOOL        flag );

  VIO_BOOL  G_is_depth_buffer_supported( void );

  void  G_set_zbuffer_state(
    Gwindow         window,
    VIO_BOOL         flag );

  VIO_BOOL  G_get_zbuffer_state(
    Gwindow         window );

  VIO_BOOL  G_get_colour_map_state(
    Gwindow        window );

  void  G_set_colour_map_state(
    Gwindow        window,
    VIO_BOOL        flag );

  int  G_get_n_colour_map_entries(
    Gwindow        window );

  void  G_set_colour_map_entry(
    Gwindow         window,
    int             ind,
    VIO_Colour          colour );

  void  restore_bitplanes(
    Gwindow         window,
    Bitplane_types  bitplane );

  int  G_get_monitor_width( void );

  int  G_get_monitor_height( void );

  VIO_Real  G_get_monitor_widths_to_eye( void );

  void  G_get_window_size(
    Gwindow        window,
    int            *x_size,
    int            *y_size );

  void  G_get_window_position(
    Gwindow        window,
    int            *x_pos,
    int            *y_pos );

  void  G_set_background_colour(
    Gwindow         window,
    VIO_Colour          colour );

  VIO_Colour  G_get_background_colour(
    Gwindow         window );

  void  G_clear_window(
    Gwindow    window );

  void  G_clear_viewport(
    Gwindow    window,
    VIO_Colour     colour );

  void  G_set_automatic_clear_state(
    Gwindow    window,
    VIO_BOOL    state );

  void  G_update_window( Gwindow   window );

  void  check_window_cleared(
     Gwindow    window  );

  VIO_BOOL  G_has_overlay_planes( void );

  VIO_BOOL  G_window_has_overlay_planes(
    Gwindow   window );

void G_set_current_window(Gwindow window);

  void  G_set_bitplanes(
    Gwindow          window,
    Bitplane_types   bitplanes );

  Bitplane_types  G_get_bitplanes(
    Gwindow          window );

  VIO_BOOL  G_can_switch_double_buffering( void );

  VIO_BOOL  G_can_switch_colour_map_mode( void );

  VIO_BOOL  G_has_transparency_mode( void );

  VIO_BOOL  G_has_rgb_mode( void );

  int  G_get_n_overlay_planes( void );

  void  G_set_overlay_colour_map(
    Gwindow         window,
    int             ind,
    VIO_Colour          colour );

  void  G_append_to_last_update(
     Gwindow   window );

  void  G_continue_last_update(
     Gwindow   window );

  void  G_set_transparency_state(
    Gwindow        window,
    VIO_BOOL        state );

  void  G_set_visibility(
    Gwindow        window,
    VIO_BOOL       is_visible );

void G_set_geometry(Gwindow window, int x, int y, int width, int height);
    
#endif
