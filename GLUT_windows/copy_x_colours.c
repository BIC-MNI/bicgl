#include  <internal_volume_io.h>
#include  <X11/Xlib.h>

extern  void  glut_set_colour_entry(
    int      ind,
    float    r,
    float    g,
    float    b );

public  void  copy_X_colours(
    int  n_colours_to_copy )
{
    Display            *display;
    int                screen;
    Window             root_window;
    XWindowAttributes  attrib;
    Colormap           cmap;
    XColor             def;
    int                ind;

    display = XOpenDisplay( 0 );

    if( display == NULL )
    {
        print_error( "Could not open X display\n" );
        return;
    }

    screen = DefaultScreen( display );

    root_window =  RootWindow( display, screen ),

    XGetWindowAttributes( display, root_window, &attrib );

    cmap = attrib.colormap;

    for_less( ind, 0, n_colours_to_copy )
    {
        def.pixel = ind;
        XQueryColor( display, cmap, &def );
        glut_set_colour_entry( ind, (float) def.red / (float) 65535.0,
                                    (float) def.green / (float) 65535.0,
                                    (float) def.blue / (float) 65535.0 );

    }

    XCloseDisplay( display );
}
