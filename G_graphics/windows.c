#include  <internal_volume_io.h>
#include  <gs_specific.h>

/*--------------- maintaining list of known windows ------------------------ */

private  Gwindow        *windows = NULL;
private  int            n_windows = 0;

private  Gwindow  create_window_struct( void )
{
    Gwindow     new_window;

    ALLOC( new_window, 1 );

    ADD_ELEMENT_TO_ARRAY( windows, n_windows, new_window, DEFAULT_CHUNK_SIZE );

    return( new_window );
}

private  int  lookup_window_index( Gwindow  window )
{
    int  i;

    for_less( i, 0, n_windows )
    {
        if( windows[i] == window )
            break;
    }

    if( i >= n_windows )
        i = -1;

    return( i );
}

public  int  get_n_graphics_windows( void )
{
    return( n_windows );
}

public  Gwindow  get_nth_graphics_window(
    int   i )
{
    return( windows[i] );
}

private  void  delete_window_struct( Gwindow   window )
{
    int            ind;

    ind = lookup_window_index( window );

    if( ind >= 0 )
    {
        DELETE_ELEMENT_FROM_ARRAY( windows, n_windows, ind,
                                   DEFAULT_CHUNK_SIZE );
    }

    FREE( window );
}

static  Gwindow   current_window = NULL;

public  void  set_current_window( Gwindow   window )
{
    if( window != current_window )
    {
        if( window != (Gwindow) 0 )
            WS_set_current_window( window->WS_window,
                                   window->current_bitplanes );
        current_window = window;
    }
}

public  Gwindow  get_current_window( void )
{
    return( current_window );
}

/*---------------------- window configurations ------------------------ */

private  void  check_graphics_initialized( void )
{
    static  BOOLEAN  first = TRUE;

    if( first )
    {
        first = FALSE;
        GS_initialize();
    }
}

#define  DEFAULT_N_CURVE_SEGMENTS  8

private  void  initialize_window(
    Gwindow   window )
{
    Colour     default_background_col;

    default_background_col = BLACK;

    set_current_window( window );

#ifdef DEBUG
    print_info();
#endif

    WS_get_window_size( window->WS_window, &window->x_size, &window->y_size );
    WS_get_window_position( window->WS_window, &window->x_origin,
                                               &window->y_origin );

    G_set_automatic_clear_state( window, TRUE );

    G_set_background_colour( window, default_background_col );

    window->current_bitplanes = NORMAL_PLANES;
    window->bitplanes_cleared[NORMAL_PLANES] = FALSE;
    window->bitplanes_cleared[OVERLAY_PLANES] = FALSE;

    GS_set_matrix_mode( VIEWING_MATRIX );

#ifndef  TWO_D_ONLY
    G_set_shaded_state( window, ON );
    G_set_shading_type( window, GOURAUD_SHADING );
    G_set_lighting_state( window, OFF );
    G_set_transparency_state( window, ON );
    G_backface_culling_state( window, OFF );
    G_set_render_lines_as_curves_state( window, OFF );
    G_set_n_curve_segments( window, DEFAULT_N_CURVE_SEGMENTS );
    G_set_markers_labels_visibility( window, ON );

    GS_set_depth_function( LESS_OR_EQUAL );

    if( window->zbuffer_state )
        GS_set_depth_buffer_state( window->zbuffer_state );

    GS_initialize_surface_property( window );

    initialize_graphics_lights( window );

    initialize_display_interrupts( window );
#endif

    initialize_window_view( window );

    add_fonts_for_window( window );

    G_clear_window( window );
    G_update_window( window );

    G_clear_window( window );
    G_update_window( window );
}

public  long  G_get_window_id(
    Gwindow   window )
{
    return( (long) GS_get_window_id( window ) );
}

public  Status  G_create_window(
    STRING         title,
    int            x_pos,
    int            y_pos,
    int            width,
    int            height,
    BOOLEAN        colour_map_desired,
    BOOLEAN        double_buffer_desired,
    BOOLEAN        depth_buffer_desired,
    int            n_overlay_planes_desired,
    Gwindow        *window )
{
    Status   status;
    BOOLEAN  actual_colour_map_flag;
    BOOLEAN  actual_double_buffer_flag;
    BOOLEAN  actual_depth_buffer_flag;
    int      actual_n_overlay_planes;

    check_graphics_initialized();

    *window = create_window_struct();

    ALLOC( (*window)->GS_window, 1 );
    ALLOC( (*window)->WS_window, 1 );

    status = GS_create_window( *window, title, x_pos, y_pos, width, height,
                               colour_map_desired,
                               double_buffer_desired,
                               depth_buffer_desired,
                               n_overlay_planes_desired,
                               &actual_colour_map_flag,
                               &actual_double_buffer_flag,
                               &actual_depth_buffer_flag,
                               &actual_n_overlay_planes );

    if( status != OK )
    {
        print_error( "G_create_window():  cannot open a graphics window.\n" );
    }
    else
    {
        (*window)->double_buffer_available = actual_double_buffer_flag;
        (*window)->double_buffer_state = actual_double_buffer_flag;

        (*window)->colour_map_state = actual_colour_map_flag;
#ifndef  TWO_D_ONLY
        (*window)->n_overlay_planes = actual_n_overlay_planes;
        (*window)->zbuffer_state = actual_depth_buffer_flag;
        (*window)->zbuffer_available = actual_depth_buffer_flag;
#endif

        initialize_window( *window );
    }

    return( status );
}

