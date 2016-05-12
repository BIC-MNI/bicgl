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
static  void     draw_marker_as_sphere( Gwindow, VIO_Colour );

static  void  set_colour(
    Gwindow   window,
    VIO_Colour    colour )
{
     if( window->colour_map_state ||
         window->current_bitplanes == OVERLAY_PLANES )
         GS_set_colour_index( colour );
     /*
     else if( window->stereo_flag )
     {
         int  intensity;
         intensity = 0.324 * get_Colour_r(colour) +
                     0.499 * get_Colour_g(colour) +
                     0.177 * get_Colour_b(colour) + 0.5;
         GS_set_colour( make_Colour( intensity, intensity, intensity ) );
     }
     */
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
    int n_iters, i;

    if( !window->shaded_mode_state &&
        polygons->line_thickness > 1.0f &&
        polygons->line_thickness < MAX_LINE_WIDTH )
    {
        GS_set_line_width( (VIO_Real) polygons->line_thickness );
    }

    if( view_is_stereo(window) )
    {
        n_iters = 2;
    }
    else
    {
        n_iters = 1;
    }

    for (i = 0; i < n_iters; i++)
    {
      if (n_iters == 2)
      {
          set_view_for_eye( window, i );
          if (i == 0)
            glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
          else
            glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
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
    }

    if( view_is_stereo(window) )
    {
        set_view_for_eye( window, -1 );
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
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

#if DEBUG
  printf("draw_quadmesh_per_vertex_colours %d %d %d %d %d %p (NEW)\n", quadmesh->m_closed, quadmesh->n_closed, quadmesh->m, quadmesh->n, quadmesh->colour_flag, quadmesh->normals);
  printf("  surfprop %f %f %f %f %f\n", 
         Surfprop_a(quadmesh->surfprop),
         Surfprop_d(quadmesh->surfprop),
         Surfprop_s(quadmesh->surfprop),
         Surfprop_se(quadmesh->surfprop),
         Surfprop_t(quadmesh->surfprop));
#endif /* DEBUG */

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
  GLuint vbo_colours;
  GLint loc_position;           /* Location of the "position" attribute. */
  GLint loc_colour;
  GLint loc_surfprop;
  GLint program;
  int save_lights;
  int start_index = 0;
  int end_index = 0;
  int i;

  if (lines->n_points == 0 || lines->n_items == 0)
  {
    /* Nothing to do! */
    return;
  }

  about_to_draw_graphics( window );

  save_lights = G_get_lighting_state( window );
  G_set_lighting_state( window, FALSE );

  if( lines->line_thickness > 1.0 && lines->line_thickness < MAX_LINE_WIDTH )
    GS_set_line_width( lines->line_thickness );

  if (lines->colour_flag == PER_VERTEX_COLOURS)
  {
    program = window->GS_window->programs[PROGRAM_VERTEX];
  }
  else
  {
    set_colour( window, lines->colours[0] );
    program = window->GS_window->programs[PROGRAM_TRIVIAL];
  }

  glUseProgram(program);

  set_program_opacity(program, 1.0);

  /* Set up the buffer for our vertex data.
   */
  glGenBuffers(1, &vbo_points);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_points);
  glBufferData(GL_ARRAY_BUFFER, lines->n_points * sizeof(VIO_Point),
               lines->points, GL_STATIC_DRAW);

  /* Assign our vertex data to the "position" value in the shader.
   */
  loc_position = glGetAttribLocation(program, "position");
  glVertexAttribPointer(loc_position, N_DIM, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(loc_position);

  if (lines->colour_flag == PER_VERTEX_COLOURS)
  {
    VIO_Surfprop spr = {1, 0, 0, 0, 1};
    loc_surfprop = glGetUniformLocation(program, "surfprop");
    /* sets the crucial 4 parameters for surface properties:
       ambient, diffuse, specular, shininess
    */
    glUniform4fv(loc_surfprop, 1, &spr.a);

    /* Set up the buffer for the colour data.
     */
    glGenBuffers(1, &vbo_colours);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_colours);
    glBufferData(GL_ARRAY_BUFFER, lines->n_points * sizeof(GLuint),
                 lines->colours, GL_STATIC_DRAW);

    /* Assign the colour data to the "colour" value in the shader.
     */
    loc_colour = glGetAttribLocation(program, "colour");
    glVertexAttribPointer(loc_colour, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
    glEnableVertexAttribArray(loc_colour);
  }

  /* Now start sending the line segments.
   */
  for (i = 0; i < lines->n_items; i++)
  {
    start_index = end_index;
    end_index = lines->end_indices[i];
    if (lines->colour_flag == PER_ITEM_COLOURS)
    {
      set_colour(window, lines->colours[i]);
    }
    glDrawElements(GL_LINE_STRIP, end_index - start_index, GL_UNSIGNED_INT,
                   &lines->indices[start_index]);
  }

  /* Clean up after ourselves.
   */
  if (lines->colour_flag == PER_VERTEX_COLOURS)
  {
    glDisableVertexAttribArray(loc_colour);
    glDeleteBuffers(1, &vbo_colours);
  }
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

    GS_set_raster_position( (VIO_Real) Point_x(text->origin),
                            (VIO_Real) Point_y(text->origin),
                            (VIO_Real) Point_z(text->origin) );

    GS_draw_text( text->font, text->size, text->string );
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
        draw_marker_as_sphere( window, marker->colour );
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

double DEGS_TO_RAD = M_PI / 180.0;

void
create_sphere(VIO_Point center, double radius, int n_latitude, int n_longitude,
              polygons_struct *polygons)
{
  int p;
  int s;
  int n_indices = 0;
  int n = 2;

  const int n_pitch = n_longitude + 1;
  const VIO_Real pitch_inc = (M_PI / (VIO_Real) n_pitch);
  const VIO_Real rot_inc = (M_PI * 2.0 / (VIO_Real) n_latitude);
  const VIO_Real cx = Point_x(center);
  const VIO_Real cy = Point_y(center);
  const VIO_Real cz = Point_z(center);
  const int n_triangles = n_longitude * 4;
  const int n_quads = n_latitude * (n_longitude - 1);
  const int first_offset = 2;
  const int last_offset = first_offset + (n_latitude * (n_longitude - 1));

  polygons->n_items = n_triangles + n_quads;
  polygons->n_points = 2 + (n_latitude * n_longitude);
  ALLOC(polygons->points, polygons->n_points);
  ALLOC(polygons->normals, polygons->n_points);
  ALLOC(polygons->end_indices, polygons->n_items);
  ALLOC(polygons->indices, 3 * n_triangles + 4 * n_quads);

  /* Generate top and bottom vertices. */
  fill_Point(polygons->points[0], cx, cy + radius, cz);
  fill_Point(polygons->points[1], cx, cy - radius, cz);

  fill_Vector(polygons->normals[0], 0, radius, 0);
  fill_Vector(polygons->normals[1], 0, -radius, 0);

  for (p = 1; p < n_pitch; p++)
  {
    VIO_Real out = fabs(radius * sin(p * pitch_inc));
    VIO_Real y = radius * cos(p * pitch_inc);

    for (s = 0; s < n_latitude; s++)
    {
      VIO_Real x = out * cos(s * rot_inc);
      VIO_Real z = out * sin(s * rot_inc);

      fill_Vector(polygons->normals[n], x, y, z);
      fill_Point(polygons->points[n], x + cx, y + cy, z + cz);
      n++;
    }
  }

  n = 0;

  /* Create the quadrilateral faces between intermediate points.
   */
  for (p = 1; p < n_longitude; p++) /* for each longitude line */
  {
    for (s = 0; s < n_latitude; s++) /* for each latitude line */
    {
      int i = p * n_latitude + s;
      int j = (s == n_latitude - 1) ? i - n_latitude : i;

      polygons->indices[n_indices++] = i - n_latitude + first_offset;
      polygons->indices[n_indices++] = j + 1 - n_latitude + first_offset;
      polygons->indices[n_indices++] = j + 1 + first_offset;
      polygons->indices[n_indices++] = i + first_offset;
      polygons->end_indices[n++] = n_indices;
    }
  }

  /* Create the "triangular fan" that surrounds the top and bottom
   * vertices.
   */
  int n_around = n_longitude * 2;
  for (s = 0; s < n_around; s++)
  {
    int j = (s == n_around - 1) ? -1 : s;

    polygons->indices[n_indices++] = 0; /* index of top  */
    polygons->indices[n_indices++] = j + 1 + first_offset;
    polygons->indices[n_indices++] = s + first_offset;
    polygons->end_indices[n++] = n_indices;

    polygons->indices[n_indices++] = 1; /* index of bottom */
    polygons->indices[n_indices++] = s + last_offset;
    polygons->indices[n_indices++] = j + 1 + last_offset;
    polygons->end_indices[n++] = n_indices;
  }
}

static void
draw_marker_as_sphere( Gwindow window, VIO_Colour colour )
{
  object_struct   *object = create_object( POLYGONS );
  polygons_struct *polygons = get_polygons_ptr( object );
  VIO_Point       pt = { { 0, 0, 0 } };
  initialize_polygons( polygons, colour, NULL );
  create_sphere( pt, 0.5, 16, 16/2, polygons );
  draw_polygons( window, polygons );
  delete_object( object );
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
