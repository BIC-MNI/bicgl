#include  <internal_volume_io.h>

extern  void  glut_set_colour_entry(
    int     ind,
    Real    r,
    Real    g,
    Real    b );

public  void  copy_X_colours(
    int  n_colours_to_copy )
{
    int     ind;
    char    command[1000];
    FILE    *file;
    Real    r, g, b;

    (void) sprintf( command, "print_X_colours %d", n_colours_to_copy );

    file = popen( command, "r" );

    ind = 0;

    while( input_real( file, &r ) == OK &&
           input_real( file, &g ) == OK &&
           input_real( file, &b ) == OK )
    {
        glut_set_colour_entry( ind, r, g, b );

        ++ind;
    }

    (void) pclose( file );
}
