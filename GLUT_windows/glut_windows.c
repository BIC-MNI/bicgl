#include  <internal_volume_io.h>
#include  <WS_windows.h>

private  void  initialize_glut( void )
{
    static  BOOLEAN  initialized = FALSE;
    int              argc = 0;
    char             **argv = NULL;

    if( !initialized )
    {
        initialized = TRUE;
        glutInit( &argc, argv );
    }
}

public  Status  WS_create_window(
    STRING                 title,
    int                    initial_x_pos,
    int                    initial_y_pos,
    int                    initial_x_size,
    int                    initial_y_size,
    BOOLEAN                colour_map_mode,
    BOOLEAN                double_buffer_flag,
    BOOLEAN                depth_buffer_flag,
    int                    n_overlay_planes,
    BOOLEAN                *actual_colour_map_mode,
    BOOLEAN                *actual_double_buffer_flag,
    BOOLEAN                *actual_depth_buffer_flag,
    int                    *actual_n_overlay_planes,
    WS_window_struct       *window )
{
    unsigned  int           mode;

    if( initial_x_pos >= 0 && initial_y_pos >= 0 )
        glutInitWindowPosition( initial_x_pos, initial_y_pos );

    if( initial_x_size > 0 && initial_y_size > 0 )
        glutInitWindowSize( initial_x_size, initial_y_size );

    mode = 0;

    if( colour_map_mode )
        mode |= GLUT_INDEX;
    else
        mode |= GLUT_RGB;

    if( double_buffer_flag )
        mode |= GLUT_DOUBLE;
    else
        mode |= GLUT_SINGLE;

    if( depth_buffer_flag )
        mode |= GLUT_DEPTH;

    mode |= GLUT_ALPHA;

    glutInitDisplayMode( mode );

    window->window_id = glutCreateWindow( title );

    if( window->window_id < 1 )
    {
        print_error( "Could not open GLUT window for OpenGL\n" );
        return( ERROR );
    }

    *actual_colour_map_mode = (glutGet((GLenum) GLUT_WINDOW_RGBA) != 1);
    *actual_double_buffer_flag = glutGet((GLenum) GLUT_WINDOW_DOUBLEBUFFER);
    *actual_depth_buffer_flag = (glutGet((GLenum) GLUT_WINDOW_DEPTH_SIZE) > 1);
    *actual_n_overlay_planes = 0;

    return( OK );
}

public  void  WS_delete_window(
    WS_window_struct  *window )
{
    glutDestroyWindow( window->window_id );
}

public  BOOLEAN  WS_window_has_overlay_planes(
    WS_window_struct  *window )
{
    return( FALSE );
}

private  void  set_window_normal_planes(
    WS_window_struct  *window )
{
    glutSetWindow( window->window_id );
}

private  void  set_window_overlay_planes(
    WS_window_struct  *window )
{
    glutSetWindow( window->window_id );
}

public  void  WS_set_current_window(
    WS_window_struct  *window,
    Bitplane_types    current_bitplanes )
{
    if( current_bitplanes == OVERLAY_PLANES )
        set_window_overlay_planes( window );
    else
        set_window_normal_planes( window );
}


public  int    WS_get_n_overlay_planes( void )
{
    return( 0 );
}

public  BOOLEAN  WS_get_event(
    Event_types          *event_type,
    Window_id            *window,
    event_info_struct    *info )
{
    return( FALSE );
}

public  void  WS_get_window_position(
    WS_window_struct  *window,
    int               *x_pos,
    int               *y_pos )
{
    set_window_normal_planes( window );

    *x_pos = glutGet( (GLenum) GLUT_WINDOW_X );
    *y_pos = glutGet( (GLenum) GLUT_WINDOW_Y );
}

public  void  WS_get_window_size(
    WS_window_struct  *window,
    int               *x_size,
    int               *y_size )
{
    set_window_normal_planes( window );

    *x_size = glutGet( (GLenum) GLUT_WINDOW_WIDTH );
    *y_size = glutGet( (GLenum) GLUT_WINDOW_HEIGHT );
}

public  void  WS_set_colour_map_entry(
    WS_window_struct  *window,
    int               ind,
    Colour            colour )
{
    glutSetColor( ind, get_Colour_r_0_1(colour),
                       get_Colour_g_0_1(colour),
                       get_Colour_b_0_1(colour) );
}

public  void  WS_set_overlay_colour_map_entry(
    WS_window_struct  *window,
    int               ind,
    Colour            colour )
{
}

public  void  WS_swap_buffers(
    WS_window_struct  *window )
{
    set_window_normal_planes( window );

    glutSwapBuffers();
}

static  struct
        {
            int     size;
            void    *font;
        }        known_fonts[] =
              {
                {10, GLUT_BITMAP_HELVETICA_10},
                {12, GLUT_BITMAP_HELVETICA_12},
                {18, GLUT_BITMAP_HELVETICA_18},
                {10, GLUT_BITMAP_TIMES_ROMAN_10},
                {24, GLUT_BITMAP_TIMES_ROMAN_24},
                {8, GLUT_BITMAP_8_BY_13},
                {9, GLUT_BITMAP_9_BY_15}
              };


public  BOOLEAN  WS_get_font(
    Font_types       type,
    Real             size,
    WS_font_info     *font_info )
{
    Real    diff, min_diff;
    int     which, best;

    if( type == FIXED_FONT )
    {
        font_info->font = GLUT_BITMAP_8_BY_13;
    }
    else
    {
        min_diff = 0.0;
        best = 0;
        for_less( which, 0, SIZEOF_STATIC_ARRAY(known_fonts) )
        {
            diff = FABS( (Real) known_fonts[which].size - size );
            if( which == 0 || diff < min_diff )
            {
                best = which;
                min_diff = diff;
            }
        }

        font_info->font = known_fonts[best].font;
    }

    return( TRUE );
}

/* ARGSUSED */

public  void  WS_build_font_in_window(
    WS_window_struct  *window,
    int               font_index,
    WS_font_info      *font_info )
{
}

/* ARGSUSED */

public  void  WS_delete_font_in_window(
    WS_window_struct     *window,
    int                  font_index,
    WS_font_info         *font_info )
{
}

/* ARGSUSED */

public  BOOLEAN  WS_set_font(
    WS_window_struct     *window,
    int                  font_index,
    WS_font_info         *font_info )
{
    window->current_font = font_info->font;

    return( TRUE );
}

public  void  WS_delete_font(
    WS_font_info  *info )
{
}

public  Real  WS_get_character_height(
    WS_font_info  *font_info )
{
    int  which;

    for_less( which, 0, SIZEOF_STATIC_ARRAY(known_fonts) )
    {
        if( known_fonts[which].font == font_info->font )
            break;
    }

    if( which >= SIZEOF_STATIC_ARRAY(known_fonts) )
    {
        print_error( "Invalid font in WS_get_character_height\n" );
        return( 0.0 );
    }

    return( known_fonts[which].size );
}

public  Real  WS_get_text_length(
    WS_font_info     *font_info,
    STRING           str )
{
    int    i, len;

    len = 0;

    for_less( i, 0, (int) strlen( str ) )
    {
        len += glutBitmapWidth( font_info->font, str[i] );
    }

    return( (Real) len );
}

public  void  WS_get_screen_size(
    int   *x_size, 
    int   *y_size  )
{
    *x_size = glutGet( (GLenum) GLUT_SCREEN_WIDTH );
    *y_size = glutGet( (GLenum) GLUT_SCREEN_HEIGHT );
}
