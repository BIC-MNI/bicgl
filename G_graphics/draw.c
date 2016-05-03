#define GL_GLEXT_PROTOTYPES 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H

#include  <volume_io.h>
#include  <graphics.h>
#include  <random_order.h>

#include <assert.h>

/**
 * Sets the amount of expansion in the model when drawing the wireframe
 * overlay on top of the shaded surface.
 */
#define MX_OVERLAY 0.9999

#define DISALLOW_DRAWING_INTERRUPT 1
#define PARANOID 1

/* #define DEBUG 1 */

#if PARANOID
#define GLCHECK {GLint _temp = glGetError(); if (_temp != GL_NO_ERROR) { fprintf(stderr, "OpenGL reporting error 0x%x at line %d.\n", _temp, __LINE__); } }
#else
#define GLCHECK
#endif

#define  MAX_LINE_WIDTH  1000.0f

static  void     draw_marker_as_cube( Gwindow, VIO_Colour );

static  void  set_colour(
    Gwindow   window,
    VIO_Colour    colour )
{
     if( window->colour_map_state ||
         window->current_bitplanes == OVERLAY_PLANES )
         GS_set_colour_index( colour );
     /*else if( stereo_flag )
     {
         int  intensity;
         intensity = 0.324 * get_Colour_r(c) +
                     0.499 * get_Colour_g(c) +
                     0.177 * get_Colour_b(c) + 0.5;
         cpack( make_Colour( intensity, intensity, intensity ) );
     } */
     else
         GS_set_colour( colour );
}

  void  initialize_display_interrupts(
    Gwindow        window )
{
    window->interrupt_allowed = FALSE;
    window->continuation_flag = FALSE;
    window->interrupt_occurred = FALSE;
    window->interrupt_interval = 100;
    window->interrupt_time_interval = 0.3;
}

  void  G_set_drawing_interrupt_state(
    Gwindow          window,
    VIO_BOOL         state )
{
    window->interrupt_allowed = state;
}

  VIO_BOOL  G_get_drawing_interrupt_state(
    Gwindow          window )
{
    return( window->interrupt_allowed );
}

  void  G_set_drawing_interrupt_check_n_objects(
    Gwindow          window,
    int             interval )
{
    window->interrupt_interval = interval;
}

  void  G_set_drawing_interrupt_time(
    Gwindow          window,
    VIO_Real             interval )
{
    window->interrupt_time_interval = interval;
}

  void  G_start_interrupt_test(
    Gwindow          window )
{
    VIO_Real   current_time;

    current_time = current_realtime_seconds();
    window->interrupt_time = current_time + window->interrupt_time_interval;
}

  VIO_BOOL  G_get_interrupt_occurred(
    Gwindow          window )
{
    return( window->interrupt_occurred );
}

  void  G_clear_drawing_interrupt_flag(
    Gwindow          window )
{
    window->interrupt_occurred = FALSE;
}

  void  set_continuation_flag(
    Gwindow         window,
    VIO_BOOL         state )
{
    window->continuation_flag = state;
}

#ifndef   DISALLOW_DRAWING_INTERRUPT

/*  macro for drawing objects with interrupts  */

#define  BEGIN_DRAW_OBJECTS( window, check_every, n_objects, wireframe_flag ) \
{ \
    Random_mask_type            OBJECT_INDEX; \
    int                         _i, _iter, _n_iters, _n_done, _n_before_check; \
    int                         _check_every, _n_objects_to_do; \
    VIO_BOOL                     _random_order, _interrupt_allowed; \
    Random_mask_type            _random_order_mask; \
    VIO_Real                        interrupt_at, current_time; \
    int                         _n_objects, _first_object, _second_object; \
    VIO_BOOL                     _wireframe_flag; \
    static   VIO_BOOL            const_true = TRUE; \
 \
    _n_objects = (n_objects); \
    _wireframe_flag = (wireframe_flag); \
 \
    (window)->interrupt_occurred = FALSE; \
\
    interrupt_at = (window)->interrupt_time; \
    _interrupt_allowed = (G_get_drawing_interrupt_state( window ) && \
                         (_n_objects) > (window)->interrupt_interval ); \
 \
    if( _interrupt_allowed ) \
    { \
        _random_order = (window->zbuffer_state || \
                         const_true == _wireframe_flag); \
        if( _random_order ) \
            _random_order_mask = get_random_order_mask( _n_objects ); \
 \
        _check_every = (check_every); \
        _n_before_check = _check_every; \
    } \
    else \
        _random_order = FALSE; \
   \
    if( (window)->continuation_flag ) \
    { \
        _first_object = (window)->next_item; \
        _n_done = (window)->n_items_done;   \
        (window)->continuation_flag = FALSE;            \
    } \
    else \
    { \
        _first_object = 0; \
        _n_done = 0; \
    } \
 \
    if( view_is_stereo(window) )                                            \
    {                                                                         \
        _n_iters = 2;                                                         \
        set_view_for_eye( window, 0 );                                        \
    }                                                                         \
    else                                                                      \
        _n_iters = 1;                                                         \
                                                                              \
    _n_objects_to_do = _n_objects;                                            \
    if( _n_objects > 1 )                                                      \
        _second_object = get_random_int( _n_objects-1 ) + 1;                  \
    else                                                                      \
        _second_object = 0;                                                   \
                                                                              \
    for_less( _iter, 0, _n_iters )                                            \
    {                                                                         \
        OBJECT_INDEX = (Random_mask_type) _first_object;                      \
                                                                              \
        if( _iter == 1 )                                                      \
        {                                                                     \
            set_view_for_eye( window, 1 );                                    \
            _interrupt_allowed = FALSE;                                       \
        }                                                                     \
                                                                              \
        for_less( _i, _n_done, _n_objects_to_do ) \
        {


#define  END_DRAW_OBJECTS  \
            if( _random_order ) \
            { \
                if( _i == 0 ) \
                { \
                    OBJECT_INDEX = (Random_mask_type) _second_object; \
                } \
                else \
                { \
                    do \
                    { \
                        INCREMENT_RANDOM_ORDER( OBJECT_INDEX, \
                                                _random_order_mask ); \
                    } while( OBJECT_INDEX >= (Random_mask_type) _n_objects ); \
                } \
            } \
            else \
            { \
                ++OBJECT_INDEX; \
            } \
 \
            if( _interrupt_allowed ) \
            { \
                --_n_before_check; \
                if( _n_before_check == 0 ) \
                { \
                    current_time = current_realtime_seconds(); \
                    if( current_time >= interrupt_at ) \
                    { \
                        _n_objects_to_do = _i+1; \
                        (window)->n_items_done = _n_objects_to_do; \
                        (window)->next_item = (int) OBJECT_INDEX; \
                        (window)->interrupt_occurred = TRUE; \
                        break; \
                    } \
                    _n_before_check = _check_every; \
                } \
            } \
        } \
    } \
\
    if( view_is_stereo(window) ) \
    { \
        set_view_for_eye( window, -1 );                                        \
    } \
}

