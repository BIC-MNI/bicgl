#ifndef  DEF_GRAPHICS
#define  DEF_GRAPHICS

#include  <GS_graphics.h>
#include  <bicpl.h>
#include  <random_order.h>

typedef  struct   window_struct
{
    GSwindow               GS_window;
    int                    x_origin, y_origin;          
    int                    x_size, y_size;          
    Colour                 background_colour;
    Colour                 background_colour_index;
    View_types             current_view_type;
    Transform              projection_matrices[(int) N_VIEW_TYPES];
    Transform              left_projection_matrices[(int) N_VIEW_TYPES];
    Transform              right_projection_matrices[(int) N_VIEW_TYPES];
    Transform              viewing_matrices[(int) N_VIEW_TYPES];
    Transform              modeling_transform;
    int                    n_overlay_planes;
    Bitplane_types         current_bitplanes;
    VIO_BOOL                automatic_clear_flag;
    VIO_BOOL                bitplanes_cleared[N_BITPLANE_TYPES];

    Real                   front_clip_distance;
    Real                   back_clip_distance;
    VIO_BOOL                perspective_flag;
    Real                   perspective_distance;
    VIO_BOOL                stereo_flag;
    Real                   eye_separation;
    Real                   window_width;
    Real                   window_height;

    VIO_BOOL                double_buffer_available;
    VIO_BOOL                double_buffer_state;

    VIO_BOOL                zbuffer_available;
    VIO_BOOL                zbuffer_state;
    VIO_BOOL                colour_map_state;

    VIO_BOOL                shaded_mode_state;
    Shading_types          shading_type;
    VIO_BOOL                lighting_state;
    VIO_BOOL                transparency_state;
    VIO_BOOL                backface_culling_state;
    VIO_BOOL                render_lines_as_curves_state;
    int                    n_curve_segments;
    VIO_BOOL                marker_labels_visibility;

    int                    x_viewport_min;
    int                    x_viewport_max;
    int                    y_viewport_min;
    int                    y_viewport_max;

    VIO_BOOL                interrupt_allowed;
    VIO_BOOL                continuation_flag;
    VIO_BOOL                interrupt_occurred;
    int                    interrupt_interval;
    Real                   interrupt_time;
    Real                   interrupt_time_interval;
    int                    next_item;
    int                    n_items_done;

    VIO_BOOL                update_required_flag;
    Real                   last_update_time;
    VIO_BOOL                last_update_was_idle;
    int                    n_update_timers;
    int                    n_update_timers_to_ignore;

    VIO_BOOL                overlay_update_required_flag;
    Real                   last_overlay_update_time;
    VIO_BOOL                last_overlay_update_was_idle;

    Real                   min_update_time;

    void    (*update_callback) ( struct window_struct *, void * );
    void    *update_data;

    void    (*update_overlay_callback) ( struct window_struct *, void * );
    void    *update_overlay_data;

    void    (*resize_callback) ( struct window_struct *, int, int, int, int,
                                 void * );
    void    *resize_data;

    void    (*key_down_callback) ( struct window_struct *, int, void * );
    void    *key_down_data;

    void    (*key_up_callback) ( struct window_struct *, int, void * );
    void    *key_up_data;

    void    (*mouse_movement_callback) ( struct window_struct *, int, int,
                                         void * );
    void    *mouse_movement_data;

    void    (*left_mouse_down_callback) ( struct window_struct *, int, int,
                                          void * );
    void    *left_mouse_down_data;

    void    (*left_mouse_up_callback) ( struct window_struct *, int, int,
                                          void * );
    void    *left_mouse_up_data;

    void    (*middle_mouse_down_callback) ( struct window_struct *, int, int,
                                          void * );
    void    *middle_mouse_down_data;

    void    (*middle_mouse_up_callback) ( struct window_struct *, int, int,
                                          void * );
    void    *middle_mouse_up_data;

    void    (*right_mouse_down_callback) ( struct window_struct *, int, int,
                                          void * );
    void    *right_mouse_down_data;

    void    (*right_mouse_up_callback) ( struct window_struct *, int, int,
                                          void * );
    void    *right_mouse_up_data;

    void    (*iconify_callback) ( struct window_struct *, void * );
    void    *iconify_data;

    void    (*deiconify_callback) ( struct window_struct *, void * );
    void    *deiconify_data;

    void    (*enter_callback) ( struct window_struct *, void * );
    void    *enter_data;

    void    (*leave_callback) ( struct window_struct *, void * );
    void    *leave_data;

    void    (*quit_callback) ( struct window_struct *, void * );
    void    *quit_data;
} window_struct;

typedef  struct  window_struct   *Gwindow;

#include  <graphics_struct.h>

#ifndef  public
#define       public
#define       public_was_defined_here
#endif

#include  <graphics_prototypes.h>

#ifdef  public_was_defined_here
#undef       public
#undef       public_was_defined_here
#endif

#endif
