 
#include  <internal_volume_io.h>
#include  <gs_specific.h>

public  void  G_set_shaded_state(
    Gwindow      window,
    BOOLEAN      state )
{
    if( state != window->shaded_mode_state )
        window->shaded_mode_state = state;
}

public  void  G_set_shading_type(
    Gwindow        window,
    Shading_types  type )
{
#ifndef  TWO_D_ONLY
    if( type != window->shading_type )
    {
        set_current_window( window );

        set_bitplanes( window, NORMAL_PLANES );

        GS_set_shade_model( type );

        restore_bitplanes( window );
            
        window->shading_type = type;
    }
#endif
}

public  void  update_blend_function(
    Gwindow         window,
    Bitplane_types  bitplane )
{
    set_current_window( window );

    if( bitplane == OVERLAY_PLANES || !window->transparency_state )
        GS_turn_off_blend_function();
    else
        GS_turn_on_blend_function();
}

public  void  G_set_transparency_state(
    Gwindow        window,
    BOOLEAN        state )
{
    if( state != window->transparency_state )
    {
        window->transparency_state = state;
        update_blend_function( window, window->current_bitplanes );
    }
}

public  BOOLEAN  G_get_lighting_state(
    Gwindow      window )
{
    return( window->lighting_state );
}

public  void  G_set_lighting_state(
    Gwindow      window,
    BOOLEAN      state )
{
    if( state != window->lighting_state )
    {
        set_current_window( window );

        GS_set_lighting_state( window, state );
            
        window->lighting_state = state;
    }
}

public  void  G_backface_culling_state(
    Gwindow     window,
    BOOLEAN     state )
{
    if( state != window->backface_culling_state )
    {
        set_current_window( window );

        GS_backface_culling_state( state );

        window->backface_culling_state = state;
    }
}

public  void  G_set_render_lines_as_curves_state(
    Gwindow     window,
    BOOLEAN     state )
{
#ifndef  TWO_D_ONLY
    window->render_lines_as_curves_state = state;
#endif
}

public  void  G_set_n_curve_segments(
    Gwindow  window,
    int      n_segments )
{
#ifndef  TWO_D_ONLY
    if( n_segments != window->n_curve_segments && n_segments > 0 )
    {
        set_current_window( window );

        GS_set_n_curve_segments( n_segments );

        window->n_curve_segments = n_segments;
    }
#endif
}

public  void  G_set_markers_labels_visibility(
    Gwindow    window,
    BOOLEAN    state )
{
#ifndef  TWO_D_ONLY
    window->marker_labels_visibility = state;
#endif
}
