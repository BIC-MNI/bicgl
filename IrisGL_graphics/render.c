 
#include  <internal_volume_io.h>
#include  <graphics.h>

public  void  GS_set_shade_model(
    Shading_types  type )
{
#ifndef  TWO_D_ONLY
    if( type == GOURAUD_SHADING )
        shademodel( GOURAUD );
    else
        shademodel( FLAT );
#endif
}

public  void  GS_turn_off_blend_function()
{
    blendfunction( BF_ONE, BF_ZERO );
}

public  void  GS_turn_on_blend_function()
{
    blendfunction( BF_SA, BF_MSA );
}

public  void  GS_set_lighting_state(
    Gwindow      window,
    BOOLEAN      state )
{
#ifndef  TWO_D_ONLY
    if( state )
        lmbind( MATERIAL, window->GS_window.unique_lmdef_id );
    else
        lmbind( MATERIAL, 0 );
#endif
}

public  void  GS_backface_culling_state(
    BOOLEAN     state )
{
#ifndef  TWO_D_ONLY
    backface( state );
#endif
}

public  void  GS_set_n_curve_segments(
    int      n_segments )
{
#ifndef  TWO_D_ONLY
    curveprecision( n_segments );
#endif
}
