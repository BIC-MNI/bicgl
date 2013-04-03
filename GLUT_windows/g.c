#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H  

#include  <volume_io.h>
#include  <WS_graphics.h>

#define  DEFAULT_WINDOW_X_SIZE    300
#define  DEFAULT_WINDOW_Y_SIZE    300

#define  USING_X11

static  int         n_windows_to_delete = 0;
static  Window_id   *windows_to_delete = NULL;

static  void  set_event_callbacks_for_current_window( VIO_BOOL );
static  void  resize_function(
    int   width,
    int   height );
static  int  flip_screen_y(
    int   screen_y );
static  void  set_colour_map_entry(
    int               ind,
    VIO_Colour            colour );

#ifdef DEBUG

static  int  my_glutCreateWindow(
    STRING  title )
{
    (void) printf( "(void) glutCreateWindow( \"%s\" );\n", title );

    return( glutCreateWindow( title ) );
}
#define  glutCreateWindow my_glutCreateWindow

static  void  my_glutInit(
    int   *argc,
    char  *argv[] )
{
    (void) printf( "glutInit( &argc, argv );\n" );

    glutInit( argc, argv );
}
#define  glutInit my_glutInit

static  void  my_glutInitWindowPosition(
    int   x,
    int   y )
{
    (void) printf( "glutInitWindowPosition( %d, %d );\n", x, y );

    glutInitWindowPosition( x, y );
}
#define  glutInitWindowPosition my_glutInitWindowPosition

static  void  my_glutInitWindowSize(
    int   x,
    int   y )
{
    (void) printf( "glutInitWindowSize( %d, %d );\n", x, y );

    glutInitWindowSize( x, y );
}
#define  glutInitWindowSize my_glutInitWindowSize

static  void  my_glutInitDisplayMode(
    int   x )
{
    (void) printf( "glutInitDisplayMode( %d );\n", x );

    glutInitDisplayMode( x );
}
#define  glutInitDisplayMode my_glutInitDisplayMode

static  void  my_glutUseLayer(
    GLenum   x )
{
    (void) printf( "glutUseLayer( %d );\n", x );

    glutUseLayer( x );
}
#define  glutUseLayer my_glutUseLayer

static  void  my_glutPopWindow(
    void )
{
    (void) printf( "glutPopWindow();\n" );

    glutPopWindow();
}
#define  glutPopWindow my_glutPopWindow

static  void  my_glutDestroyWindow(
    int   x )
{
    (void) printf( "glutDestroyWindow( %d );\n", x );

    glutDestroyWindow( x );
}
#define  glutDestroyWindow my_glutDestroyWindow

static  void  my_glutSetWindow(
    int   x )
{
    (void) printf( "glutSetWindow( %d );\n", x );

    glutSetWindow( x );
}
#define  glutSetWindow my_glutSetWindow

static  void  my_glutMainLoop(
    void )
{
    (void) printf( "/* glutMainLoop(); */\n" );

    glutMainLoop();
}
#define  glutMainLoop my_glutMainLoop

static  void  my_glutSetColor(
    int  index,
    float r,
    float g,
    float b )
{
    (void) printf( "glutSetColor( %d, %g, %g, %g );\n", index, r, g, b );

    glutSetColor( index, r, g, b );
}
#define  glutSetColor my_glutSetColor

static  void  my_glutDisplayFunc(
    void  (*func)( void ) )
{
    (void) printf( "glutDisplayFunc( &display_it );\n" );

    glutDisplayFunc( func );
}
#define  glutDisplayFunc my_glutDisplayFunc

#endif  /* DEBUG */

