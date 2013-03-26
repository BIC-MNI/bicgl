#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H  

#include  <volume_io.h>
#include  <GS_graphics.h>

  void  GS_set_shade_model(
    Shading_types  type )
{
    if( type == GOURAUD_SHADING )
        glShadeModel( GL_SMOOTH );
    else
        glShadeModel( GL_FLAT );
}

  void  GS_turn_off_blend_function( void )
{
    glDisable( GL_BLEND );
    glBlendFunc( GL_ONE, GL_ZERO );
}

  void  GS_turn_on_blend_function( void )
{
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
}

/* ARGSUSED */

  void  GS_set_lighting_state(
    GSwindow     window,
    VIO_BOOL      state )
{
    if( state )
        glEnable( GL_LIGHTING );
    else
        glDisable( GL_LIGHTING );
}

  void  GS_backface_culling_state(
    VIO_BOOL     state )
{
    if( state )
    {
        glEnable( GL_CULL_FACE );
        glCullFace( GL_BACK );
    }
    else
        glDisable( GL_CULL_FACE );
}

/* ARGSUSED */

  void  GS_set_n_curve_segments(
    int      n_segments )
{
#ifdef  TO_DO
    curveprecision( n_segments );
#endif
}
