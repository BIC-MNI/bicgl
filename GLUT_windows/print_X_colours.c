#include  <X11/Xlib.h>
#include  <stdio.h>

int  main(
    int  argc,
    char *argv[] )
{
    Display            *display;
    int                screen;
    Window             root_window;
    XWindowAttributes  attrib;
    Colormap           cmap;
    XColor             def;
    int                ind, n_colours;

    if( argc < 2 || sscanf( argv[1], "%d", &n_colours ) != 1 ||
        n_colours < 0 )
    {
        n_colours = 32;
    }

    display = XOpenDisplay( 0 );

    if( display == NULL )
    {
        (void) fprintf( stderr, "%s could not open X display\n", argv[0] );
        return( 1 );
    }

    screen = DefaultScreen( display );

    root_window =  RootWindow( display, screen ),

    XGetWindowAttributes( display, root_window, &attrib );

    cmap = attrib.colormap;

    for( ind = 0;  ind < n_colours;  ++ind )
    {
        def.pixel = ind;
        XQueryColor( display, cmap, &def );
        (void) printf( "%g %g %g\n",
                           (float) def.red / (float) 65535.0,
                           (float) def.green / (float) 65535.0,
                           (float) def.blue / (float) 65535.0 );

    }

    XCloseDisplay( display );

    return( 0 );
}
