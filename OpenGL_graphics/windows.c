 
#include  <internal_volume_io.h>
#include  <graphics.h>

private  void  initialize_window(
    Gwindow   window );

public  Window_id  GS_get_window_id(
    Gwindow  window )
{
    return( window->WS_window.x_window.window_id );
}

public  void  GS_initialize( void )
{
#ifdef TO_DO
    static  Matrix   catmull_rom_matrix = {
                                              { -0.5,  1.5, -1.5,  0.5 },
                                              {  1.0, -2.5,  2.0, -0.5 },
                                              { -0.5,  0.0,  0.5,  0.0 },
                                              {  0.0,  1.0,  0.0,  0.0 },
                                          };
#endif

#ifndef  TWO_D_ONLY
#ifdef TO_DO
    defbasis( CATMULL_ROM_ID, catmull_rom_matrix );
    curvebasis( CATMULL_ROM_ID );
#endif
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

public  void  GS_terminate( void )
{
}

public  BOOLEAN  GS_can_switch_double_buffering()
{
    return( FALSE );
}

public  BOOLEAN  GS_can_switch_colour_map_mode()
{
    return( FALSE );
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

public  Status  GS_create_window(
    Gwindow        window,
    char           title[],
    int            x_pos,
    int            y_pos,
    int            width,
    int            height,
    BOOLEAN        colour_map_desired,
    BOOLEAN        double_buffer_desired,
    BOOLEAN        depth_buffer_desired,
    int            n_overlay_planes_desired,
    BOOLEAN        *actual_colour_map_flag,
    BOOLEAN        *actual_double_buffer_flag,
    BOOLEAN        *actual_depth_buffer_flag,
    int            *actual_n_overlay_planes )
{
    Status       status;

    status = WS_create_window( title, x_pos, y_pos, width, height,
                               colour_map_desired, double_buffer_desired,
                               depth_buffer_desired, 0,
                               actual_colour_map_flag,
                               actual_double_buffer_flag,
                               actual_depth_buffer_flag,
                               actual_n_overlay_planes, &window->WS_window);

    if( status == OK )
        initialize_window( window );

    return( status );
}

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

private  void  initialize_window(
    Gwindow   window )   /* ARGSUSED */
{
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

public  BOOLEAN  GS_has_transparency_mode()
{
    return( TRUE );
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

public  BOOLEAN  GS_has_rgb_mode()
{
    GLint  r_bits, g_bits, b_bits;

    glGetIntegerv( GL_RED_BITS,   &r_bits );
    glGetIntegerv( GL_GREEN_BITS, &g_bits );
    glGetIntegerv( GL_BLUE_BITS,  &b_bits );
    return( r_bits > 0 && g_bits > 0 && b_bits > 0 );
}

public  BOOLEAN  GS_set_double_buffer_state(
    BOOLEAN        flag )
{
    print( "GS_set_double_buffer_state():  OpenGL cannot change state.\n" );
    return( FALSE );
}

public  void  GS_set_colour_map_state(
    BOOLEAN        flag )
{
    print( "GS_set_colour_map_state():  OpenGL cannot change state.\n" );
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

public  int  GS_get_n_colour_map_entries(
    Gwindow  window  ) /* ARGSUSED */
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
              index
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

public  void  GS_set_colour_map_entry(
    Gwindow         window,
    int             index,
    Colour          colour )
{
    WS_set_colour_map_entry( &window->WS_window, index, colour );
}

public  BOOLEAN  GS_is_double_buffer_supported( void )
{
    int   available;

    glGetIntegerv( GL_DOUBLEBUFFER, &available );

    return( TRUE || available );
}

public  BOOLEAN  GS_is_depth_buffer_supported( void )
{
#ifdef  TWO_D_ONLY
    return( FALSE );
#else
    BOOLEAN   available;
    int       n_bits;

    glGetIntegerv( GL_DEPTH_BITS, &n_bits );
    available = (n_bits > 0);

    return( TRUE || available );
#endif
}

public  void  GS_set_depth_function(
    Depth_functions  func )
{
    glDepthFunc( func );
}

public  void  GS_set_depth_buffer_state(
    BOOLEAN         flag )
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

public  Status  GS_delete_window(
    Gwindow   window )
{
    Status    status;

    if( window->WS_window.x_window.window_id >= 0 )
    {
#ifndef  TWO_D_ONLY
#ifdef TO_DO
        if( G_has_overlay_planes() )
        {
            viewport( 0, window->x_size-1, 0, window->y_size-1 );
            drawmode( OVERDRAW );
            color( 0 );
            clear();
        }
#endif
#endif

        WS_delete_window( &window->WS_window );

        status = OK;
    }
    else
    {
        print( "Error:  tried to delete invalid window.\n" );
        status = ERROR;
    }

    return( status );
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

public  int  GS_get_monitor_width( void )
{
    GLint  width;

    width = 1000;

    print( "GS_get_monitor_width(): not implemented\n" );

    return( width );
}

public  int  GS_get_monitor_height( void )
{
    GLint  height;

    height = 1000;

    print( "GS_get_monitor_height(): not implemented\n" );

    return( height );
}

public  void  GS_clear_depth_buffer()
{
    glClear( GL_DEPTH_BUFFER_BIT );
}

private  void  clear_viewport(
     Gwindow       window,
     Colour        colour )
{
    if( window->current_bitplanes == NORMAL_PLANES )
    {
        if( window->colour_map_state )
        {
            glClearIndex( colour );
        }
        else
        {
            glClearColor( get_Colour_r_0_1(colour),
                          get_Colour_g_0_1(colour),
                          get_Colour_b_0_1(colour), 1.0 );
        }

        glClear( GL_COLOR_BUFFER_BIT );

#ifndef  TWO_D_ONLY
        if( window->zbuffer_state )
            glClear( GL_DEPTH_BUFFER_BIT );
#endif
    }
#ifndef  TWO_D_ONLY
    else
         GS_clear_overlay();
#endif
}

public  void  GS_clear_overlay()
{
#ifndef  TWO_D_ONLY
#ifdef TO_DO
    color( OVERLAY_CLEAR_INDEX );
    clear();
#endif
#endif
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

public  void  GS_clear_window(
    Gwindow    window,
    Colour     colour )
{
    clear_viewport( window, colour );
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

public  void  GS_clear_viewport(
    Gwindow      window,
    Colour       colour )
{
    BOOLEAN  scissoring;

    if( window->x_viewport_min != 0 ||
        window->x_viewport_max != window->x_size-1 ||
        window->y_viewport_min != 0 ||
        window->y_viewport_max != window->y_size-1 )
    {
        glScissor( window->x_viewport_min, window->y_viewport_min,
                   window->x_viewport_max - window->x_viewport_min + 1,
                   window->y_viewport_max - window->y_viewport_min + 1 );
        glEnable( GL_SCISSOR_TEST );
        scissoring = TRUE;
    }
    else
        scissoring = FALSE;

    clear_viewport( window, colour );

    if( scissoring )
        glDisable( GL_SCISSOR_TEST );
}

public  void  GS_flush()
{
    glFlush();
}

public  void  GS_append_to_last_update(
    Gwindow   window )
{
#ifndef  TWO_D_ONLY
    glReadBuffer( GL_FRONT );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    glRasterPos2i( 0, 0 );
    glCopyPixels( 0, 0, window->x_size, window->y_size, GL_COLOR );

    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
#endif
}

public  int  GS_get_num_overlay_planes()
{
#ifdef  TWO_D_ONLY
    return( 0 );
#else
    static  BOOLEAN  first = TRUE;
    static  int      n_planes;

    if( first )
    {
        first = FALSE;
/*
        n_planes = getgdesc( GD_BITS_OVER_SNG_CMODE );
*/
        n_planes = 0;
    }

    return( n_planes );
#endif
}

public  void  set_bitplanes( 
    Gwindow          window,
    Bitplane_types   bitplanes )
{
#ifndef  TWO_D_ONLY
#ifdef TO_DO
    set_current_window( window );

    if( G_has_overlay_planes() )
    {
        switch( bitplanes )
        {
        case NORMAL_PLANES:
            drawmode( NORMALDRAW );
            zwritemask( 0xffffffff );
            update_blend_function( window, bitplanes );
            break;

        case OVERLAY_PLANES:
            drawmode( NORMALDRAW );
            zwritemask( 0 );
            drawmode( OVERDRAW );
            update_blend_function( window, bitplanes );
            break;
        }
    }
#endif
#endif
}

public  void  GS_set_overlay_colour_map(
    Gwindow         window,
    int             index,
    Colour          colour )
{
#ifndef  TWO_D_ONLY
#ifdef TO_DO
    if( G_has_overlay_planes() )
    {
        if( window->current_bitplanes != OVERLAY_PLANES )
            set_bitplanes( window, OVERLAY_PLANES );

        mapcolor( index, get_Colour_r(colour), get_Colour_g(colour),
                         get_Colour_b(colour) );

        if( window->current_bitplanes != OVERLAY_PLANES )
            restore_bitplanes( window );
    }
#endif
#endif
}
