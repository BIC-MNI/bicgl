#ifndef  DEF_GRAPHICS
#define  DEF_GRAPHICS

#include  <bicpl.h>
#include  <random_order.h>
#include  <window_events.h>

typedef  enum  { PIXEL_VIEW, SCREEN_VIEW, WORLD_VIEW, MODEL_VIEW,
                 N_VIEW_TYPES }     View_types;

typedef  enum  { FLAT_SHADING, GOURAUD_SHADING }  Shading_types;

#define  N_LIGHTS  8

typedef  enum { 
                DIRECTIONAL_LIGHT,
                POINT_LIGHT,
                SPOT_LIGHT
              } Light_types;

typedef  enum { VIEWING_MATRIX, PROJECTION_MATRIX } Matrix_modes;

typedef  enum { LESS_OR_EQUAL } Depth_functions;



#define  NO_BELL       0
#define  SHORT_BELL    1
#define  LONG_BELL     2

/*--- if only doing 2D graphics, change the names of any 3D struct
      elements, so that they cannot be accessed, but the size of the
      window struct is still the same for 2D and 3D */

#ifdef  TWO_D_ONLY
#define  UNUSED_IF_2D(x)    GLUE(x,_unused_2d)
#else
#define  UNUSED_IF_2D(x)    x
#endif

#ifdef  WS_WINDOW_DEFINED

#define  WS_type  WS_window_struct
#define  GS_type  GS_window_struct

#else

#define  WS_WINDOW_DEFINED
struct  WS_window_struct;
struct  GS_window_struct;
#define  WS_type  struct  WS_window_struct
#define  GS_type  struct  GS_window_struct

#endif

typedef  struct
{
    WS_type                *WS_window;
    GS_type                *GS_window;
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
    int                    UNUSED_IF_2D(n_overlay_planes);
    Bitplane_types         current_bitplanes;
    BOOLEAN                automatic_clear_flag;
    BOOLEAN                bitplanes_cleared[N_BITPLANE_TYPES];

    Real                   UNUSED_IF_2D(front_clip_distance);
    Real                   UNUSED_IF_2D(back_clip_distance);
    BOOLEAN                UNUSED_IF_2D(perspective_flag);
    Real                   UNUSED_IF_2D(perspective_distance);
    BOOLEAN                UNUSED_IF_2D(stereo_flag);
    Real                   UNUSED_IF_2D(eye_separation);
    Real                   UNUSED_IF_2D(window_width);
    Real                   UNUSED_IF_2D(window_height);

    BOOLEAN                double_buffer_state;

    BOOLEAN                UNUSED_IF_2D(zbuffer_available);
    BOOLEAN                UNUSED_IF_2D(zbuffer_state);
    BOOLEAN                colour_map_state;

    BOOLEAN                UNUSED_IF_2D(shaded_mode_state);
    Shading_types          UNUSED_IF_2D(shading_type);
    BOOLEAN                UNUSED_IF_2D(lighting_state);
    BOOLEAN                UNUSED_IF_2D(transparency_state);
    BOOLEAN                UNUSED_IF_2D(backface_culling_state);
    BOOLEAN                UNUSED_IF_2D(render_lines_as_curves_state);
    int                    UNUSED_IF_2D(n_curve_segments);
    BOOLEAN                UNUSED_IF_2D(marker_labels_visibility);

    int                    x_viewport_min;
    int                    x_viewport_max;
    int                    y_viewport_min;
    int                    y_viewport_max;

    BOOLEAN                UNUSED_IF_2D(interrupt_allowed);
    int                    UNUSED_IF_2D(interrupt_size);
    BOOLEAN                UNUSED_IF_2D(continuation_flag);
    BOOLEAN                UNUSED_IF_2D(interrupt_occurred);
    int                    UNUSED_IF_2D(interrupt_interval);
    Real                   UNUSED_IF_2D(interrupt_time);
    int                    UNUSED_IF_2D(next_item);
    int                    UNUSED_IF_2D(n_items_done);
} window_struct;

typedef  window_struct   *Gwindow;

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