public  Status  G_delete_window(
    Gwindow   window )
{
    Status    status;

    set_current_window( window );

    delete_fonts_for_window( window );

    status = GS_delete_window( window );

    set_current_window( (Gwindow) 0 );

    FREE( window->GS_window );
    FREE( window->WS_window );

    delete_window_struct( window );

    return( status );
}

public  void  G_set_window_title(
    Gwindow   window,
    STRING    title )
{
    set_current_window( window );

    GS_set_window_title( window, title );
}

public  void  G_terminate( void )
{
    delete_event_queue();
    delete_fonts();

    while( n_windows > 0 )
    {
        (void) G_delete_window( windows[0] );
    }

    GS_terminate();
}

public  BOOLEAN  G_is_double_buffer_supported( void )
{
    static  BOOLEAN   first = TRUE;
    static  BOOLEAN   available;

    if( first )
    {
        available = GS_is_double_buffer_supported();

        first = FALSE;
    }

    return( available );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_get_double_buffer_state
@INPUT      : window
@OUTPUT     :
@RETURNS    : TRUE or FALSE
@DESCRIPTION: Returns true if the window is in double buffered mode.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

public  BOOLEAN  G_get_double_buffer_state(
    Gwindow        window )
{
    return( window->double_buffer_state );
}

public  void  G_set_double_buffer_state(
    Gwindow        window,
    BOOLEAN        flag )
{
    if( flag && (!G_can_switch_double_buffering() ||
                 !window->double_buffer_available) )
        flag = FALSE;

    if( flag != window->double_buffer_state )
    {
        set_current_window( window );

        set_bitplanes( window, NORMAL_PLANES );

        window->double_buffer_state = GS_set_double_buffer_state( flag );

        restore_bitplanes( window );
    }
}

public  BOOLEAN  G_is_depth_buffer_supported( void )
{
    static  BOOLEAN   first = TRUE;
    static  BOOLEAN   available;

    if( first )
    {
        available = GS_is_depth_buffer_supported();

        first = FALSE;
    }

    return( available );
}

public  void  G_set_zbuffer_state(
    Gwindow         window,
    BOOLEAN         flag )
{
#ifndef  TWO_D_ONLY
    if( flag && (!G_is_depth_buffer_supported() || !window->zbuffer_available) )
        flag = FALSE;

    if( flag != window->zbuffer_state )
    {
        set_current_window( window );

        GS_set_depth_buffer_state( flag );

        window->zbuffer_state = flag;
    }
#endif
}

public  BOOLEAN  G_get_zbuffer_state(
    Gwindow         window )
{
#ifndef  TWO_D_ONLY
    return( window->zbuffer_state );
#else
    return( FALSE );
#endif
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_get_colour_map_state
@INPUT      : window
@OUTPUT     :
@RETURNS    : TRUE or FALSE
@DESCRIPTION: Returns true if window is in colour map mode.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

public  BOOLEAN  G_get_colour_map_state(
    Gwindow        window )
{
    return( window->colour_map_state );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_set_colour_map_state
@INPUT      : window
              flat
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Sets the colour map state of the window.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

public  void  G_set_colour_map_state(
    Gwindow        window,
    BOOLEAN        flag )
{
    if( G_can_switch_colour_map_mode() && flag != window->colour_map_state )
    {
        set_current_window( window );

        set_bitplanes( window, NORMAL_PLANES );

        GS_set_colour_map_state( flag );

        restore_bitplanes( window );

        window->colour_map_state = flag;
    }
}

public  int  G_get_n_colour_map_entries(
    Gwindow        window )
{
    set_current_window( window );

    return( GS_get_n_colour_map_entries( window ) );
}

public  void  G_set_colour_map_entry(
    Gwindow         window,
    int             ind,
    Colour          colour )
{
    set_current_window( window );

    if( window->current_bitplanes != NORMAL_PLANES )
        set_bitplanes( window, NORMAL_PLANES );

    GS_set_colour_map_entry( window, ind, colour );

    if( window->current_bitplanes != NORMAL_PLANES )
        restore_bitplanes( window );
}

public  void  restore_bitplanes(
    Gwindow    window )
{
#ifndef  TWO_D_ONLY
    if( window == (Gwindow) NULL )
        window = current_window;

    if( window != (Gwindow) NULL )
        set_bitplanes( window, window->current_bitplanes );
#endif
}

public  int  G_get_monitor_width( void )
{
    return( GS_get_monitor_width() );
}

public  int  G_get_monitor_height( void )
{
    return( GS_get_monitor_height() );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_get_monitor_widths_to_eye
@INPUT      :
@OUTPUT     :
@RETURNS    : ratio of monitor width to the eye distance
@DESCRIPTION: Returns the ratio of the monitor width to the distance to the
              users eye.  Used to help define reasonable 3D views.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

public  Real  G_get_monitor_widths_to_eye( void )
{
    return( 1.2 );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_get_window_size
@INPUT      : window
@OUTPUT     : x_size
              y_size
@RETURNS    :
@DESCRIPTION: Passes back the size of the window in pixels.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

public  void  G_get_window_size(
    Gwindow        window,
    int            *x_size,
    int            *y_size )
{
    *x_size = window->x_size;
    *y_size = window->y_size;
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_get_window_position
@INPUT      : window
@OUTPUT     : x_pos
              y_pos
@RETURNS    : 
@DESCRIPTION: Passes back the position of the window in pixels from bottom left.@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

public  void  G_get_window_position(
    Gwindow        window,
    int            *x_pos,
    int            *y_pos )
{
    *x_pos = (int) window->x_origin;
    *y_pos = (int) window->y_origin;
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_set_background_colour
@INPUT      : window
              colour
@OUTPUT     : 
@RETURNS    :
@DESCRIPTION: Sets the background colour of the window.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

public  void  G_set_background_colour(
    Gwindow         window,
    Colour          colour )
{
    if( window->colour_map_state )
        window->background_colour_index = colour;
    else
        window->background_colour = colour;
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_get_background_colour
@INPUT      : window
@OUTPUT     :
@RETURNS    : colour
@DESCRIPTION: Returns the background colour of the window.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : Jan. 24, 1995         David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

public  Colour  G_get_background_colour(
    Gwindow         window )
{
    if( window->colour_map_state )
        return( window->background_colour_index );
    else
        return( window->background_colour );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_clear_window
@INPUT      : window
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Clears the entire window to the background colour.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

public  void  G_clear_window(
    Gwindow    window )
{
    Colour    colour;

    set_current_window( window );

    if( G_get_colour_map_state(window) )
        colour = window->background_colour_index;
    else
        colour = window->background_colour;

    GS_clear_window( window, colour );

    window->bitplanes_cleared[window->current_bitplanes] = TRUE;
}

public  void  G_clear_viewport(
    Gwindow    window,
    Colour     colour )
{
    set_current_window( window );

    GS_clear_viewport( window, colour );
}

public  void  G_set_automatic_clear_state(
    Gwindow    window,
    BOOLEAN    state )
{
    window->automatic_clear_flag = state;
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_update_window
@INPUT      : window
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Called at the end of drawing a frame, this basically does just
              a swap front and back buffers.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

public  void  G_update_window( Gwindow   window )
{
    set_current_window( window );

    check_window_cleared( window );

    if( window->current_bitplanes == NORMAL_PLANES )
    {
        if( window->double_buffer_state )
            WS_swap_buffers( window->WS_window );
        GS_flush();
    }
#ifndef  TWO_D_ONLY
    else
        GS_flush();

    set_continuation_flag( window, FALSE );
#endif

    window->bitplanes_cleared[window->current_bitplanes] = FALSE;
}

/* ARGSUSED */

public  void  check_window_cleared(
     Gwindow    window  )
{
    if( window->automatic_clear_flag &&
        !window->bitplanes_cleared[window->current_bitplanes] )
    {
        G_clear_window( window );
    }
}

public  BOOLEAN  G_has_overlay_planes( void )
{
#ifdef  TWO_D_ONLY
    return( FALSE );
#else
    return( G_get_n_overlay_planes() > 0 );
#endif
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_set_bitplanes
@INPUT      : window
              bitplanes
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Sets the current bitplanes to OVERLAY_PLANES, NORMAL_PLANES, etc.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

public  void  G_set_bitplanes(
    Gwindow          window,
    Bitplane_types   bitplanes )
{
    if( window->current_bitplanes != bitplanes )
    {
        set_bitplanes( window, bitplanes );
        window->current_bitplanes = bitplanes;
    }
}

public  BOOLEAN  G_can_switch_double_buffering( void )
{
    return( GS_can_switch_double_buffering() );
}

public  BOOLEAN  G_can_switch_colour_map_mode( void )
{
    return( GS_can_switch_colour_map_mode() );
}

public  BOOLEAN  G_has_transparency_mode( void )
{
    return( GS_has_transparency_mode() );
}

public  BOOLEAN  G_has_rgb_mode( void )
{
    return( GS_has_transparency_mode() );
}

public  int  G_get_n_overlay_planes( void )
{
    return( GS_get_num_overlay_planes() );
}

public  void  G_set_overlay_colour_map(
    Gwindow         window,
    int             ind,
    Colour          colour )
{
    set_current_window( window );

    GS_set_overlay_colour_map( window, ind, colour );
}

public  void  G_append_to_last_update(
     Gwindow   window )
{
    set_current_window( window );
    GS_append_to_last_update( window );
    set_continuation_flag( window, TRUE );
    window->bitplanes_cleared[NORMAL_PLANES] = TRUE;
}

public  void  G_continue_last_update(
     Gwindow   window )
{
    set_current_window( window );
    set_continuation_flag( window, TRUE );
}