/*----------------------------------------------------- */

  void  WS_initialize( void )
{
    static  VIO_BOOL  initialized = FALSE;
    int              argc = 1;
    char             *argv[] = { "no_program_name" };

    if( !initialized )
    {
        initialized = TRUE;
        glutInit( &argc, argv );
    }
}

 int  test_glut_window(
    VIO_BOOL   colour_map_mode,
    VIO_BOOL   double_buffer_flag,
    VIO_BOOL   depth_buffer_flag )
{
    int                rgba, doub, depth;
    unsigned  int      mode;
    Window_id          window_id;
    char               title[1000];

    WS_initialize();

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
    
    glutInitDisplayMode( mode );

    if( !glutGet( (GLenum) GLUT_DISPLAY_MODE_POSSIBLE ) )
    {
        (void) printf( "Test window: Could not open GLUT window in Display mode (%d,%d,%d) for OpenGL\n",
                     colour_map_mode, double_buffer_flag, depth_buffer_flag );
        return( -1 );
    }

    (void) sprintf( title, "Title: %d %d %d",
                    colour_map_mode, double_buffer_flag, depth_buffer_flag );

    window_id = glutCreateWindow( title );

    if( window_id < 1 )
    {
        (void) printf( "Test window: Could not open GLUT window for OpenGL\n" );
        return( -1 );
    }

    rgba = glutGet((GLenum) GLUT_WINDOW_RGBA);
    doub = glutGet((GLenum) GLUT_WINDOW_DOUBLEBUFFER);
    depth = glutGet((GLenum) GLUT_WINDOW_DEPTH_SIZE);

    if( rgba != !colour_map_mode ||
        doub != double_buffer_flag ||
        (depth > 0) != depth_buffer_flag )
    {
        (void) printf( "Test window: Could not get requested mode( %d %d %d ), got( %d %d %d )\n",
                     colour_map_mode, double_buffer_flag, depth_buffer_flag,
                     !rgba, doub, (depth > 0 ) );
    }

    return( window_id );
}

  void  test_glut_windows()
{
    int  window_id;

    window_id = test_glut_window( FALSE, FALSE, FALSE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( FALSE, FALSE, TRUE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( FALSE, TRUE, FALSE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( FALSE, TRUE, TRUE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( TRUE, FALSE, FALSE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( TRUE, FALSE, TRUE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( TRUE, TRUE, FALSE );
    glutDestroyWindow( window_id );

    window_id = test_glut_window( TRUE, TRUE, TRUE );
    glutDestroyWindow( window_id );
}

static  Window_id  create_GLUT_window(
    STRING                 title,
    int                    initial_x_pos,
    int                    initial_y_pos,
    int                    initial_x_size,
    int                    initial_y_size,
    VIO_BOOL                colour_map_mode,
    VIO_BOOL                double_buffer_flag,
    VIO_BOOL                depth_buffer_flag,
    int                    n_overlay_planes,
    VIO_BOOL                *actual_colour_map_mode_ptr,
    VIO_BOOL                *actual_double_buffer_flag_ptr,
    VIO_BOOL                *actual_depth_buffer_flag_ptr,
    int                    *actual_n_overlay_planes_ptr )
{
    int                rgba, doub, depth;
    unsigned  int      mode;
    int                used_size;
    Window_id          window_id;
    VIO_BOOL            actual_colour_map_mode;
    VIO_BOOL            actual_double_buffer_flag;
    VIO_BOOL            actual_depth_buffer_flag;
    int                actual_n_overlay_planes;

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
    
    if( initial_x_pos >= 0 && initial_y_pos >= 0 )
    {
        if( initial_y_size <= 0 )
            used_size = glutGet( (GLenum) GLUT_INIT_WINDOW_HEIGHT );
        else
            used_size = initial_y_size;

        glutInitWindowPosition( initial_x_pos,
                                flip_screen_y( initial_y_pos + used_size - 1) );
    }
    else
        glutInitWindowPosition( -1, -1 );

    if( initial_x_size > 0 && initial_y_size > 0 )
        glutInitWindowSize( initial_x_size, initial_y_size );
    else
        glutInitWindowSize( DEFAULT_WINDOW_X_SIZE, DEFAULT_WINDOW_Y_SIZE );

    glutInitDisplayMode( mode );

#ifdef BUG_ON_DISPLAY_ON_LINUX_XTERM
/*
  --- when running on SGI, displaying on Linux xterm, doing this get causes
      some windows to get the wrong colour map mode
*/

    if( !glutGet( (GLenum) GLUT_DISPLAY_MODE_POSSIBLE ) &&
        double_buffer_flag )
    {
        print_error( "Double buffer mode unavailable, trying single buffer\n" );
        mode -= GLUT_DOUBLE;
        mode |= GLUT_SINGLE;
        glutInitDisplayMode( mode );
    }

    if( !glutGet( (GLenum) GLUT_DISPLAY_MODE_POSSIBLE ) )
    {
        print_error( "Could not open GLUT window in Display mode (%d,%d) for OpenGL\n",
                     colour_map_mode, double_buffer_flag );
        return( -1 );
    }
#endif

    window_id = glutCreateWindow( title );

    if( window_id < 1 )
    {
        print_error( "Could not open GLUT window for OpenGL\n" );
        return( -1 );
    }

    if( actual_double_buffer_flag )
        glDrawBuffer( GL_BACK );

    rgba = glutGet((GLenum) GLUT_WINDOW_RGBA);
    doub = glutGet((GLenum) GLUT_WINDOW_DOUBLEBUFFER);
    depth = glutGet((GLenum) GLUT_WINDOW_DEPTH_SIZE);

    /* why set this? it is the default layer and only gives greif with some GLUT implementations. */
    /* glutUseLayer( GLUT_NORMAL ); */

    glutPopWindow();

    actual_n_overlay_planes = 0;
    set_event_callbacks_for_current_window( actual_n_overlay_planes );

    actual_colour_map_mode = !rgba;
    actual_double_buffer_flag = doub;
    actual_depth_buffer_flag = (depth > 0);

    if( actual_colour_map_mode != colour_map_mode )
    {
        print_error( "Could not get requested colour_map_mode(%d), got(%d,%d)\n",
                     colour_map_mode,
                     !glutGet( (GLenum) GLUT_WINDOW_RGBA ),
                     glutGet( (GLenum) GLUT_WINDOW_COLORMAP_SIZE ) );

/*
        glutDestroyWindow( window_id );
        window_id = -1;
*/
    }

    if( actual_double_buffer_flag && !double_buffer_flag )
    {

        /*--- print_error( "For some reason got double buffer window, when requesting single buffer.\n" );*/

        /*--- testing has shown that it actually does get a single
              buffer mode, but reports double, so there may be a bug
              in GLUT, and we just assign the double buffering to FALSE */
        actual_double_buffer_flag = FALSE;
    }
    else if( !actual_double_buffer_flag && double_buffer_flag )
    {
        print_error( "Could not get requested double buffer window\n" );
    }

#ifdef  USING_X11
    if( window_id >= 1 && actual_colour_map_mode )
    {
        int   n_colours_to_copy;

        n_colours_to_copy = glutGet( (GLenum) GLUT_WINDOW_COLORMAP_SIZE );

        copy_X_colours( n_colours_to_copy );
    }
#endif

    if( actual_colour_map_mode_ptr != NULL )
        *actual_colour_map_mode_ptr = actual_colour_map_mode;

    if( actual_double_buffer_flag_ptr != NULL )
        *actual_double_buffer_flag_ptr = actual_double_buffer_flag;

    if( actual_depth_buffer_flag_ptr != NULL )
        *actual_depth_buffer_flag_ptr = actual_depth_buffer_flag;

    if( actual_n_overlay_planes_ptr != NULL )
        *actual_n_overlay_planes_ptr = actual_n_overlay_planes;

    return( window_id );
}

static  void  delete_GLUT_window(
    Window_id   window_id )
{
    Window_id  current;

    current = glutGetWindow();
    glutSetWindow( window_id );
    glutHideWindow();
    glutSetWindow( current );
/*
    glutDestroyWindow( window_id );
*/
}

static  void  reestablish_colour_map_in_new_window(
    WSwindow               window )
{
    int   ind, n_colours, prev_n_colours;

    if( ! glutGet((GLenum) GLUT_WINDOW_RGBA) )
    {
        prev_n_colours = window->n_colours;

        n_colours = glutGet( (GLenum) GLUT_WINDOW_COLORMAP_SIZE );

        for_less( ind, 0, MIN(n_colours,prev_n_colours) )
        {
            if( window->colour_map_entry_set[ind] )
                set_colour_map_entry( ind, window->colour_map[ind] );
        }

        if( prev_n_colours > 0 )
        {
            REALLOC( window->colour_map_entry_set, n_colours );
            REALLOC( window->colour_map, n_colours );
        }
        else
        {
            ALLOC( window->colour_map_entry_set, n_colours );
            ALLOC( window->colour_map, n_colours );
        }

        window->n_colours = n_colours;

        for_less( ind, prev_n_colours, n_colours )
            window->colour_map_entry_set[ind] = (VIO_SCHAR) FALSE;
    }

}

  Status  WS_create_window(
    STRING                 title,
    int                    initial_x_pos,
    int                    initial_y_pos,
    int                    initial_x_size,
    int                    initial_y_size,
    VIO_BOOL                colour_map_mode,
    VIO_BOOL                double_buffer_flag,
    VIO_BOOL                depth_buffer_flag,
    int                    n_overlay_planes,
    VIO_BOOL                *actual_colour_map_mode,
    VIO_BOOL                *actual_double_buffer_flag,
    VIO_BOOL                *actual_depth_buffer_flag,
    int                    *actual_n_overlay_planes,
    WSwindow               window )
{
    window->window_id = create_GLUT_window( title, 
                                            initial_x_pos,
                                            initial_y_pos,
                                            initial_x_size,
                                            initial_y_size,
                                            colour_map_mode,
                                            double_buffer_flag,
                                            depth_buffer_flag,
                                            n_overlay_planes,
                                            actual_colour_map_mode,
                                            actual_double_buffer_flag,
                                            actual_depth_buffer_flag,
                                            actual_n_overlay_planes );


    if( window->window_id < 1 )
        return( VIO_ERROR );

    glutSetWindow( window->window_id );
    window->title = create_string( title );

    window->n_colours = 0;
    window->colour_map_entry_set = NULL;
    window->colour_map = NULL;

    reestablish_colour_map_in_new_window( window );

    return( VIO_OK );
}

  VIO_BOOL  WS_set_double_buffer_state(
    WSwindow               window,
    VIO_BOOL                double_buffer_flag )
{
    VIO_BOOL    colour_map_mode, depth_buffer_flag;
    int        n_overlay_planes, actual_n_overlay_planes;
    int        x_pos, y_pos, x_size, y_size;
    VIO_BOOL    actual_colour_map_mode, actual_double_buffer_flag;
    VIO_BOOL    actual_depth_buffer_flag;
    Window_id  old_window_id;

    old_window_id = window->window_id;

    glutSetWindow( window->window_id );

    colour_map_mode = glutGet( (GLenum) GLUT_WINDOW_RGBA ) == 0;
    depth_buffer_flag = glutGet( (GLenum) GLUT_WINDOW_DEPTH_SIZE ) > 0;
    n_overlay_planes = 0;

    x_pos = glutGet( (GLenum) GLUT_WINDOW_X );
    y_pos = glutGet( (GLenum) GLUT_WINDOW_Y );
    x_size = glutGet( (GLenum) GLUT_WINDOW_WIDTH );
    y_size = glutGet( (GLenum) GLUT_WINDOW_HEIGHT );

    y_pos = flip_screen_y( y_pos + y_size - 1 );

    window->window_id = create_GLUT_window( window->title,
                                            x_pos,
                                            y_pos,
                                            x_size,
                                            y_size,
                                            colour_map_mode,
                                            double_buffer_flag,
                                            depth_buffer_flag,
                                            n_overlay_planes,
                                            &actual_colour_map_mode,
                                            &actual_double_buffer_flag,
                                            &actual_depth_buffer_flag,
                                            &actual_n_overlay_planes );

    if( window->window_id < 1 )
    {
        print_error( "Could not open GLUT window for OpenGL\n" );
        window->window_id = old_window_id;
    }
    else
    {
        reestablish_colour_map_in_new_window( window );
        ADD_ELEMENT_TO_ARRAY( windows_to_delete, n_windows_to_delete,
                              old_window_id, 1 );
    }

    glutSetWindow( window->window_id );

    return( actual_double_buffer_flag );
}

  VIO_BOOL  WS_set_colour_map_state(
    WSwindow               window,
    VIO_BOOL                colour_map_flag )
{
    VIO_BOOL    double_buffer_state, depth_buffer_flag;
    int        n_overlay_planes, actual_n_overlay_planes;
    int        x_pos, y_pos, x_size, y_size;
    VIO_BOOL    actual_colour_map_mode, actual_double_buffer_flag;
    VIO_BOOL    actual_depth_buffer_flag;
    Window_id  old_window_id;

    old_window_id = window->window_id;
    glutSetWindow( window->window_id );

    double_buffer_state = glutGet( (GLenum) GLUT_WINDOW_DOUBLEBUFFER ) != 0;
    depth_buffer_flag = glutGet( (GLenum) GLUT_WINDOW_DEPTH_SIZE ) > 0;
    n_overlay_planes = 0;

    x_pos = glutGet( (GLenum) GLUT_WINDOW_X );
    y_pos = glutGet( (GLenum) GLUT_WINDOW_Y );
    x_size = glutGet( (GLenum) GLUT_WINDOW_WIDTH );
    y_size = glutGet( (GLenum) GLUT_WINDOW_HEIGHT );

    y_pos = flip_screen_y( y_pos + y_size - 1 );

    window->window_id = create_GLUT_window( window->title,
                                            x_pos, y_pos,
                                            x_size, y_size,
                                            colour_map_flag,
                                            double_buffer_state,
                                            depth_buffer_flag,
                                            n_overlay_planes,
                                            &actual_colour_map_mode,
                                            &actual_double_buffer_flag,
                                            &actual_depth_buffer_flag,
                                            &actual_n_overlay_planes );

    if( window->window_id < 1 )
    {
        print_error( "Could not open GLUT window for OpenGL\n" );
        window->window_id = old_window_id;
    }
    else
    {
        reestablish_colour_map_in_new_window( window );
        ADD_ELEMENT_TO_ARRAY( windows_to_delete, n_windows_to_delete,
                              old_window_id, 1 );
    }

    glutSetWindow( window->window_id );

    return( actual_colour_map_mode );
}

  void  WS_set_window_title(
    WSwindow   window,
    STRING     title )
{
    glutSetWindowTitle( title );
}


  void  WS_delete_window(
    WSwindow  window )
{
    delete_GLUT_window( window->window_id );
    delete_string( window->title );
}

static  Window_id  get_current_event_window( void )
{
    return( glutGetWindow() );
}

  VIO_BOOL  WS_window_has_overlay_planes(
    WSwindow  window )
{
    return( FALSE );
}

static  void  set_window_normal_planes(
    WSwindow  window )
{
    glutSetWindow( window->window_id );
}

static  void  set_window_overlay_planes(
    WSwindow  window )
{
    glutSetWindow( window->window_id );
}

  void  WS_set_current_window(
    WSwindow          window )
{
    WS_set_bitplanes( window, NORMAL_PLANES );
}

  Window_id   WS_get_window_id(
    WSwindow  window )
{
    return( window->window_id );
}

  void  WS_set_bitplanes(
    WSwindow          window,
    Bitplane_types    bitplanes )
{
    if( bitplanes == OVERLAY_PLANES )
        set_window_overlay_planes( window );
    else
        set_window_normal_planes( window );
}


  int    WS_get_n_overlay_planes( void )
{
    return( 0 );
}

  void  WS_get_window_position(
    int          *x_pos,
    int          *y_pos )
{
    *x_pos = glutGet( (GLenum) GLUT_WINDOW_X );
    *y_pos = flip_screen_y( glutGet( (GLenum) GLUT_WINDOW_Y ) +
                            glutGet( (GLenum) GLUT_WINDOW_HEIGHT ) - 1 );
}

  void  WS_get_window_size(
    int          *x_size,
    int          *y_size )
{
    *x_size = glutGet( (GLenum) GLUT_WINDOW_WIDTH );
    *y_size = glutGet( (GLenum) GLUT_WINDOW_HEIGHT );
}

  void  glut_set_colour_entry(
    int      ind,
    VIO_Real     r,
    VIO_Real     g,
    VIO_Real     b )
{
    glutSetColor( ind, (float) r, (float) g, (float) b );
}

static  void  set_colour_map_entry(
    int               ind,
    VIO_Colour            colour )
{
    glut_set_colour_entry( ind,
                           get_Colour_r_0_1(colour),
                           get_Colour_g_0_1(colour),
                           get_Colour_b_0_1(colour) );
}

  void  WS_set_colour_map_entry(
    WSwindow          window,
    Bitplane_types    bitplane,
    int               ind,
    VIO_Colour            colour )
{
    set_colour_map_entry( ind, colour );
    window->colour_map_entry_set[ind] = TRUE;
    window->colour_map[ind] = colour;
}

  void  WS_set_overlay_colour_map_entry(
    WSwindow          window,
    int               ind,
    VIO_Colour            colour )
{
}

  void  WS_swap_buffers( void )
{
    if(glutGetWindow()) //VF:a hack
      glutSwapBuffers();
}

static  struct
        {
            int     height;
            void    *font;
        }        known_fonts[] =
              {
                {10, GLUT_BITMAP_HELVETICA_10},
                {12, GLUT_BITMAP_HELVETICA_12},
                {13,  GLUT_BITMAP_8_BY_13},
                {15 , GLUT_BITMAP_9_BY_15},
                {18, GLUT_BITMAP_HELVETICA_18},
                {24, GLUT_BITMAP_TIMES_ROMAN_24},
                {10, GLUT_BITMAP_TIMES_ROMAN_10}
              };


static  void  *lookup_font(
    Font_types       type,
    VIO_Real             size,
    int              *actual_height )
{
    VIO_Real    diff, min_diff;
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
        for_less( which, 0, VIO_SIZEOF_STATIC_ARRAY(known_fonts) )
        {
            diff = FABS( (VIO_Real) known_fonts[which].height - size );
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

  void  WS_draw_text(
    Font_types  type,
    VIO_Real        size,
    STRING      string )
{
    int   i;
    void  *font;

    font = lookup_font( type, size, NULL );

    for_less( i, 0, string_length( string ) )
        glutBitmapCharacter( font, (int) string[i] );
}

  VIO_Real  WS_get_character_height(
    Font_types       type,
    VIO_Real             size )
{
    int   height;

    (void) lookup_font( type, size, &height );

    return( (VIO_Real) height );
}

  VIO_Real  WS_get_text_length(
    STRING           str,
    Font_types       type,
    VIO_Real             size )
{
    int    i, len;
    void   *font;

    font = lookup_font( type, size, NULL );

    len = 0;
    for_less( i, 0, (int) strlen( str ) )
        len += glutBitmapWidth( font, (int) str[i] );

    return( (VIO_Real) len );
}

  void  WS_get_screen_size(
    int   *x_size, 
    int   *y_size  )
{
    *x_size = glutGet( (GLenum) GLUT_SCREEN_WIDTH );
    *y_size = glutGet( (GLenum) GLUT_SCREEN_HEIGHT );
}

  void  WS_set_mouse_position(
    int       x_screen,
    int       y_screen )
{
}

static  void  (*display_callback) ( Window_id );
static  void  (*display_overlay_callback) ( Window_id );
static  void  (*resize_callback) ( Window_id, int, int, int, int );
static  void  (*key_down_callback) ( Window_id, int, int, int, int );
static  void  (*key_up_callback) ( Window_id, int, int, int, int );
static  void  (*mouse_motion_callback) ( Window_id, int, int );
static  void  (*left_down_callback) ( Window_id, int, int, int );
static  void  (*left_up_callback) ( Window_id, int, int, int );
static  void  (*middle_down_callback) ( Window_id, int, int, int );
static  void  (*middle_up_callback) ( Window_id, int, int, int );
static  void  (*right_down_callback) ( Window_id, int, int, int );
static  void  (*right_up_callback) ( Window_id, int, int, int );
static  void  (*iconify_callback) ( Window_id );
static  void  (*deiconify_callback) ( Window_id );
static  void  (*enter_callback) ( Window_id );
static  void  (*leave_callback) ( Window_id );
static  void  (*quit_callback) ( Window_id );

  void  WS_set_update_function(
    void  (*func)( Window_id ) )
{
    display_callback = func;
}

  void  WS_set_update_overlay_function(
    void  (*func)( Window_id ) )
{
    display_overlay_callback = func;
}

  void  WS_set_resize_function(
    void  (*func)( Window_id, int, int, int, int ) )
{
    resize_callback = func;
}

  void  WS_set_key_down_function(
    void  (*func)( Window_id, int, int, int, int ) )
{
    key_down_callback = func;
}

  void  WS_set_key_up_function(
    void  (*func)( Window_id, int, int, int, int ) )
{
    key_up_callback = func;
}

  void  WS_set_mouse_movement_function(
    void  (*func)( Window_id, int, int ) )
{
    mouse_motion_callback = func;
}

  void  WS_set_left_mouse_down_function(
    void  (*func)( Window_id, int, int, int ) )
{
    left_down_callback = func;
}

  void  WS_set_left_mouse_up_function(
    void  (*func)( Window_id, int, int, int ) )
{
    left_up_callback = func;
}

  void  WS_set_middle_mouse_down_function(
    void  (*func)( Window_id, int, int, int ) )
{
    middle_down_callback = func;
}

  void  WS_set_middle_mouse_up_function(
    void  (*func)( Window_id, int, int, int ) )
{
    middle_up_callback = func;
}

  void  WS_set_right_mouse_down_function(
    void  (*func)( Window_id, int, int, int ) )
{
    right_down_callback = func;
}

  void  WS_set_right_mouse_up_function(
    void  (*func)( Window_id, int, int, int ) )
{
    right_up_callback = func;
}

  void  WS_set_iconify_function(
    void  (*func)( Window_id ) )
{
    iconify_callback = func;
}

  void  WS_set_deiconify_function(
    void  (*func)( Window_id ) )
{
    deiconify_callback = func;
}

  void  WS_set_enter_function(
    void  (*func)( Window_id ) )
{
    enter_callback = func;
}

  void  WS_set_leave_function(
    void  (*func)( Window_id ) )
{
    leave_callback = func;
}

  void  WS_set_quit_function(
    void  (*func)( Window_id ) )
{
    quit_callback = func;
}

static  int  get_keyboard_modifiers( void )
{
    int  modifier, glut_mod;

    modifier = 0;
    glut_mod = glutGetModifiers();

    if( (glut_mod & GLUT_ACTIVE_SHIFT) != 0 )
        modifier |= SHIFT_KEY_BIT;
    if( (glut_mod & GLUT_ACTIVE_CTRL) != 0 )
        modifier |= CTRL_KEY_BIT;
    if( (glut_mod & GLUT_ACTIVE_ALT) != 0 )
        modifier |= ALT_KEY_BIT;

    return( modifier );
}

static  int  flip_window_y(
    int   window_y )
{
    return( glutGet( (GLenum) GLUT_WINDOW_HEIGHT ) - 1 - window_y );
}

static  int  flip_screen_y(
    int   screen_y )
{
    return( glutGet( (GLenum) GLUT_SCREEN_HEIGHT ) - 1 - screen_y );
}

static  void  display_function( void )
{
    int   i, save_window_id;

    if( n_windows_to_delete > 0 )
    {
        /*--- glut seems to reset the current window, even if it is not
              deleted */

        save_window_id = glutGetWindow();

        for_less( i, 0, n_windows_to_delete )
        {
            if( windows_to_delete[i] == save_window_id )
                save_window_id = 0;
            delete_GLUT_window( windows_to_delete[i] );
        }

        FREE( windows_to_delete );
        n_windows_to_delete = 0;

        if( save_window_id >= 1 )
            glutSetWindow( save_window_id );
    }

    (*display_callback) ( get_current_event_window() );
}

static  void  display_overlay_function( void )
{
    (*display_overlay_callback) ( get_current_event_window() );
}

static  void  resize_function(
    int   width,
    int   height )
{
    Window_id  window_id;
    int        x, y;

    x = glutGet( (GLenum) GLUT_WINDOW_X );
    y = glutGet( (GLenum) GLUT_WINDOW_Y );

    window_id = get_current_event_window();

    (*resize_callback) ( window_id, x, y, width, height );
}

static  void  keyboard_function(
    unsigned  char  key,
    int             x,
    int             y )
{
    y = flip_window_y( y );
    (*key_down_callback) ( get_current_event_window(), (int) key, x, y,
                           get_keyboard_modifiers() );
}

static  void  special_keyboard_function(
    int     key,
    int     x,
    int     y )
{
    int   translated;

    y = flip_window_y( y );

    translated = -1000;

    switch( key )
    {
    case GLUT_KEY_LEFT:     translated = LEFT_ARROW_KEY;  break;
    case GLUT_KEY_RIGHT:    translated = RIGHT_ARROW_KEY;  break;
    case GLUT_KEY_UP:       translated = UP_ARROW_KEY;  break;
    case GLUT_KEY_DOWN:     translated = DOWN_ARROW_KEY;  break;
    }

    if( translated != -1000 )
        (*key_down_callback) ( get_current_event_window(), translated,
                               x, y, get_keyboard_modifiers() );
}

static  void  mouse_button_function(
    int     button,
    int     state,
    int     x,
    int     y )
{
    int         modifiers;
    Window_id   window_id;

    window_id = get_current_event_window();
    modifiers = get_keyboard_modifiers();
    y = flip_window_y( y );

    switch( button )
    {
    case GLUT_LEFT_BUTTON:
        if( state == GLUT_DOWN )
            (*left_down_callback) ( window_id, x, y, modifiers );
        else
            (*left_up_callback) ( window_id, x, y, modifiers );
        break;

    case GLUT_MIDDLE_BUTTON:
        if( state == GLUT_DOWN )
            (*middle_down_callback) ( window_id, x, y, modifiers );
        else
            (*middle_up_callback) ( window_id, x, y, modifiers );
        break;

    case GLUT_RIGHT_BUTTON:
        if( state == GLUT_DOWN )
            (*right_down_callback) ( window_id, x, y, modifiers );
        else
            (*right_up_callback) ( window_id, x, y, modifiers );
        break;
    }
}

static  void  mouse_motion_function(
    int     x,
    int     y )
{
    y = flip_window_y(y);
    (*mouse_motion_callback) ( get_current_event_window(), x, y );
}

static  void  entry_function(
    int     state )
{
    Window_id   window_id;

    window_id = get_current_event_window();

    if( state == GLUT_LEFT )
        (*leave_callback) ( window_id );
    else
        (*enter_callback) ( window_id );
}

static  void  set_event_callbacks_for_current_window(
    int        n_overlay_planes )
{
    glutDisplayFunc( display_function );
    glutReshapeFunc( resize_function );

    if( n_overlay_planes > 0 )
        glutOverlayDisplayFunc( display_overlay_function );

    glutKeyboardFunc( keyboard_function );
    glutSpecialFunc( special_keyboard_function );
    glutMouseFunc( mouse_button_function );
    glutMotionFunc( mouse_motion_function );
    glutPassiveMotionFunc( mouse_motion_function );
    glutEntryFunc( entry_function );
}

typedef struct
{
    void  (*function) ( void * );
    void              *data;
    VIO_BOOL           active;
} callback_info_struct;

static  callback_info_struct   *timers;
static  int                    n_timers = 0;

static  void  global_timer_function(
    int   index )
{
    if( index < 0 || index >= n_timers )
    {
        handle_internal_error( "global_timer_function" );
        return;
    }

    (*timers[index].function)( timers[index].data );

    timers[index].active = FALSE;
}

  void  WS_add_timer_function(
    VIO_Real          seconds,
    void          (*func) ( void * ),
    void          *data )
{
    int                   i;

    for_less( i, 0, n_timers )
    {
        if( !timers[i].active )
            break;
    }

    if( i >= n_timers )
    {
        SET_ARRAY_SIZE( timers, n_timers, n_timers+1, 1 );
        ++n_timers;
    }

    timers[i].function = func;
    timers[i].data = data;
    timers[i].active = TRUE;

    glutTimerFunc( (unsigned int) (1000.0 * seconds + 0.5),
                   global_timer_function, i );
}

static  callback_info_struct   *idles;
static  int                    n_idles = 0;

static  void  global_idle_function( void )
{
    int   i;

    for_less( i, 0, n_idles )
    {
        (*idles[i].function) ( idles[i].data );
    }
}

  void  WS_add_idle_function(
    void  (*func) ( void * ),
    void          *data )
{
    callback_info_struct  info;

    if( n_idles == 0 )
        glutIdleFunc( global_idle_function );

    info.function = func;
    info.data = data;

    ADD_ELEMENT_TO_ARRAY( idles, n_idles, info, 1 );
}

  void  WS_remove_idle_function(
    void  (*func) ( void * ),
    void          *data )
{
    int   i;

    for_less( i, 0, n_idles )
    {
        if( idles[i].function == func && idles[i].data == data )
            break;
    }

    if( i >= n_idles )
    {
        handle_internal_error( "WS_delete_idle_func" );
        return;
    }

    DELETE_ELEMENT_FROM_ARRAY( idles, n_idles, i, 1 );

    if( n_idles == 0 )
        glutIdleFunc( NULL );
}

  void  WS_event_loop( void )
{
    glutMainLoop();
}

  void  WS_set_update_flag(
    WSwindow   window  )
{
    glutPostRedisplay();
}