#else

#define  BEGIN_DRAW_OBJECTS( window, check_every, n_objects, wireframe_flag )\
{                                                                             \
    int                      _i, _n_iters, OBJECT_INDEX;                      \
                                                                              \
    if( view_is_stereo(window) )                                            \
    {                                                                         \
        _n_iters = 2;                                                         \
        set_view_for_eye( window, 0 );                                        \
    }                                                                         \
    else                                                                      \
        _n_iters = 1;                                                         \
                                                                              \
    for_less( _i, 0, _n_iters )                                               \
    {                                                                         \
        if( _i == 1 )                                                         \
        {                                                                     \
            set_view_for_eye( window, 1 );                                    \
        }                                                                     \
                                                                              \
        for_less( OBJECT_INDEX, 0, n_objects )                                \
        {

#define  END_DRAW_OBJECTS                                                     \
        }                                                                     \
    }                                                                         \
    if( view_is_stereo(window) )                                              \
    {                                                                         \
        set_view_for_eye( window, -1 );                                       \
    }                                                                         \
}

#endif

static  void  set_surface_property(
    Gwindow        window,
    VIO_Colour         col,
    VIO_Surfprop       *surfprop )
{
    GS_set_surface_property( window->GS_window, col, surfprop );
}

  VIO_BOOL  view_is_stereo(
    Gwindow        window )
{
    return( window->stereo_flag &&
            !window->colour_map_state &&
            window->current_bitplanes == NORMAL_PLANES &&
            (window->current_view_type == WORLD_VIEW ||
             window->current_view_type == MODEL_VIEW) );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : about_to_draw_graphics
@INPUT      : window
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Called just before drawing into a window, to set the current
              window, and clear it, if automatic clearing is enabled.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

static  void  about_to_draw_graphics( Gwindow        window )
{
    set_current_window( window );
    check_window_cleared( window );
}

#define N_DIM 3
#define N_VTX_PER_TRIANGLE 3
/**
 * Check whether a polygons_struct is (most likely) a triangular mesh.
 */
static VIO_BOOL
is_triangular(polygons_struct *polygons)
{
  int start_index = 0;
  int end_index = 0;
  int i;

  for (i = 0; i < polygons->n_items; i++)
  {
    start_index = end_index;
    end_index = polygons->end_indices[i];
    if ((end_index - start_index) != N_VTX_PER_TRIANGLE)
    {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Simple helper function to set the opacity in the fragment shader.
 */
static void set_program_opacity(GLint program, GLfloat opacity)
{
    glUniform1f( glGetUniformLocation( program, "opacity" ), opacity );
}

/**
 * Simple helper function to set the "model expansion factor" in the vertex
 * shader.
 */
static void set_program_mxfactor(GLint program, GLfloat mxfactor)
{
    glUniform1f( glGetUniformLocation( program, "mxfactor" ), mxfactor );
}

static void do_wireframe_overlay( Gwindow window,
                                  polygons_struct *polygons,
                                  VIO_Colour colour)
{
    GLint program = window->GS_window->programs[PROGRAM_SINGLE];
    glUseProgram( program );
    set_program_opacity( program, Surfprop_t( polygons->surfprop ) );
    set_program_mxfactor( program, MX_OVERLAY );
    set_surface_property( window, colour, &polygons->surfprop );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : draw_polygons_one_colour
@INPUT      : window
              polygons
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Draws a set of polygons which has a single colour for all
              polygons.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

/**
 * Draws a polygonal object (non-triangular) using glDrawElements.
 * Supports either single colour or per-item colours. This relies on
 * glDrawElements supporting GL_POLYGON as a mode argument -
 * apparently this is deprecated behavior.
 *
 * \param window The window in which we are drawing.
 * \param polygons The polygons_struct to draw.
 */
static void
draw_polygons_one_colour(Gwindow window, polygons_struct *polygons)
{
  GLuint vbo_points;
  GLuint vbo_normals;
  GLint loc_position;
  GLint loc_normal = -1;
  GLint program;
  int start_index = 0;
  int end_index = 0;
  int i;

#if DEBUG
  printf("dp_one_colour %d %d (normals %p points %d items %d opacity %f)\n",
         window->shaded_mode_state, window->lighting_state,
         polygons->normals, polygons->n_points, polygons->n_items,
         Surfprop_t(polygons->surfprop));
#endif  /* DEBUG */

  if (polygons->normals != NULL)
  {
    /*
     * We have surface normals, so use the standard program for
     * vertex+attribute shading.
     */
    program = window->GS_window->programs[PROGRAM_SINGLE];

    /*
     * Set up nondefault surface properties.
     */
    set_surface_property( window, polygons->colours[0], &polygons->surfprop );
  }
  else
  {
    program = window->GS_window->programs[PROGRAM_TRIVIAL];
  }

  set_colour( window, polygons->colours[0] );
  if( window->shaded_mode_state )
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
  }
  else
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
  }
  glUseProgram(program);

  set_program_opacity(program, Surfprop_t(polygons->surfprop));
  set_program_mxfactor( program, 1.0 );

  glGenBuffers(1, &vbo_points);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_points);
  glBufferData(GL_ARRAY_BUFFER, polygons->n_points * sizeof(VIO_Point),
               polygons->points, GL_STATIC_DRAW);

  loc_position = glGetAttribLocation(program, "position");
  glVertexAttribPointer(loc_position, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(loc_position);
  if (polygons->normals != NULL)
  {
    glGenBuffers(1, &vbo_normals);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
    glBufferData(GL_ARRAY_BUFFER, polygons->n_points * sizeof(VIO_Point),
                 polygons->normals, GL_STATIC_DRAW);

    loc_normal = glGetAttribLocation(program, "normal");
    glVertexAttribPointer(loc_normal, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(loc_normal);
  }

  if (polygons->colour_flag == PER_ITEM_COLOURS)
  {
    for (i = 0; i < polygons->n_items; i++)
    {
      start_index = end_index;
      end_index = polygons->end_indices[i];
      if (polygons->visibilities == NULL || polygons->visibilities[i])
      {
        set_colour(window, polygons->colours[i]);
        if (polygons->normals != NULL)
        {
          set_surface_property( window, polygons->colours[i],
                                &polygons->surfprop);
        }
        glDrawElements(GL_POLYGON, end_index - start_index, GL_UNSIGNED_INT,
                       &polygons->indices[start_index]);
      }
    }
  }
  else
  {
    for (i = 0; i < polygons->n_items; i++)
    {
      start_index = end_index;
      end_index = polygons->end_indices[i];
      if (polygons->visibilities == NULL || polygons->visibilities[i])
      {
        glDrawElements(GL_POLYGON, end_index - start_index, GL_UNSIGNED_INT,
                       &polygons->indices[start_index]);
      }
    }
  }

  /* Clean up.
   */
  if (polygons->normals != NULL)
  {
    glDisableVertexAttribArray(loc_normal);
    glDeleteBuffers(1, &vbo_normals);
  }

  glDisableVertexAttribArray(loc_position);
  glDeleteBuffers(1, &vbo_points);

  glUseProgram(0);
}

/**
 * Choose whether to use black or white to get the best contrast
 * with the given colour.
 */
static VIO_Colour get_contrasting_colour(VIO_Colour colour)
{
    int r = get_Colour_r(colour);
    int g = get_Colour_g(colour);
    int b = get_Colour_b(colour);
    int yiq = ((r * 299) + (g * 587) + (b * 114)) / 1000;
    return (yiq >= 128) ? BLACK : WHITE;
}

/**
 * Draw a triangular mesh in a single colour.
 */
static void
draw_triangles_one_colour(Gwindow window, polygons_struct *polygons)
{
  GLuint vbo_points;
  GLuint vbo_normals;
  GLuint ebo;
  GLint loc_position;
  GLint loc_normal = -1;
  int *eboBuffer = NULL;  /* Temporary storage for visible indices. */
  int n_indices = 0;
  GLenum mode = GL_TRIANGLES;
  GLint program;

#if DEBUG
  printf("dt_one_colour %d %d (normals %p points %d items %d opacity %f)\n",
         window->shaded_mode_state, window->lighting_state,
         polygons->normals, polygons->n_points, polygons->n_items,
         Surfprop_t(polygons->surfprop));
#endif  /* DEBUG */

  n_indices = NUMBER_INDICES(*polygons);

  /* If the visibilities field is set, we need to copy the visible
   * indices to a new buffer. I don't know of any other way to control
   * visibility on a per-polygon basis (perhaps we could set the
   * colour to a transparent value in the fragment shader?).
   */
  if (polygons->visibilities != NULL)
  {
    int i, k = 0;

    eboBuffer = malloc(n_indices * sizeof(int));
    if (eboBuffer == NULL)
    {
      return;
    }

    for (i = 0; i < polygons->n_items; i++)
    {
      if (polygons->visibilities[i])
      {
        int start_index = START_INDEX( polygons->end_indices, i );
        int end_index = polygons->end_indices[i];
        int j;
        for (j = start_index; j < end_index; j++)
        {
          eboBuffer[k++] = polygons->indices[j];
        }
      }
    }
    n_indices = k;
  }

  if (polygons->normals != NULL)
  {
    program = window->GS_window->programs[PROGRAM_SINGLE];

    /*
     * Set up nondefault surface properties.
     */
    set_surface_property( window, polygons->colours[0], &polygons->surfprop );
  }
  else
  {
    program = window->GS_window->programs[PROGRAM_TRIVIAL];
  }

  set_colour( window, polygons->colours[0] );
  GLCHECK;
  switch (window->shaded_mode_state)
  {
  case WIREFRAME:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  case POINT:
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    break;
  default:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  }

  glUseProgram(program);

  set_program_opacity(program, Surfprop_t(polygons->surfprop));
  set_program_mxfactor( program, 1.0 );

  glGenBuffers(1, &vbo_points);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_points);
  glBufferData(GL_ARRAY_BUFFER, polygons->n_points * sizeof(VIO_Point),
               polygons->points, GL_STATIC_DRAW);

  loc_position = glGetAttribLocation(program, "position");
  glVertexAttribPointer(loc_position, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(loc_position);

  if (polygons->normals != NULL)
  {
    glGenBuffers(1, &vbo_normals);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
    glBufferData(GL_ARRAY_BUFFER, polygons->n_points * sizeof(VIO_Point),
                 polygons->normals, GL_STATIC_DRAW);

    loc_normal = glGetAttribLocation(program, "normal");
    glVertexAttribPointer(loc_normal, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(loc_normal);
  }

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               n_indices * sizeof(int),
               (eboBuffer != NULL) ? eboBuffer : polygons->indices,
               GL_STATIC_DRAW);

  glDrawElements(mode, n_indices, GL_UNSIGNED_INT, 0);
  if (window->shaded_mode_state == OVERLAY)
  {
    do_wireframe_overlay( window, polygons,
                          get_contrasting_colour(polygons->colours[0]) );
    glDrawElements(mode, n_indices, GL_UNSIGNED_INT, 0);
  }

  glDeleteBuffers(1, &ebo);

  if (polygons->normals != NULL)
  {
    glDisableVertexAttribArray(loc_normal);
    glDeleteBuffers(1, &vbo_normals);
  }

  glDisableVertexAttribArray(loc_position);
  glDeleteBuffers(1, &vbo_points);

  if (eboBuffer != NULL)
  {
    free(eboBuffer);
  }
  glUseProgram(0);
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : draw_triangles_per_item_colours
@INPUT      : window
              polygons
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Draws a set of polygons which has a separate colour for each
              polygon.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */
static void
draw_triangles_per_item_colours(Gwindow window,
                                polygons_struct *polygons)
{
  GLuint vbo_points;
  GLuint vbo_normals;
  GLuint vbo_colours;
  GLint loc_position;
  GLint loc_normal;
  GLint loc_colour;
  GLint loc_surfprop;
  float *bufPoints = NULL; /* Temporary storage for visible vertices. */
  float *bufNormals = NULL;
  GLuint *bufColours = NULL;
  int i, k = 0;
  int start_index = 0;
  int end_index = 0;
  int n_items;
  GLuint program;

#if DEBUG
  printf("tri_per_item: %d %d\n", polygons->n_points, polygons->n_items);
#endif /* DEBUG */

  /* To save a little space and time, allocate all of the memory we
   * will need in one go...
   */
  bufPoints = malloc(polygons->n_items * sizeof(float) * N_VTX_PER_TRIANGLE * N_DIM +
                     polygons->n_items * sizeof(float) * N_VTX_PER_TRIANGLE * N_DIM +
                     polygons->n_items * sizeof(GLuint) * N_VTX_PER_TRIANGLE);
  if (bufPoints == NULL)
  {
    return;
  }
  bufNormals = &bufPoints[polygons->n_items * N_VTX_PER_TRIANGLE * N_DIM];
  bufColours = (GLuint *)&bufNormals[polygons->n_items * N_VTX_PER_TRIANGLE * N_DIM];

  for (i = 0; i < polygons->n_items; i++)
  {
    start_index = end_index;
    end_index = polygons->end_indices[i];
    if (polygons->visibilities == NULL || polygons->visibilities[i])
    {
      int j;
      for (j = start_index; j < end_index; j++)
      {
        int c = polygons->indices[j];
        int nk = k * N_DIM;
        bufPoints[nk + VIO_X] = Point_x(polygons->points[c]);
        bufPoints[nk + VIO_Y] = Point_y(polygons->points[c]);
        bufPoints[nk + VIO_Z] = Point_z(polygons->points[c]);
        bufNormals[nk + VIO_X] = Vector_x(polygons->normals[c]);
        bufNormals[nk + VIO_Y] = Vector_y(polygons->normals[c]);
        bufNormals[nk + VIO_Z] = Vector_z(polygons->normals[c]);
        bufColours[k] = polygons->colours[i];
        k++;
      }
    }
  }
  n_items = k;

  program = window->GS_window->programs[PROGRAM_VERTEX];

  switch (window->shaded_mode_state)
  {
  case WIREFRAME:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  case POINT:
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    break;
  default:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  }

  glUseProgram(program);

  set_program_opacity(program, Surfprop_t(polygons->surfprop));

  glGenBuffers(1, &vbo_points);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_points);
  glBufferData(GL_ARRAY_BUFFER, n_items * sizeof(float) * N_VTX_PER_TRIANGLE,
               bufPoints, GL_STATIC_DRAW);

  loc_position = glGetAttribLocation(program, "position");
  glVertexAttribPointer(loc_position, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(loc_position);

  loc_surfprop = glGetUniformLocation(program, "surfprop");
  /* sets the crucial 4 parameters for surface properties:
     ambient, diffuse, specular, shininess
  */
  glUniform4fv(loc_surfprop, 1, &polygons->surfprop.a);

  glGenBuffers(1, &vbo_normals);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
  glBufferData(GL_ARRAY_BUFFER, n_items * sizeof(float) * N_VTX_PER_TRIANGLE,
               bufNormals, GL_STATIC_DRAW);

  loc_normal = glGetAttribLocation(program, "normal");
  glVertexAttribPointer(loc_normal, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(loc_normal);

  glGenBuffers(1, &vbo_colours);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_colours);
  glBufferData(GL_ARRAY_BUFFER, n_items * sizeof(GLuint),
               bufColours, GL_STATIC_DRAW);

  loc_colour = glGetAttribLocation(program, "colour");
  glVertexAttribPointer(loc_colour, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
  glEnableVertexAttribArray(loc_colour);

  glDrawArrays(GL_TRIANGLES, 0, n_items);

  if (window->shaded_mode_state == OVERLAY)
  {
    do_wireframe_overlay(window, polygons, WHITE);
    glDrawArrays(GL_TRIANGLES, 0, n_items);
  }

  glDisableVertexAttribArray(loc_position);
  glDisableVertexAttribArray(loc_normal);
  glDisableVertexAttribArray(loc_colour);

  glDeleteBuffers(1, &vbo_colours);
  glDeleteBuffers(1, &vbo_normals);
  glDeleteBuffers(1, &vbo_points);
  free(bufPoints);
  glUseProgram(0);
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : draw_polygons_per_vertex_colours
@INPUT      : window
              polygons
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Draws a set of polygons which have a separate colour for each
              vertex.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */
static void
draw_polygons_per_vertex_colours(Gwindow window, polygons_struct *polygons)
{
  GLuint vbo_points;
  GLuint vbo_normals;
  GLuint vbo_colours;
  GLuint ebo;
  GLint loc_position;
  GLint loc_normal;
  GLint loc_colour;
  GLint loc_surfprop;
  int *eboBuffer = NULL;        /* Temporary storage for visible indices. */
  int n_indices = 0;
  GLenum mode;
  GLuint program;

  if (!is_triangular(polygons))
  {
    mode = GL_POLYGON;
  }
  else
  {
    mode = GL_TRIANGLES;
  }

  n_indices = NUMBER_INDICES(*polygons);

  /* If the visibilities field is set, we need to copy the visible
   * indices to a new buffer. I don't know of any other way to control
   * visibility on a per-polygon basis (perhaps we could set the
   * colour to a transparent value in the fragment shader?).
   */
  if (polygons->visibilities != NULL)
  {
    int i, k = 0;

    eboBuffer = malloc(n_indices * sizeof(int));
    if (eboBuffer == NULL)
    {
      return;
    }

    for (i = 0; i < polygons->n_items; i++)
    {
      if (polygons->visibilities[i])
      {
        int start_index = START_INDEX( polygons->end_indices, i );
        int end_index = polygons->end_indices[i];
        int j;
        for (j = start_index; j < end_index; j++)
        {
          eboBuffer[k++] = polygons->indices[j];
        }
      }
    }
    n_indices = k;
#if DEBUG
    printf("Displaying %d/%d indices\n", n_indices, NUMBER_INDICES(*polygons));
#endif  /* DEBUG */
  }

  program = window->GS_window->programs[PROGRAM_VERTEX];

  switch (window->shaded_mode_state)
  {
  case WIREFRAME:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  case POINT:
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    break;
  default:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  }

  glUseProgram(program);

  set_program_opacity(program, Surfprop_t(polygons->surfprop));

  glGenBuffers(1, &vbo_points);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_points);
  glBufferData(GL_ARRAY_BUFFER, polygons->n_points * sizeof(VIO_Point),
               polygons->points, GL_STATIC_DRAW);

  loc_position = glGetAttribLocation(program, "position");
  glVertexAttribPointer(loc_position, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(loc_position);

  loc_surfprop = glGetUniformLocation(program, "surfprop");
  /* sets the crucial 4 parameters for surface properties:
     ambient, diffuse, specular, shininess
  */
  glUniform4fv(loc_surfprop, 1, &polygons->surfprop.a);

  glGenBuffers(1, &vbo_normals);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
  glBufferData(GL_ARRAY_BUFFER, polygons->n_points * sizeof(VIO_Point),
               polygons->normals, GL_STATIC_DRAW);

  loc_normal = glGetAttribLocation(program, "normal");
  glVertexAttribPointer(loc_normal, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(loc_normal);

  glGenBuffers(1, &vbo_colours);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_colours);
  glBufferData(GL_ARRAY_BUFFER, polygons->n_points * sizeof(VIO_Colour),
               polygons->colours, GL_STATIC_DRAW);

  loc_colour = glGetAttribLocation(program, "colour");
  glVertexAttribPointer(loc_colour, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
  glEnableVertexAttribArray(loc_colour);

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               n_indices * sizeof(int),
               (eboBuffer != NULL) ? eboBuffer : polygons->indices,
               GL_STATIC_DRAW);

  glDrawElements( mode, n_indices, GL_UNSIGNED_INT, 0 );
  if ( window->shaded_mode_state == OVERLAY )
  {
    do_wireframe_overlay( window, polygons, WHITE );
    glDrawElements( mode, n_indices, GL_UNSIGNED_INT, 0 );
  }

  glDisableVertexAttribArray(loc_position);
  glDisableVertexAttribArray(loc_normal);
  glDisableVertexAttribArray(loc_colour);

  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo_colours);
  glDeleteBuffers(1, &vbo_normals);
  glDeleteBuffers(1, &vbo_points);
  if (eboBuffer != NULL)
  {
    free(eboBuffer);
  }
  glUseProgram(0);
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : draw_polygons
@INPUT      : windows
              polygons
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Draws a set of polygons in the window.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

static  void  draw_polygons(
    Gwindow         window,
    polygons_struct *polygons )
{
    if( !window->shaded_mode_state &&
        polygons->line_thickness > 1.0f &&
        polygons->line_thickness < MAX_LINE_WIDTH )
    {
        GS_set_line_width( (VIO_Real) polygons->line_thickness );
    }

    switch( polygons->colour_flag )
    {
    case ONE_COLOUR:
      if (!is_triangular(polygons))
      {
        draw_polygons_one_colour( window, polygons );
      }
      else
      {
        draw_triangles_one_colour( window, polygons );
      }
      break;

    case PER_ITEM_COLOURS:
      if (!is_triangular(polygons))
      {
        draw_polygons_one_colour( window, polygons );
      }
      else
      {
        draw_triangles_per_item_colours( window, polygons );
      }
      break;

    case  PER_VERTEX_COLOURS:
      draw_polygons_per_vertex_colours( window, polygons );
      break;
    }

    if( !window->shaded_mode_state &&
        polygons->line_thickness > 1.0f &&
        polygons->line_thickness < MAX_LINE_WIDTH )
        GS_set_line_width( 1.0 );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_draw_polygons
@INPUT      : window
              polygons
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Routine to draw a set of polygons in the window.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

  void  G_draw_polygons(
    Gwindow         window,
    polygons_struct *polygons )
{
    if (polygons->n_points == 0 || polygons->n_items == 0)
    {
        return;                 /* no work to do! */
    }

    about_to_draw_graphics( window );

    draw_polygons( window, polygons );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : draw_quadmesh_one_colour
@INPUT      : window
              quadmesh
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Draws a quadmesh of a single colour into the window.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */
void draw_quadmesh_one_colour(Gwindow window, quadmesh_struct *quadmesh)
{
  GLuint vbo_points;
  GLuint vbo_normals;
  GLuint ebo;
  GLint loc_position;
  GLint loc_normal = -1;
  int n_items = 0;
  GLint program;
  int m_size, n_size;
  int i, j;
  GLuint *indices;

#if DEBUG
  printf("draw_quadmesh_one_colour %d %d %d %d %d %p (NEW)\n", quadmesh->m_closed, quadmesh->n_closed, quadmesh->m, quadmesh->n, quadmesh->colour_flag, quadmesh->normals);
#endif  /* DEBUG */

  get_quadmesh_n_objects(quadmesh, &m_size, &n_size);
  n_items = quadmesh->m * quadmesh->n;

  if (quadmesh->normals != NULL)
  {
    program = window->GS_window->programs[PROGRAM_SINGLE];
    /*
     * Set up nondefault surface properties.
     */
    set_surface_property( window, quadmesh->colours[0], &quadmesh->surfprop );
  }
  else
  {
    program = window->GS_window->programs[PROGRAM_TRIVIAL];
  }

  set_colour( window, quadmesh->colours[0] );
  if( window->shaded_mode_state )
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
  }
  else
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
  }
  glUseProgram(program);

  set_program_opacity(program, Surfprop_t(quadmesh->surfprop));
  set_program_mxfactor( program, 1.0 );

  glGenBuffers(1, &vbo_points);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_points);
  glBufferData(GL_ARRAY_BUFFER, n_items * sizeof(VIO_Point),
               quadmesh->points, GL_STATIC_DRAW);

  loc_position = glGetAttribLocation(program, "position");
  glVertexAttribPointer(loc_position, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(loc_position);
  GLCHECK;

  if (quadmesh->normals != NULL)
  {
    glGenBuffers(1, &vbo_normals);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
    glBufferData(GL_ARRAY_BUFFER, n_items * sizeof(VIO_Vector),
                 quadmesh->normals, GL_STATIC_DRAW);
    loc_normal = glGetAttribLocation(program, "normal");
    glVertexAttribPointer(loc_normal, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(loc_normal);
    GLCHECK;
  }

  indices = malloc((n_size + 1) * 2 * sizeof(GLuint));

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  /* Draw these in m_size separate strips, otherwise we get unwanted
   * extra faces "closing" the surface.
   */
  for (i = 0; i < m_size; i++)
  {
    int k = 0;
    for (j = 0; j <= n_size; j++)
    {
      int used_j = j % quadmesh->n;
      indices[k++] = VIO_IJ(i, used_j, quadmesh->n);
      indices[k++] = VIO_IJ((i + 1) % quadmesh->m, used_j, quadmesh->n);
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, k * sizeof(GLuint),
                 indices, GL_DYNAMIC_DRAW);
    glDrawElements(GL_TRIANGLE_STRIP, k, GL_UNSIGNED_INT, 0);
    GLCHECK;
  }

  free(indices);

  if (quadmesh->normals != NULL)
  {
    glDisableVertexAttribArray(loc_normal);
    glDeleteBuffers(1, &vbo_normals);
    GLCHECK;
  }

  glDeleteBuffers(1, &ebo);
  glDisableVertexAttribArray(loc_position);
  glDeleteBuffers(1, &vbo_points);
  GLCHECK;

  glUseProgram(0);
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : draw_quadmesh_per_item_colours
@INPUT      : window
              quadmesh
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Draws a quadmesh with a separate colour per quadrilateral.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

static  void  draw_quadmesh_per_item_colours(
    Gwindow         window,
    quadmesh_struct *quadmesh )
{
  printf("draw_quadmesh_per_item_colours\n");
#define    DEF_PER_ITEM_COLOURS

    if( window->shaded_mode_state )
    {
        if( window->lighting_state && quadmesh->normals != (VIO_Vector *) 0 )
        {
#define     DEF_NORMALS
#include    "draw_quadmesh.include.c"
#undef      DEF_NORMALS
        }
        else
        {
#include    "draw_quadmesh.include.c"
        }
    }
    else
    {
#define  DEF_WIREFRAME
        if( window->lighting_state && quadmesh->normals != (VIO_Vector *) 0 )
        {
#define     DEF_NORMALS
#include    "draw_quadmesh.include.c"
#undef      DEF_NORMALS
        }
        else
        {
#include    "draw_quadmesh.include.c"
        }
#undef   DEF_WIREFRAME
    }

#undef      DEF_PER_ITEM_COLOURS
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : draw_quadmesh_per_vertex_colours
@INPUT      : window
              quadmesh
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Draws a quadmesh with a separate colour per vertex.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */
#if 1
void draw_quadmesh_per_vertex_colours(Gwindow window, quadmesh_struct *quadmesh)
{
  GLuint vbo_points;
  GLuint vbo_normals;
  GLuint vbo_colours;
  GLuint ebo;
  GLint loc_position;
  GLint loc_normal;
  GLint loc_colour;
  GLint loc_surfprop;
  int n_items = 0;
  GLint program;
  int m_size, n_size;
  int i, j;
  GLuint *indices;

  printf("draw_quadmesh_per_vertex_colours %d %d %d %d %d %p (NEW)\n", quadmesh->m_closed, quadmesh->n_closed, quadmesh->m, quadmesh->n, quadmesh->colour_flag, quadmesh->normals);
  printf("  surfprop %f %f %f %f %f\n", 
         Surfprop_a(quadmesh->surfprop),
         Surfprop_d(quadmesh->surfprop),
         Surfprop_s(quadmesh->surfprop),
         Surfprop_se(quadmesh->surfprop),
         Surfprop_t(quadmesh->surfprop));

  get_quadmesh_n_objects(quadmesh, &m_size, &n_size);
  n_items = quadmesh->m * quadmesh->n;

  program = window->GS_window->programs[PROGRAM_VERTEX];
  glUseProgram(program);
  set_program_opacity(program, Surfprop_t(quadmesh->surfprop));

  if( window->shaded_mode_state )  
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
  }
  else
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
  }

  glGenBuffers(1, &vbo_points);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_points);
  glBufferData(GL_ARRAY_BUFFER, n_items * sizeof(VIO_Point),
               quadmesh->points, GL_STATIC_DRAW);

  loc_position = glGetAttribLocation(program, "position");
  glVertexAttribPointer(loc_position, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(loc_position);
  GLCHECK;

  loc_surfprop = glGetUniformLocation(program, "surfprop");
  glUniform4fv(loc_surfprop, 1, &quadmesh->surfprop.a);
  
  glGenBuffers(1, &vbo_normals);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
  glBufferData(GL_ARRAY_BUFFER, n_items * sizeof(VIO_Vector),
               quadmesh->normals, GL_STATIC_DRAW);
  
  loc_normal = glGetAttribLocation(program, "normal");
  glVertexAttribPointer(loc_normal, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(loc_normal);

  glGenBuffers(1, &vbo_colours);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_colours);
  glBufferData(GL_ARRAY_BUFFER, n_items * sizeof(VIO_Colour),
               quadmesh->colours, GL_STATIC_DRAW);

  loc_colour = glGetAttribLocation(program, "colour");
  glVertexAttribPointer(loc_colour, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
  glEnableVertexAttribArray(loc_colour);

  indices = malloc((n_size + 1) * 2 * sizeof(GLuint));

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  /* Draw these in m_size separate strips, otherwise we get unwanted
   * extra faces "closing" the surface.
   */
  for (i = 0; i < m_size; i++)
  {
    int k = 0;
    for (j = 0; j <= n_size; j++)
    {
      int used_j = j % quadmesh->n;
      indices[k++] = VIO_IJ(i, used_j, quadmesh->n);
      indices[k++] = VIO_IJ((i + 1) % quadmesh->m, used_j, quadmesh->n);
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, k * sizeof(GLuint),
                 indices, GL_DYNAMIC_DRAW);
    glDrawElements(GL_TRIANGLE_STRIP, k, GL_UNSIGNED_INT, 0);
    GLCHECK;
  }

  free(indices);

  glDisableVertexAttribArray(loc_position);
  glDisableVertexAttribArray(loc_colour);
  glDisableVertexAttribArray(loc_normal);
  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo_points);
  glDeleteBuffers(1, &vbo_colours);
  glDeleteBuffers(1, &vbo_normals);

  glUseProgram(0);
}
#else
static  void  draw_quadmesh_per_vertex_colours(
    Gwindow         window,
    quadmesh_struct *quadmesh )
{
  printf("draw_quadmesh_per_vertex_colours\n");
#define DEF_PER_VERTEX_COLOURS

    if( window->shaded_mode_state )
    {
        if( window->lighting_state && quadmesh->normals != (VIO_Vector *) 0 )
        {
#define     DEF_NORMALS
#include    "draw_quadmesh.include.c"
#undef      DEF_NORMALS
        }
        else
        {
#include    "draw_quadmesh.include.c"
        }
    }
    else
    {
#define  DEF_WIREFRAME
        if( window->lighting_state && quadmesh->normals != (VIO_Vector *) 0 )
        {
#define     DEF_NORMALS
#include    "draw_quadmesh.include.c"
#undef      DEF_NORMALS
        }
        else
        {
#include    "draw_quadmesh.include.c"
        }
#undef   DEF_WIREFRAME
    }

#undef      DEF_PER_VERTEX_COLOURS
}
#endif
/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_draw_quadmesh
@INPUT      : window
              quadmesh
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Draws a quadmesh in the window.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

  void  G_draw_quadmesh(
    Gwindow         window,
    quadmesh_struct *quadmesh )
{
    about_to_draw_graphics( window );

    switch( quadmesh->colour_flag )
    {
    case  ONE_COLOUR:
        draw_quadmesh_one_colour( window, quadmesh );
        break;

    case  PER_ITEM_COLOURS:
        draw_quadmesh_per_item_colours( window, quadmesh );
        break;

    case  PER_VERTEX_COLOURS:
        draw_quadmesh_per_vertex_colours( window, quadmesh );
        break;
    }
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_draw_lines
@INPUT      : window
              lines
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Draws the lines in the given window.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */
void
G_draw_lines(Gwindow window, lines_struct *lines )
{
  GLuint vbo_points;            /* OpenGL vertex buffer object */
  GLint loc_position;           /* Location of the "position" attribute. */
  GLint program;
  int save_lights;
  int i;

  if (lines->n_points == 0 || lines->n_items == 0)
  {
    /* Nothing to do! */
    return;
  }

  if (lines->colour_flag == PER_VERTEX_COLOURS)
  {
    fprintf(stderr, "G_draw_lines does not handle per-vertex colours yet.\n");
    return;
  }

  about_to_draw_graphics( window );

  save_lights = G_get_lighting_state( window );
  G_set_lighting_state( window, FALSE );

  if( lines->line_thickness > 1.0 && lines->line_thickness < MAX_LINE_WIDTH )
    GS_set_line_width( lines->line_thickness );

  if( lines->colour_flag == ONE_COLOUR )
    set_colour( window, lines->colours[0] );

  program = window->GS_window->programs[PROGRAM_TRIVIAL];

  /* Set up the buffer for our vertex data.
   */
  glUseProgram(program);

  set_program_opacity(program, 1.0);

  glGenBuffers(1, &vbo_points);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_points);
  glBufferData(GL_ARRAY_BUFFER, lines->n_points * sizeof(VIO_Point),
               lines->points, GL_STATIC_DRAW);

  /* Assign our vertex data to the "position" value in the shader.
   */
  loc_position = glGetAttribLocation(program, "position");
  glVertexAttribPointer(loc_position, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(loc_position);

  /* Now start sending the line segments.
   */
  if (lines->colour_flag == PER_ITEM_COLOURS)
  {
    int start_index = 0;
    int end_index = 0;

    for (i = 0; i < lines->n_items; i++)
    {
      start_index = end_index;
      end_index = lines->end_indices[i];
      set_colour(window, lines->colours[i]);
      glDrawElements(GL_LINE_STRIP, end_index - start_index,
                     GL_UNSIGNED_INT,
                     &lines->indices[start_index]);
    }
  }
  else
  {
    int start_index = 0;
    int end_index = 0;

    for (i = 0; i < lines->n_items; i++)
    {
      start_index = end_index;
      end_index = lines->end_indices[i];
      glDrawElements(GL_LINE_STRIP, end_index - start_index,
                     GL_UNSIGNED_INT,
                     &lines->indices[start_index]);
    }
  }

  /* Clean up after ourselves.
   */
  glDisableVertexAttribArray(loc_position);
  glDeleteBuffers(1, &vbo_points);
  glUseProgram(0);

  if( lines->line_thickness > 1.0 && lines->line_thickness < MAX_LINE_WIDTH )
    GS_set_line_width( 1.0 );

  G_set_lighting_state( window, save_lights );
}

/* ------------------------------ text and fonts ------------------- */

  VIO_Real  G_get_text_height(
    Font_types       type,
    VIO_Real             size )
{
    return( GS_get_character_height( type, size ) );
}

  VIO_Real  G_get_text_length(
    VIO_STR           str,
    Font_types       type,
    VIO_Real             size )
{
    return( GS_get_text_length( str, type, size ) );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_draw_text
@INPUT      : window
              text
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Draws the text in the given window.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

  void  G_draw_text(
    Gwindow         window,
    text_struct     *text )
{
    about_to_draw_graphics( window );

    set_colour( window, text->colour );

    BEGIN_DRAW_OBJECTS( window, window->interrupt_interval, 1, TRUE )

        GS_set_raster_position( (VIO_Real) Point_x(text->origin),
                                (VIO_Real) Point_y(text->origin),
                                (VIO_Real) Point_z(text->origin) );

        GS_draw_text( text->font, text->size, text->string );

    END_DRAW_OBJECTS
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_draw_marker
@INPUT      : window
              marker
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Draws a 3D marker in the window
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

  void  G_draw_marker(
    Gwindow         window,
    marker_struct   *marker )
{
    VIO_BOOL     save_lights;
    VIO_Transform   transform;

    about_to_draw_graphics( window );

    save_lights = G_get_lighting_state( window );
    G_set_lighting_state( window, FALSE );

    set_colour( window, marker->colour );

    BEGIN_DRAW_OBJECTS( window, window->interrupt_interval, 1, TRUE )

        if( marker->size <= 0.0 )
        {
            GS_begin_point();
                GS_set_point( &marker->position );
            GS_end_point();
        }
        else
        {
            GS_push_transform();

            make_translation_transform( (VIO_Real) Point_x(marker->position),
                                        (VIO_Real) Point_y(marker->position),
                                        (VIO_Real) Point_z(marker->position),
                                        &transform );
            GS_mult_transform( &transform );

            make_scale_transform( marker->size, marker->size, marker->size,
                                  &transform );
            GS_mult_transform( &transform );

            switch( marker->type )
            {
            case  BOX_MARKER:
                draw_marker_as_cube( window, marker->colour );
                break;

            case  SPHERE_MARKER:
                draw_marker_as_cube( window, marker->colour );
                break;

            default:
                print_error("G_draw_marker(): unknown marker type.");
                break;
            }

            GS_pop_transform();
        }

        if( window->marker_labels_visibility )
        {
            GS_set_raster_position( (VIO_Real) Point_x(marker->position) +
                                       1.5 * (VIO_Real) marker->size / 2.0,
                                    (VIO_Real) Point_y(marker->position),
                                    (VIO_Real) Point_z(marker->position) );

            GS_draw_text( FIXED_FONT, 10.0, marker->label );
        }

    END_DRAW_OBJECTS

    G_set_lighting_state( window, save_lights );
}

#define N_POINTS_CUBE 24
#define N_FACES_CUBE 6

static void
draw_marker_as_cube( Gwindow window, VIO_Colour colour )
{
    static VIO_Colour colours[1];
    static VIO_Point  points[N_POINTS_CUBE] = {
      { { -0.5f, -0.5f, -0.5f } },
      { { -0.5f, -0.5f,  0.5f } },
      { { -0.5f,  0.5f,  0.5f } },
      { { -0.5f,  0.5f, -0.5f } },

      { {  0.5f, -0.5f, -0.5f } },
      { {  0.5f,  0.5f, -0.5f } },
      { {  0.5f,  0.5f,  0.5f } },
      { {  0.5f, -0.5f,  0.5f } },

      { { -0.5f, -0.5f, -0.5f } },
      { {  0.5f, -0.5f, -0.5f } },
      { {  0.5f, -0.5f,  0.5f } },
      { { -0.5f, -0.5f,  0.5f } },

      { { -0.5f,  0.5f, -0.5f } },
      { { -0.5f,  0.5f,  0.5f } },
      { {  0.5f,  0.5f,  0.5f } },
      { {  0.5f,  0.5f, -0.5f } },

      { { -0.5f, -0.5f, -0.5f } },
      { { -0.5f,  0.5f, -0.5f } },
      { {  0.5f,  0.5f, -0.5f } },
      { {  0.5f, -0.5f, -0.5f } },

      { { -0.5f, -0.5f,  0.5f } },
      { {  0.5f, -0.5f,  0.5f } },
      { {  0.5f,  0.5f,  0.5f } },
      { { -0.5f,  0.5f,  0.5f } } };


    static VIO_Vector normals[N_POINTS_CUBE] = {

      { { -1.0f,  0.0f,  0.0f } },
      { { -1.0f,  0.0f,  0.0f } },
      { { -1.0f,  0.0f,  0.0f } },
      { { -1.0f,  0.0f,  0.0f } },

      { {  1.0f,  0.0f,  0.0f } },
      { {  1.0f,  0.0f,  0.0f } },
      { {  1.0f,  0.0f,  0.0f } },
      { {  1.0f,  0.0f,  0.0f } },

      { {  0.0f, -1.0f,  0.0f } },
      { {  0.0f, -1.0f,  0.0f } },
      { {  0.0f, -1.0f,  0.0f } },
      { {  0.0f, -1.0f,  0.0f } },

      { {  0.0f,  1.0f,  0.0f } },
      { {  0.0f,  1.0f,  0.0f } },
      { {  0.0f,  1.0f,  0.0f } },
      { {  0.0f,  1.0f,  0.0f } },

      { {  0.0f,  0.0f, -1.0f } },
      { {  0.0f,  0.0f, -1.0f } },
      { {  0.0f,  0.0f, -1.0f } },
      { {  0.0f,  0.0f, -1.0f } },

      { {  0.0f,  0.0f,  1.0f } },
      { {  0.0f,  0.0f,  1.0f } },
      { {  0.0f,  0.0f,  1.0f } },
      { {  0.0f,  0.0f,  1.0f } }
     };
     static int end_indices[N_FACES_CUBE] = {
       4, 8, 12, 16, 20, 24
     };
     static int indices[N_POINTS_CUBE] = {
       0,  1,  2,  3,  4,  5,  6,  7,
       8,  9, 10, 11, 12, 13, 14, 15,
       16, 17, 18, 19, 20, 21, 22, 23
     };

     static  polygons_struct  polygons = {
       ONE_COLOUR,
       colours,
       { 0.4, 0.6, 0.6, 30.0, 1.0 },
       1.0,
       N_POINTS_CUBE,
       points,
       normals,
       N_FACES_CUBE,
       end_indices,
       indices,
       (VIO_SCHAR *) NULL,
       (int *) NULL,
       (bintree_struct_ptr) NULL
     };

    polygons.colours[0] = colour;
    draw_polygons( window, &polygons );
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_draw_pixels
@INPUT      : window
              pixels
@OUTPUT     :
@RETURNS    :
@DESCRIPTION: Draws the pixels in the window.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

  void  G_draw_pixels(
    Gwindow         window,
    pixels_struct   *pixels )
{
    if( pixels->x_size <= 0 || pixels->y_size <= 0 )
        return;

    about_to_draw_graphics( window );

    if( pixels->x_zoom > 0.0 && pixels->y_zoom > 0.0 )
        GS_set_pixel_zoom( pixels->x_zoom, pixels->y_zoom );
    else
        GS_set_pixel_zoom( 1.0, 1.0 );

    if( window->colour_map_state )
    {
        if( pixels->pixel_type == RGB_PIXEL )
        {
            print_error(
               "G_draw_pixels(): cannot draw rgb pixels in colour map mode.\n");
            return;
        }

        GS_draw_colour_map_pixels( window->x_viewport_min,
                                   window->y_viewport_min, pixels );
    }
    else
    {
        if( pixels->pixel_type != RGB_PIXEL )
        {
            print_error(
               "G_draw_pixels(): cannot draw colour map pixels in rgb mode.\n");
            return;
        }

        GS_draw_rgb_pixels( window->x_viewport_min,
                            window->y_viewport_min, pixels );
    }
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : G_read_pixels
@INPUT      : window
              x_min
              x_max
              y_min
              y_max
@OUTPUT     : pixels
@RETURNS    :
@DESCRIPTION: Reads the pixels from the frame buffer.
@METHOD     :
@GLOBALS    :
@CALLS      :
@CREATED    : 1993            David MacDonald
@MODIFIED   :
---------------------------------------------------------------------------- */

  void  G_read_pixels(
    Gwindow         window,
    int             x_min,
    int             x_max,
    int             y_min,
    int             y_max,
    VIO_Colour          pixels[] )
{
    set_current_window( window );

    GS_read_pixels( G_get_colour_map_state(window),
                    x_min, x_max, y_min, y_max, pixels );
}
