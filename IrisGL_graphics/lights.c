 
#include  <internal_volume_io.h>
#include  <gs_specific.h>

public  void   GS_initialize_lights(
    Gwindow  window )
{
    static   float    data[] = { AMBIENT, 1.0, 1.0, 1.0,
                                 LOCALVIEWER, 0.0, TWOSIDE, 1.0, LMNULL };

    lmdef( DEFLMODEL, window->GS_window->unique_lmdef_id,
           SIZEOF_STATIC_ARRAY(data), data );

    lmbind( LMODEL, window->GS_window->unique_lmdef_id );

    lmbind( LIGHT0, 0 );
    lmbind( LIGHT1, 0 );
    lmbind( LIGHT2, 0 );
    lmbind( LIGHT3, 0 );
    lmbind( LIGHT4, 0 );
    lmbind( LIGHT5, 0 );
    lmbind( LIGHT6, 0 );
    lmbind( LIGHT7, 0 );
}

public  void  GS_set_ambient_light(
    Gwindow        window,
    Colour         colour )
{
#ifndef  TWO_D_ONLY
    static   float    data[] = { AMBIENT, 0.0, 0.0, 0.0, LMNULL };

    data[1] = get_Colour_r_0_1(colour);
    data[2] = get_Colour_g_0_1(colour);
    data[3] = get_Colour_b_0_1(colour);

    lmdef( DEFLMODEL, window->GS_window->unique_lmdef_id,
           SIZEOF_STATIC_ARRAY(data), data );
#endif
}

public  void  GS_define_light(
    Gwindow         window,
    int             light_index,
    Light_types     type,
    Colour          colour,
    Vector          *direction,
    Point           *position,
    Real            spot_exponent,
    Real            spot_angle )
{
#ifndef  TWO_D_ONLY
    Vector  unit_direction;
    int     n_data;
    float   data[25];

    n_data = 0;

    switch( type )
    {
    case DIRECTIONAL_LIGHT:
        NORMALIZE_VECTOR( unit_direction, *direction );
        data[n_data++] = AMBIENT;
        data[n_data++] = 0.0;
        data[n_data++] = 0.0;
        data[n_data++] = 0.0;
        data[n_data++] = LCOLOR;
        data[n_data++] = get_Colour_r_0_1(colour);
        data[n_data++] = get_Colour_g_0_1(colour);
        data[n_data++] = get_Colour_b_0_1(colour);
        data[n_data++] = POSITION;
        data[n_data++] = -Vector_x(unit_direction);
        data[n_data++] = -Vector_y(unit_direction);
        data[n_data++] = -Vector_z(unit_direction);
        data[n_data++] = 0.0;
        data[n_data++] = LMNULL;
        break;

    case POINT_LIGHT:
        data[n_data++] = AMBIENT;
        data[n_data++] = 0.0;
        data[n_data++] = 0.0;
        data[n_data++] = 0.0;
        data[n_data++] = LCOLOR;
        data[n_data++] = get_Colour_r_0_1(colour);
        data[n_data++] = get_Colour_g_0_1(colour);
        data[n_data++] = get_Colour_b_0_1(colour);
        data[n_data++] = POSITION;
        data[n_data++] = Point_x(*position);
        data[n_data++] = Point_y(*position);
        data[n_data++] = Point_z(*position);
        data[n_data++] = 1.0;
        data[n_data++] = LMNULL;
        break;

    case SPOT_LIGHT:
        NORMALIZE_VECTOR( unit_direction, *direction );
        data[n_data++] = AMBIENT;
        data[n_data++] = 0.0;
        data[n_data++] = 0.0;
        data[n_data++] = 0.0;
        data[n_data++] = LCOLOR;
        data[n_data++] = get_Colour_r_0_1(colour);
        data[n_data++] = get_Colour_g_0_1(colour);
        data[n_data++] = get_Colour_b_0_1(colour);
        data[n_data++] = POSITION;
        data[n_data++] = Point_x(*position);
        data[n_data++] = Point_y(*position);
        data[n_data++] = Point_z(*position);
        data[n_data++] = SPOTLIGHT;
        data[n_data++] = spot_exponent;
        data[n_data++] = spot_angle;
        data[n_data++] = SPOTDIRECTION;
        data[n_data++] = Vector_x(unit_direction);
        data[n_data++] = Vector_y(unit_direction);
        data[n_data++] = Vector_z(unit_direction);
        data[n_data++] = LMNULL;
        break;
    }

    lmdef( DEFLIGHT, window->GS_window->unique_lmdef_id + light_index,
           n_data, data );
#endif
}

public  void  GS_set_light_state(
    Gwindow         window,
    int             light_index,
    BOOLEAN         state )
{
#ifndef  TWO_D_ONLY
    short   gl_light_index;

    switch( light_index )
    {
    case 0:  gl_light_index = LIGHT0;    break;
    case 1:  gl_light_index = LIGHT1;    break;
    case 2:  gl_light_index = LIGHT2;    break;
    case 3:  gl_light_index = LIGHT3;    break;
    case 4:  gl_light_index = LIGHT4;    break;
    case 5:  gl_light_index = LIGHT5;    break;
    case 6:  gl_light_index = LIGHT6;    break;
    case 7:  gl_light_index = LIGHT7;    break;
    }

    if( state )
        lmbind( gl_light_index,
                window->GS_window->unique_lmdef_id+light_index );
    else
        lmbind( gl_light_index, 0 );
#endif
}
