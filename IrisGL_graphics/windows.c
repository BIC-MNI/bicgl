 
#include  <internal_volume_io.h>
#include  <gs_specific.h>
#include  <gl/get.h>

private  Window_id  get_window_id( 
    Gwindow  window )
{
    return( window->WS_window->window_id );
}

#define  CALL_GFLUSH_EVERY_NTH_TIME  200

#define  OVERLAY_CLEAR_INDEX       0
#define  DEFAULT_N_CURVE_SEGMENTS  8

#define  CATMULL_ROM_ID            1

private  void  initialize_window(
    Gwindow        window,
    BOOLEAN        colour_map_desired,
    BOOLEAN        double_buffer_desired,
    BOOLEAN        depth_buffer_desired,
    int            n_overlay_planes_desired );

private  long     zdepth_clear;

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_initialize
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Initializes any graphics type stuff, is called just before first
              graphics window is opened.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1993            David MacDonald
@MODIFIED   : 
---------------------------------------------------------------------------- */

public  void  GS_initialize( void )
{
    static  Matrix   catmull_rom_matrix = {
                                              { -0.5,  1.5, -1.5,  0.5 },
                                              {  1.0, -2.5,  2.0, -0.5 },
                                              { -0.5,  0.0,  0.5,  0.0 },
                                              {  0.0,  1.0,  0.0,  0.0 },
                                          };

    noport();
    foreground();
    (void) winopen( "" );

#ifndef  TWO_D_ONLY
    zdepth_clear = getgdesc( GD_ZMAX );
#endif

#ifndef  TWO_D_ONLY
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

public  void  GS_terminate( void )
{
    gexit();
}

public  BOOLEAN  GS_can_switch_double_buffering()
{
    return( TRUE );
}

public  BOOLEAN  GS_can_switch_colour_map_mode()
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
    Status   status;

    if( width > 0 && height > 0 )
    {
        if( x_pos >= 0 && y_pos >= 0 )
        {
            prefposition( (long) x_pos, (long) x_pos + width - 1,
                          (long) y_pos, (long) y_pos + height - 1 );
        }
        else
        {
            prefsize( (long) width, (long) height );
        }
    }

    foreground();

    window->WS_window->window_id = winopen( title );

    if( width > 0 && height > 0 )
    {
        winconstraints();
        winconstraints();
    }

    if( get_window_id(window) >= 0 )
    {
        *actual_colour_map_flag = colour_map_desired;
        *actual_double_buffer_flag = double_buffer_desired;
        *actual_depth_buffer_flag = depth_buffer_desired;
        *actual_n_overlay_planes = n_overlay_planes_desired;

        initialize_window( window,
                           colour_map_desired,
                           double_buffer_desired,
                           depth_buffer_desired,
                           n_overlay_planes_desired );
        status = OK;
    }
    else
        status = ERROR;

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
    Gwindow        window,
    BOOLEAN        colour_map_desired,
    BOOLEAN        double_buffer_desired,
    BOOLEAN        depth_buffer_desired,
    int            n_overlay_planes_desired )
{
    set_current_window( window );

    (void) GS_set_double_buffer_state( double_buffer_desired );

    GS_set_colour_map_state( colour_map_desired );

    initialize_window_events( window );

#ifndef  TWO_D_ONLY
    subpixel( TRUE );

    if( n_overlay_planes_desired > 0 )
    {
        overlay( n_overlay_planes_desired );
        gconfig();

        G_set_overlay_colour_map( window, 1, RED );
        G_set_overlay_colour_map( window, 2, GREEN );
        G_set_overlay_colour_map( window, 3, BLUE );
    }
#endif
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
    return( getgdesc( GD_BLEND ) != 0 );
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
    return( getgdesc( GD_BITS_NORM_SNG_RED ) > 0 &&
            getgdesc( GD_BITS_NORM_SNG_GREEN ) > 0 &&
            getgdesc( GD_BITS_NORM_SNG_BLUE ) > 0 );
}

