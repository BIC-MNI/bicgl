 
#include  <internal_volume_io.h>
#include  <graphics.h>

typedef  double  Matrix[16];

private  void  make_matrix(
    Transform  *trans,
    Matrix  gl_trans )
{
    int      i, j;

    for_less( i, 0, 4 )
    {
        for_less( j, 0, 4 )
            gl_trans[IJ(j,i,4)] = Transform_elem( *trans, i, j );
    }
}

public  void  GS_load_transform(
    Transform  *transform )
{
    Matrix  gl_trans;

    make_matrix( transform, gl_trans );

    glLoadMatrixd( gl_trans );
}

public  void  GS_mult_transform(
    Transform  *transform )
{
    Matrix  gl_trans;

    make_matrix( transform, gl_trans );

    glMultMatrixd( gl_trans );
}

public  void  GS_get_transform(
    Transform  *trans )
{
    int         i, j, type;
    Matrix   gl_trans;

    glGetIntegerv( GL_MATRIX_MODE, &type );

    if( type == GL_MODELVIEW )
        glGetDoublev( GL_MODELVIEW_MATRIX, gl_trans );
    else if( type == GL_PROJECTION )
        glGetDoublev( GL_PROJECTION_MATRIX, gl_trans );
    else
        handle_internal_error( "GS_get_transform" );

    for_less( i, 0, 4 )
    {
        for_less( j, 0, 4 )
            Transform_elem( *trans, i, j ) = gl_trans[IJ(j,i,4)];
    }
}

public  void  GS_ortho(
    Real   x_min,
    Real   x_max,
    Real   y_min,
    Real   y_max,
    Real   z_min,
    Real   z_max )
{
    glLoadIdentity();
    glOrtho( x_min, x_max, y_min, y_max, z_min, z_max );
}

public  void  GS_frustum(
    Real   x_min,
    Real   x_max,
    Real   y_min,
    Real   y_max,
    Real   z_min,
    Real   z_max )
{
    glLoadIdentity();
    glFrustum( x_min, x_max, y_min, y_max, z_min, z_max );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : initialize_window_view
@INPUT      : window
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Initializes the window view, called on creation of the window.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  void  GS_initialize_window_view(
    Gwindow   window )
{
}

public  void  GS_push_transform()
{
    glPushMatrix();
}

public  void  GS_pop_transform()
{
    glPopMatrix();
}

public  void  GS_set_viewport(
    int            x_min,
    int            x_max,
    int            y_min,
    int            y_max )
{
    glViewport( x_min, y_min, x_max - x_min + 1, y_max - y_min + 1 );
}

public  void  clear_overlay_planes()
{
#ifndef  TWO_D_ONLY
#ifdef TO_DO
    Matrix          save_projection;

    if( G_has_overlay_planes() )
    {
        mmode( MPROJECTION );
        getmatrix( save_projection );
        mmode( MVIEWING );

        pushmatrix();
        pushviewport();

        fullscrn();
/*
        drawmode( NORMALDRAW );
        blendfunction( BF_ONE, BF_ZERO );
*/
        drawmode( OVERDRAW );
        color( 0 );
        clear();
        endfullscrn();

        restore_bitplanes( (Gwindow) NULL );

        popviewport();
        popmatrix();

        mmode( MPROJECTION );
        loadmatrix( save_projection );
        mmode( MVIEWING );
    }
#endif
#endif
}

public  void  GS_set_matrix_mode(
    Matrix_modes   mode )
{
    glMatrixMode( mode );
}

