#include  <internal_volume_io.h>
#include  <objects.h>

public  Colour  make_rgba_Colour(
    int    r,
    int    g,
    int    b,
    int    a )
{
    return( (Colour) a |
            ((Colour) b << (Colour) 8) |
            ((Colour) g << (Colour) 16) |
            ((Colour) r << (Colour) 24) );
}

public  int  get_Colour_a(
    Colour   colour )
{
    return( colour & 255ul );
}

public  int  get_Colour_b(
    Colour   colour )
{
    return( (colour >> 8ul) & 255ul );
}

public  int  get_Colour_g(
    Colour   colour )
{
    return( (colour >> 16ul) & 255ul );
}

public  int  get_Colour_r(
    Colour   colour )
{
    return( colour >> 24ul );
}