public  BOOLEAN  GS_set_double_buffer_state(
    BOOLEAN        flag )
{
    long   display_mode;

    if( flag )
        doublebuffer();
    else
        singlebuffer();

    gconfig();

    display_mode = getdisplaymode();
    if( display_mode == DMRGBDOUBLE || display_mode == DMDOUBLE )
        flag = TRUE;
    else
        flag = FALSE;

    return( flag );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : GS_set_colour_map_state
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

public  void  GS_set_colour_map_state(
    BOOLEAN        flag )
{
    if( flag )
        cmode();
    else
        RGBmode();

    gconfig();
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
    Gwindow  window )
{
    int   n_bits, n_colours, query;

    if( window->double_buffer_state )
    {
        query = GD_BITS_NORM_DBL_CMODE;
    }
    else
    {
        query = GD_BITS_NORM_SNG_CMODE;
    }

    n_bits = getgdesc( query );

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
    static   int  count = 0;

    mapcolor( (Colorindex) index,
              (short) get_Colour_r(colour),
              (short) get_Colour_g(colour),
              (short) get_Colour_b(colour) );

    if( window->current_bitplanes == NORMAL_PLANES )
    {
        ++count;
        if( count == CALL_GFLUSH_EVERY_NTH_TIME )
        {
            gflush();
            count = 0;
        }
    }
}

public  BOOLEAN  GS_is_double_buffer_supported( void )
{
    BOOLEAN   available;

    if( getgdesc( GD_BITS_NORM_DBL_RED ) <= 0 ||
        getgdesc( GD_BITS_NORM_DBL_GREEN ) <= 0 ||
        getgdesc( GD_BITS_NORM_DBL_BLUE ) <= 0 )
    {
        available = FALSE;
    }
    else
    {
        available = TRUE;
    }

    return( available );
}

public  BOOLEAN  GS_is_depth_buffer_supported( void )
{
#ifdef  TWO_D_ONLY
    return( FALSE );
#else
    BOOLEAN   available;

    available = (getgdesc( GD_BITS_NORM_ZBUFFER ) > 0);

    return( available );
#endif
}

public  void  GS_set_depth_buffer_state(
    BOOLEAN         flag )
{
#ifndef  TWO_D_ONLY
    zbuffer( flag );
#endif
}

public  void  GS_set_depth_function(
    Depth_functions   depth_func )
{
#ifndef  TWO_D_ONLY
    int   gl_depth_func;

    switch( depth_func )
    {
    case LESS_OR_EQUAL:
        gl_depth_func = ZF_LEQUAL;
        break;
    default:
        gl_depth_func = ZF_LEQUAL;
        break;
    }

    zfunction( gl_depth_func  );
#endif
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

    if( get_window_id(window) >= 0 )
    {
#ifndef  TWO_D_ONLY
        if( G_has_overlay_planes() )
        {
            viewport( 0, window->x_size-1, 0, window->y_size-1 );
            drawmode( OVERDRAW );
            color( 0 );
            clear();
        }
#endif

        winclose( get_window_id(window) );

        status = OK;
    }
    else
    {
        print_error( "Error:  tried to delete invalid window.\n" );
        status = ERROR;
    }

    return( status );
}

/* ARGSUSED */

public  void  WS_set_current_window(
    WS_window_struct  *window,
    Bitplane_types    current_bitplanes )
{
    winset( window->window_id );
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
    long   width;

    width = getgdesc( GD_XPMAX );

    return( (int) width );
}

public  int  GS_get_monitor_height( void )
{
    long   height;

    height = getgdesc( GD_YPMAX );

    return( (int) height );
}

public  void  GS_clear_depth_buffer()
{
    zclear();
}

private   void  clear_viewport(
    Gwindow       window,
    Colour        colour )
{
    if( window->current_bitplanes == NORMAL_PLANES )
    {
        if( window->colour_map_state )
        {
            GS_set_colour_index( colour );
            clear();
#ifndef  TWO_D_ONLY
            if( window->zbuffer_state )
                zclear();
#endif
        }
        else
        {
            if( window->zbuffer_state )
                czclear( colour, zdepth_clear );
            else
            {
                GS_set_colour( colour );
                clear();
            }
        }
    }
#ifndef  TWO_D_ONLY
    else
        GS_clear_overlay();
#endif
}

public  void  GS_clear_overlay()
{
    GS_set_colour_index( OVERLAY_CLEAR_INDEX );
    clear();
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
    pushviewport();
    viewport( 0, window->x_size-1, 0, window->y_size-1 );
    clear_viewport( window, colour );
    popviewport();
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
    clear_viewport( window, colour );
}

public  void  GS_flush()
{
    gflush();
}

/* ARGSUSED */

public  void  WS_swap_buffers(
    WS_window_struct  *window )
{
    swapbuffers();
}

public  void  GS_append_to_last_update(
    Gwindow   window )
{
#ifndef  TWO_D_ONLY
    BOOLEAN   zbuffer_on;

    zbuffer_on = window->zbuffer_state;

    if( zbuffer_on )
        G_set_zbuffer_state( window, FALSE );

    readsource( SRC_FRONT );

    rectcopy( (Screencoord) 0,
              (Screencoord) 0,
              (Screencoord) (window->x_size-1),
              (Screencoord) (window->y_size-1),
              (Screencoord) 0,
              (Screencoord) 0 );

    if( zbuffer_on )
        G_set_zbuffer_state( window, TRUE );
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
        n_planes = getgdesc( GD_BITS_OVER_SNG_CMODE );
    }

    return( n_planes );
#endif
}

public  void  set_bitplanes( 
    Gwindow          window,
    Bitplane_types   bitplanes )
{
#ifndef  TWO_D_ONLY
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
}

public  void  GS_set_overlay_colour_map(
    Gwindow         window,
    int             index,
    Colour          colour )
{
#ifndef  TWO_D_ONLY
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
}

public  void  WS_get_window_position(
    WS_window_struct  *window,
    int               *x_pos,
    int               *y_pos )
{
    long    lx_pos, ly_pos;

    getorigin( &lx_pos, &ly_pos );
    *x_pos = (int) lx_pos;
    *y_pos = (int) ly_pos;
}

public  void  WS_get_window_size(
    WS_window_struct  *window,
    int               *x_size,
    int               *y_size )
{
    long    lx_size, ly_size;

    getsize( &lx_size, &ly_size );
    *x_size = (int) lx_size;
    *y_size = (int) ly_size;
}

public  Gwindow   find_window_for_id(
    Window_id  window_id )
{
    int            i;
    Gwindow        window;

    for_less( i, 0, get_n_graphics_windows() )
    {
        if( get_window_id( get_nth_graphics_window(i) ) == window_id )
            break;
    }

    if( i >= get_n_graphics_windows() )
        window = (Gwindow) 0;
    else
        window = get_nth_graphics_window(i);

    return( window );
}
