#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H  

#include  <volume_io.h>
#include  <GS_graphics.h>

static  void  initialize_window(
    GSwindow   window );

  Window_id  GS_get_window_id(
    GSwindow  window )
{
    return( WS_get_window_id(&window->WS_window) );
}

  void  GS_set_current_window(
    GSwindow        window )
{
    WS_set_current_window( &window->WS_window );
}

  void  GS_initialize( void )
{
#ifdef TO_DO
    static  Matrix   catmull_rom_matrix = {
                                              { -0.5,  1.5, -1.5,  0.5 },
                                              {  1.0, -2.5,  2.0, -0.5 },
                                              { -0.5,  0.0,  0.5,  0.0 },
                                              {  0.0,  1.0,  0.0,  0.0 },
                                          };
#endif

    WS_initialize();

#ifdef TO_DO
    defbasis( CATMULL_ROM_ID, catmull_rom_matrix );
    curvebasis( CATMULL_ROM_ID );
#endif
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_terminate
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Terminates the use of graphics, freeing up memory.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

  void  GS_terminate( void )
{
}

  VIO_BOOL  GS_can_switch_double_buffering( void )
{
    return( TRUE );
}

  VIO_BOOL  GS_can_switch_colour_map_mode( void )
{
    return( TRUE );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_create_window
@INPUT      : title
              x_pos
              y_pos
              width
              height
@OUTPUT     : window
@RETURNS    : 
@DESCRIPTION: Creates a window of the specified size and shape.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

/* ARGSUSED */

  VIO_Status  GS_create_window(
    GSwindow       window,
    VIO_STR         title,
    int            x_pos,
    int            y_pos,
    int            width,
    int            height,
    VIO_BOOL        colour_map_desired,
    VIO_BOOL        double_buffer_desired,
    VIO_BOOL        depth_buffer_desired,
    int            n_overlay_planes_desired,
    VIO_BOOL        *actual_colour_map_flag,
    VIO_BOOL        *actual_double_buffer_flag,
    VIO_BOOL        *actual_depth_buffer_flag,
    int            *actual_n_overlay_planes )
{
    VIO_Status       status;

    status = WS_create_window( title, x_pos, y_pos, width, height,
                               colour_map_desired, double_buffer_desired,
                               depth_buffer_desired,
                               0, actual_colour_map_flag,
                               actual_double_buffer_flag,
                               actual_depth_buffer_flag,
                               actual_n_overlay_planes, &window->WS_window);


    if( status == VIO_OK )
        initialize_window( window );

    return( status );
}

#ifdef DEBUG
  void  print_info( void )
{
    int     i, j;
    double  list[4];
    double  trans[16];

    glGetDoublev( GL_VIEWPORT, list );

    print( "Viewport: %g %g %g %g\n", list[0], list[1], list[2], list[3] );
    glGetDoublev( GL_MODELVIEW_MATRIX, trans );
    print( "Modelview\n" );
    for_less( i, 0, 4 )
    {
        for_less( j, 0, 4 )
            print( " %15.10g", trans[IJ(i,j,4)] );
        print( "\n" );
    }

    glGetDoublev( GL_PROJECTION_MATRIX, trans );
    print( "Projection\n" );
    for_less( i, 0, 4 )
    {
        for_less( j, 0, 4 )
            print( " %15.10g", trans[IJ(i,j,4)] );
        print( "\n" );
    }
}
#endif

/* ----------------------------- MNI Header -----------------------------------
@NAME       : initialize_window
@INPUT      : window
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Initializes the window, called when the window is created.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

/* ARGSUSED */

static  void  initialize_window(
    GSwindow   window )
{
}

  void  GS_set_window_title(
    GSwindow   window,
    VIO_STR     title )
{
    WS_set_window_title( &window->WS_window, title );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_has_transparency_mode
@INPUT      : 
@OUTPUT     : 
@RETURNS    : TRUE or FALSE
@DESCRIPTION: Returns true if transparency mode is supported.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

  VIO_BOOL  GS_has_transparency_mode( void )
{
    GLint   blend_flag;

    glGetIntegerv( GL_BLEND, &blend_flag );

    return( blend_flag > 0 );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_has_rgb_mode
@INPUT      : 
@OUTPUT     : 
@RETURNS    : TRUE or FALSE
@DESCRIPTION: Returns true if rgb mode is supported.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

  VIO_BOOL  GS_has_rgb_mode( void )
{
    GLint  r_bits, g_bits, b_bits;

    glGetIntegerv( GL_RED_BITS,   &r_bits );
    glGetIntegerv( GL_GREEN_BITS, &g_bits );
    glGetIntegerv( GL_BLUE_BITS,  &b_bits );
    return( r_bits > 0 && g_bits > 0 && b_bits > 0 );
}

static  void  reinitialize_after_switching_configuration(
    GSwindow       window )
{
    redefine_lights( window );
}

/* ARGSUSED */

  VIO_BOOL  GS_set_double_buffer_state(
    GSwindow       window,
    VIO_BOOL        flag )
{
    VIO_BOOL   state;

    state = WS_set_double_buffer_state( &window->WS_window, flag );

    reinitialize_after_switching_configuration( window );

    return( state );
}

/* ARGSUSED */

  VIO_BOOL  GS_set_colour_map_state(
    GSwindow       window,
    VIO_BOOL        flag )
{
    VIO_BOOL  state;

    state = WS_set_colour_map_state( &window->WS_window, flag );

    reinitialize_after_switching_configuration( window );

    return( state );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_get_n_colour_map_entries
@INPUT      : window
@OUTPUT     : 
@RETURNS    : n colours
@DESCRIPTION: Returns the number of colour map entries for the window, based
              on its current double buffer state.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

/* ARGSUSED */

  int  GS_get_n_colour_map_entries(
    GSwindow  window,
    VIO_BOOL   double_buffer_state )
{
    GLint  n_bits;
    int    n_colours;

    glGetIntegerv( GL_INDEX_BITS, &n_bits );

    n_colours = (1 << n_bits);

    if( n_colours == 4096 )   /* from man page on mapcolor, should not use */
        n_colours -= 256;     /* upper 256 colours on G systems            */

    return( n_colours );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_set_colour_map_entry
@INPUT      : window
              ind
              colour
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Sets the colour of the index'th colour map entry.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

  void  GS_set_colour_map_entry(
    GSwindow        window,
    Bitplane_types  bitplanes,
    int             ind,
    VIO_Colour          colour )
{
    WS_set_colour_map_entry( &window->WS_window, bitplanes, ind, colour );
}

  VIO_BOOL  GS_is_double_buffer_supported( void )
{
    int   available;

    glGetIntegerv( GL_DOUBLEBUFFER, &available );

    return( TRUE );
/*
    return( available );
*/
}

  VIO_BOOL  GS_is_depth_buffer_supported( void )
{
/*
    VIO_BOOL   available;
*/
    int       n_bits;

    glGetIntegerv( GL_DEPTH_BITS, &n_bits );

    return( TRUE );
/*
    available = (n_bits > 0);
    return( available );
*/
}

  void  GS_set_depth_function(
    Depth_functions  func )
{
    int   gl_depth_func;

    switch( func )
    {
    case LESS_OR_EQUAL:
        gl_depth_func = GL_LEQUAL;
        break;
    default:
        gl_depth_func = GL_LEQUAL;
        break;
    }

    glDepthFunc( (GLenum) gl_depth_func );
}

  void  GS_set_depth_buffer_state(
    VIO_BOOL         flag )
{
    if( flag )
        glEnable( GL_DEPTH_TEST );
    else
        glDisable( GL_DEPTH_TEST );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_delete_window
@INPUT      : window
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Deletes the graphics window, and frees up any associated memory.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

  VIO_Status  GS_delete_window(
    GSwindow   window )
{
    delete_lights( window );

    WS_delete_window( &window->WS_window );

    return( VIO_OK );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_get_monitor_width
@INPUT      : 
@OUTPUT     : 
@RETURNS    : width of the monitor in pixels
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

  int  GS_get_monitor_width( void )
{
    int  x_size, y_size;

    WS_get_screen_size( &x_size, &y_size );

    return( x_size );
}

  int  GS_get_monitor_height( void )
{
    int  x_size, y_size;

    WS_get_screen_size( &x_size, &y_size );

    return( y_size );
}

  void  GS_get_window_size(
    int        *x_size,
    int        *y_size )
{
    WS_get_window_size( x_size, y_size );
}

  void  GS_get_window_position(
    int        *x_position,
    int        *y_position )
{
    WS_get_window_position( x_position, y_position );
}

  void  GS_clear_depth_buffer( void )
{
    glClear( GL_DEPTH_BUFFER_BIT );
}

static  void  clear_viewport(
     GSwindow        window,
     Bitplane_types  current_bitplanes,
     VIO_BOOL         colour_map_state,
     VIO_BOOL         zbuffer_state,
     VIO_Colour          colour )
{
    if( current_bitplanes == NORMAL_PLANES )
    {
        if( colour_map_state )
        {
            glClearIndex( (GLfloat) colour );
        }
        else
        {
            glClearColor( (GLclampf) get_Colour_r_0_1(colour),
                          (GLclampf) get_Colour_g_0_1(colour),
                          (GLclampf) get_Colour_b_0_1(colour),
                          (GLclampf) 1.0 );
        }

        glClear( GL_COLOR_BUFFER_BIT );

        if( zbuffer_state )
            glClear( GL_DEPTH_BUFFER_BIT );
    }
    else
         GS_clear_overlay();
}

  void  GS_clear_overlay( void )
{
    glClearIndex( 0.0f );
    glClear( GL_COLOR_BUFFER_BIT );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_clear_window
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

  void  GS_clear_window(
    GSwindow        window,
    int             x_size,
    int             y_size,
    Bitplane_types  current_bitplanes,
    VIO_BOOL         colour_map_state,
    VIO_BOOL         zbuffer_state,
    VIO_Colour          colour )
{
    clear_viewport( window, current_bitplanes, colour_map_state,
                    zbuffer_state, colour );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_clear_viewport
@INPUT      : window
              colour
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Clears the current viewport to the given colour.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

  void  GS_clear_viewport(
    GSwindow        window,
    int             x_viewport_min,
    int             x_viewport_max,
    int             y_viewport_min,
    int             y_viewport_max,
    int             x_size,
    int             y_size,
    Bitplane_types  current_bitplanes,
    VIO_BOOL         colour_map_state,
    VIO_BOOL         zbuffer_state,
    VIO_Colour          colour )
{
    VIO_BOOL  scissoring;

    if( x_viewport_min != 0 ||
        x_viewport_max != x_size-1 ||
        y_viewport_min != 0 ||
        y_viewport_max != y_size-1 )
    {
        glScissor( x_viewport_min, y_viewport_min,
                   x_viewport_max - x_viewport_min + 1,
                   y_viewport_max - y_viewport_min + 1 );
        glEnable( GL_SCISSOR_TEST );
        scissoring = TRUE;
    }
    else
        scissoring = FALSE;

    clear_viewport( window, current_bitplanes, colour_map_state,
                    zbuffer_state, colour );

    if( scissoring )
        glDisable( GL_SCISSOR_TEST );
}

  void  GS_flush( void )
{
    glFlush();
}

  void  GS_append_to_last_update(
    GSwindow   window,
    VIO_BOOL    zbuffer_state,
    int        x_size,
    int        y_size )
{
    glReadBuffer( GL_FRONT );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    GS_ortho_2d( 0, x_size - 1, 0, y_size - 1 );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    glRasterPos2i( 0, 0 );
    glCopyPixels( 0, 0, x_size, y_size, GL_COLOR );

    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
}

  int  GS_get_num_overlay_planes( void )
{
    return( WS_get_n_overlay_planes() );
}

  void  GS_set_bitplanes( 
    GSwindow         window,
    Bitplane_types   bitplanes )
{
    WS_set_bitplanes( &window->WS_window, bitplanes );

    switch( bitplanes )
    {
    case NORMAL_PLANES:
        glDepthMask( (GLboolean) TRUE );
        break;

    case OVERLAY_PLANES:
        glDepthMask( (GLboolean) FALSE );
        break;
    }
}

  void  GS_set_overlay_colour_map(
    GSwindow         window,
    int              ind,
    VIO_Colour           colour )
{
    WS_set_overlay_colour_map_entry( &window->WS_window, ind, colour );
}

  void  GS_swap_buffers( void )
{
    WS_swap_buffers();
}
