#include  <internal_volume_io.h>
#include  <WS_graphics.h>

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
    WSwindow               window )
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
    WSwindow  window )
{
    glutDestroyWindow( window->window_id );
}

public  Window_id  WS_get_current_window_id( void )
{
    return( glutGetWindow() );
}

public  BOOLEAN  WS_window_has_overlay_planes(
    WSwindow  window )
{
    return( FALSE );
}

private  void  set_window_normal_planes(
    WSwindow  window )
{
    glutSetWindow( window->window_id );
}

private  void  set_window_overlay_planes(
    WSwindow  window )
{
    glutSetWindow( window->window_id );
}

public  void  WS_set_current_window(
    WSwindow          window,
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

public  Window_id   WS_get_window_id(
    WSwindow  window )
{
    return( window->window_id );
}

public  void  WS_get_window_position(
    WSwindow     window,
    int          *x_pos,
    int          *y_pos )
{
    Window_id  save_id;

    save_id = glutGetWindow();

    set_window_normal_planes( window );

    *x_pos = glutGet( (GLenum) GLUT_WINDOW_X );
    *y_pos = glutGet( (GLenum) GLUT_WINDOW_Y );

    glutSetWindow( save_id );
}

public  void  WS_get_window_size(
    WSwindow     window,
    int          *x_size,
    int          *y_size )
{
    Window_id  save_id;

    save_id = glutGetWindow();

    set_window_normal_planes( window );

    *x_size = glutGet( (GLenum) GLUT_WINDOW_WIDTH );
    *y_size = glutGet( (GLenum) GLUT_WINDOW_HEIGHT );

    glutSetWindow( save_id );
}

public  void  WS_set_colour_map_entry(
    WSwindow          window,
    int               ind,
    Colour            colour )
{
    glutSetColor( ind, get_Colour_r_0_1(colour),
                       get_Colour_g_0_1(colour),
                       get_Colour_b_0_1(colour) );
}

public  void  WS_set_overlay_colour_map_entry(
    WSwindow          window,
    int               ind,
    Colour            colour )
{
}

public  void  WS_swap_buffers(
    WSwindow          window )
{
    Window_id  save_id;

    save_id = glutGetWindow();

    set_window_normal_planes( window );

    glutSwapBuffers();

    glutSetWindow( save_id );
}

static  struct
        {
            int     height;
            void    *font;
        }        known_fonts[] =
              {
                {10, GLUT_BITMAP_HELVETICA_10},
                {12, GLUT_BITMAP_HELVETICA_12},
                {18, GLUT_BITMAP_HELVETICA_18},
                {10, GLUT_BITMAP_TIMES_ROMAN_10},
                {24, GLUT_BITMAP_TIMES_ROMAN_24},
                {8,  GLUT_BITMAP_8_BY_13},
                {9 , GLUT_BITMAP_9_BY_15}
              };


private  void  *lookup_font(
    Font_types       type,
    Real             size,
    int              *actual_height )
{
    Real    diff, min_diff;
    int     which, best;
    void    *font;

    if( type == FIXED_FONT )
    {
        font = GLUT_BITMAP_8_BY_13;
        if( actual_height != NULL )
            *actual_height = 13;
    }
    else
    {
        min_diff = 0.0;
        best = 0;
        for_less( which, 0, SIZEOF_STATIC_ARRAY(known_fonts) )
        {
            diff = FABS( (Real) known_fonts[which].height - size );
            if( which == 0 || diff < min_diff )
            {
                best = which;
                min_diff = diff;
            }
        }

        font = known_fonts[best].font;
        if( actual_height != NULL )
            *actual_height = known_fonts[best].height;
    }

    return( font );
}

public  void  WS_draw_text(
    Font_types  type,
    Real        size,
    STRING      string )
{
    int   i;
    void  *font;

    font = lookup_font( type, size, NULL );

    for_less( i, 0, string_length( string ) )
        glutBitmapCharacter( font, string[i] );
}

public  Real  WS_get_character_height(
    Font_types       type,
    Real             size )
{
    int   which, height;

    (void) lookup_font( type, size, &height );

    return( height );
}

public  Real  WS_get_text_length(
    STRING           str,
    Font_types       type,
    Real             size )
{
    int    i, len;
    void   *font;

    font = lookup_font( type, size, NULL );

    len = 0;
    for_less( i, 0, (int) strlen( str ) )
        len += glutBitmapWidth( font, str[i] );

    return( (Real) len );
}

public  void  WS_get_screen_size(
    int   *x_size, 
    int   *y_size  )
{
    *x_size = glutGet( (GLenum) GLUT_SCREEN_WIDTH );
    *y_size = glutGet( (GLenum) GLUT_SCREEN_HEIGHT );
}

public  void  WS_set_update_function(
    void  (*func)( void ) )
{
    glutDisplayFunc( func );
}

public  void  WS_event_loop( void )
{
    glutMainLoop();
}
