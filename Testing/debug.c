
#include  <internal_volume_io.h>
#include  <graphics.h>


int main(
    int    argc,
    char   *argv[] )
{
    Status            status;
    Gwindow           window;

    status = G_create_window( "Test Window",
                              100, 600, 300, 300,
                              FALSE, TRUE, TRUE, 0, &window );

    print( "\n" );
    test_glut_windows();

    print( "\n" );
    test_glut_windows();

    print( "\n" );
    test_glut_windows();

    print( "\n" );
    test_glut_windows();

    print( "\n" );
    test_glut_windows();

    G_main_loop();

    return( 0 );
}
