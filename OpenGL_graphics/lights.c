 
#include  <internal_volume_io.h>
#include  <gs_specific.h>

/* ARGSUSED */

public  void  GS_initialize_lights(
    Gwindow        window )
{
    static  float ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_light );
    glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE );
    glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );

}

/* ARGSUSED */

public  void  GS_set_ambient_light(
    Gwindow        window,
    Colour         colour )
{
#ifndef  TWO_D_ONLY
    float     ambient_light[4];

    ambient_light[0] = (float) get_Colour_r_0_1(colour);
    ambient_light[1] = (float) get_Colour_g_0_1(colour);
    ambient_light[2] = (float) get_Colour_b_0_1(colour);
    ambient_light[3] = (float) get_Colour_a_0_1(colour);

    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_light );
#endif
}

/* ARGSUSED */

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
    int     gl_light_index;
    Vector  unit_direction;
    float   colour_list[4];
    float   position_list[4];
    static  float  null_colour[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    colour_list[0] = (float) get_Colour_r_0_1(colour);
    colour_list[1] = (float) get_Colour_g_0_1(colour);
    colour_list[2] = (float) get_Colour_b_0_1(colour);
    colour_list[3] = (float) get_Colour_a_0_1(colour);

    gl_light_index = GL_LIGHT0 + light_index;

    switch( type )
    {
    case DIRECTIONAL_LIGHT:
        glLightfv( (GLenum) gl_light_index, GL_AMBIENT, null_colour );
        glLightfv( (GLenum) gl_light_index, GL_DIFFUSE, colour_list );
        glLightfv( (GLenum) gl_light_index, GL_SPECULAR, colour_list );
        NORMALIZE_VECTOR( unit_direction, *direction );
        position_list[0] = -Vector_x(unit_direction);
        position_list[1] = -Vector_y(unit_direction);
        position_list[2] = -Vector_z(unit_direction);
        position_list[3] = 0.0f;
        glLightfv( (GLenum) gl_light_index, GL_POSITION, position_list );
        break;

    case POINT_LIGHT:
        glLightfv( (GLenum) gl_light_index, GL_AMBIENT, null_colour );
        glLightfv( (GLenum) gl_light_index, GL_DIFFUSE, colour_list );
        glLightfv( (GLenum) gl_light_index, GL_SPECULAR, colour_list );
        position_list[0] = Point_x(*position);
        position_list[1] = Point_y(*position);
        position_list[2] = Point_z(*position);
        position_list[3] = 0.0f;
        glLightfv( (GLenum) gl_light_index, GL_POSITION, position_list );
        break;

    case SPOT_LIGHT:
        glLightfv( (GLenum) gl_light_index, GL_AMBIENT, null_colour );
        glLightfv( (GLenum) gl_light_index, GL_DIFFUSE, colour_list );
        glLightfv( (GLenum) gl_light_index, GL_SPECULAR, colour_list );

        position_list[0] = Point_x(*position);
        position_list[1] = Point_y(*position);
        position_list[2] = Point_z(*position);
        position_list[3] = 1.0f;
        glLightfv( (GLenum) gl_light_index, GL_POSITION, position_list );

        NORMALIZE_VECTOR( unit_direction, *direction );
        position_list[0] = Vector_x(unit_direction);
        position_list[1] = Vector_y(unit_direction);
        position_list[2] = Vector_z(unit_direction);
        position_list[3] = 0.0f;
        glLightfv( (GLenum) gl_light_index, GL_SPOT_DIRECTION, position_list );

        glLightf( (GLenum) gl_light_index, GL_SPOT_EXPONENT,
                  (GLfloat) spot_exponent );

        glLightf( (GLenum) gl_light_index, GL_SPOT_CUTOFF,
                  (GLfloat) spot_angle );
        break;
    }
#endif
}

/* ARGSUSED */

public  void  GS_set_light_state(
    Gwindow         window,
    int             light_index,
    BOOLEAN         state )
{
#ifndef  TWO_D_ONLY
    short   gl_light_index;

    switch( light_index )
    {
    case 0:  gl_light_index = GL_LIGHT0;    break;
    case 1:  gl_light_index = GL_LIGHT1;    break;
    case 2:  gl_light_index = GL_LIGHT2;    break;
    case 3:  gl_light_index = GL_LIGHT3;    break;
    case 4:  gl_light_index = GL_LIGHT4;    break;
    case 5:  gl_light_index = GL_LIGHT5;    break;
    case 6:  gl_light_index = GL_LIGHT6;    break;
    case 7:  gl_light_index = GL_LIGHT7;    break;
    }

    if( state )
        glEnable( (GLenum) gl_light_index );
    else
        glDisable( (GLenum) gl_light_index );
#endif
}
