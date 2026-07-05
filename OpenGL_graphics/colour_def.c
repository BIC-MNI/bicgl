#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H  

#include  <volume_io.h>

  VIO_Colour  make_rgba_Colour(
    int    r,
    int    g,
    int    b,
    int    a )
{
    VIO_Colour          c;
    unsigned  char  *byte_ptr;

    c = 0;         /* to avoid used-before-set compiler messages */

    byte_ptr = (unsigned char *) &c;

    byte_ptr[3] = (unsigned char) a;
    byte_ptr[2] = (unsigned char) b;
    byte_ptr[1] = (unsigned char) g;
    byte_ptr[0] = (unsigned char) r;

    return( c );
}

  int  get_Colour_r(
    VIO_Colour   colour )
{
    unsigned  char  *b;

    b = (unsigned char *) &colour;

    return( (int) b[0] );
}

  int  get_Colour_g(
    VIO_Colour   colour )
{
    unsigned  char  *b;

    b = (unsigned char *) &colour;

    return( (int) b[1] );
}

  int  get_Colour_b(
    VIO_Colour   colour )
{
    unsigned  char  *b;

    b = (unsigned char *) &colour;

    return( (int) b[2] );
}

  int  get_Colour_a(
    VIO_Colour   colour )
{
    unsigned  char  *b;

    b = (unsigned char *) &colour;

    return( (int) b[3] );
}

/* The [0,1]-range wrapper functions below only exist as compiled code
 * inside libminc2's shared library (volume_io has no static archive in
 * this build) -- so on macOS, calls to them from statically-linked code
 * (e.g. bicpl's get_colour_code()) bind, via Mach-O two-level namespace
 * rules, to *that dylib's own* internal calls to make_rgba_Colour()/
 * get_Colour_r/g/b/a(), which were fixed at the dylib's build time
 * (before bicgl existed) to volume_io's generic byte order -- silently
 * bypassing the override above. (On Linux/ELF this isn't an issue: ELF's
 * default symbol interposition lets the override reach across shared
 * library boundaries, which is why this was never seen there.)
 * Overriding these wrapper functions here too, in the same translation
 * unit as the int-based override above, makes the linker satisfy them
 * statically instead, avoiding the dylib entirely. */

  VIO_Colour  make_rgba_Colour_0_1(
    VIO_Real   r,
    VIO_Real   g,
    VIO_Real   b,
    VIO_Real   a )
{
    return( make_rgba_Colour( (int) (r * 255.0 + 0.5),
                               (int) (g * 255.0 + 0.5),
                               (int) (b * 255.0 + 0.5),
                               (int) (a * 255.0 + 0.5) ) );
}

  VIO_Real  get_Colour_r_0_1(
    VIO_Colour   colour )
{
    return( (VIO_Real) get_Colour_r(colour) / 255.0 );
}

  VIO_Real  get_Colour_g_0_1(
    VIO_Colour   colour )
{
    return( (VIO_Real) get_Colour_g(colour) / 255.0 );
}

  VIO_Real  get_Colour_b_0_1(
    VIO_Colour   colour )
{
    return( (VIO_Real) get_Colour_b(colour) / 255.0 );
}

  VIO_Real  get_Colour_a_0_1(
    VIO_Colour   colour )
{
    return( (VIO_Real) get_Colour_a(colour) / 255.0 );
}

/* Same dylib-boundary issue as above, for the two remaining functions from
 * volume_io/Geometry/colour.c: all of bicpl's named UI colour constants
 * (bicpl/Include/bicpl/colours.h, e.g. DARK_SLATE_BLUE) are built via
 * make_Colour_0_1(), which -- unoverridden -- calls make_Colour() and then
 * make_rgba_Colour() entirely within libminc2.dylib, using its frozen
 * generic byte order regardless of the overrides above. */

  VIO_Colour  make_Colour(
    int   r,
    int   g,
    int   b )
{
    return( make_rgba_Colour( r, g, b, 255 ) );
}

  VIO_Colour  make_Colour_0_1(
    VIO_Real   r,
    VIO_Real   g,
    VIO_Real   b )
{
    return( make_Colour( (int) (r * 255.0 + 0.5),
                          (int) (g * 255.0 + 0.5),
                          (int) (b * 255.0 + 0.5) ) );
}
