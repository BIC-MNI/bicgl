
#include  <internal_volume_io.h>
#include  <graphics.h>

int main(
    int    argc,
    char   *argv[] )
{
    Gwindow  first, second;

    (void) G_create_window( "Test Window",
                              100, 600, 300, 300,
                              FALSE, TRUE, TRUE, 0,
                              &first );

    (void) G_create_window( "Second Window",
                            500, 600, 200, 200,
                            TRUE, TRUE, TRUE, 0, &second );

    G_main_loop();

    return( 0 );
}
