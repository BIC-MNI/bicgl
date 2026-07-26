#define GL_GLEXT_PROTOTYPES 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <font_render_gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Same GLSL 1.20 idiom as OpenGL_graphics/windows.c's vertex_shader_/
 * fragment_shader pair (gl_ModelViewProjectionMatrix, attribute-qualified
 * inputs) -- this codebase already runs a GLSL-based pipeline for regular
 * 3D rendering on all three windowing backends, so the same convention is
 * used here rather than introducing a different shading style. */
static const char *vertex_shader_text[] = {
"#version 120 \n\
attribute vec3 position;\n\
attribute vec2 texcoord;\n\
varying vec2 v_texcoord;\n\
void main() {\n\
    v_texcoord = texcoord;\n\
    gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);\n\
}"
};

static const char *fragment_shader_text[] = {
"#version 120\n\
uniform sampler2D atlasTex;\n\
uniform vec4 uColor;\n\
varying vec2 v_texcoord;\n\
void main()\n\
{\n\
    float coverage = texture2D(atlasTex, v_texcoord).a;\n\
    gl_FragColor = vec4(uColor.rgb, uColor.a * coverage);\n\
}"
};

static GLuint create_program( const char *vs[], const char *fs[] )
{
    GLuint vertexShader   = glCreateShader( GL_VERTEX_SHADER );
    GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
    GLuint program        = glCreateProgram();
    GLint  status;
    char   buffer[512];

    glShaderSource( vertexShader, 1, vs, NULL );
    glCompileShader( vertexShader );
    glShaderSource( fragmentShader, 1, fs, NULL );
    glCompileShader( fragmentShader );

    glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &status );
    if( status != GL_TRUE )
    {
        glGetShaderInfoLog( vertexShader, sizeof(buffer), NULL, buffer );
        fprintf( stderr, "font_render_gl: vertex shader compile failed: %s\n", buffer );
    }

    glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &status );
    if( status != GL_TRUE )
    {
        glGetShaderInfoLog( fragmentShader, sizeof(buffer), NULL, buffer );
        fprintf( stderr, "font_render_gl: fragment shader compile failed: %s\n", buffer );
    }

    glAttachShader( program, vertexShader );
    glAttachShader( program, fragmentShader );
    glLinkProgram( program );

    glGetProgramiv( program, GL_LINK_STATUS, &status );
    if( status != GL_TRUE )
    {
        glGetProgramInfoLog( program, sizeof(buffer), NULL, buffer );
        fprintf( stderr, "font_render_gl: program link failed: %s\n", buffer );
    }

    return program;
}

/* Lazily-initialised, process-lifetime GL objects. A GL context is always
 * current by the time WS_draw_text (and therefore this) is called. */
static GLuint  s_program           = 0;
static GLuint  s_vbo               = 0;
static GLuint  s_texture           = 0;
static const FontAtlas *s_uploaded_atlas = NULL;

static void ensure_gl_objects( void )
{
    if( s_program == 0 )
        s_program = create_program( vertex_shader_text, fragment_shader_text );

    if( s_vbo == 0 )
        glGenBuffers( 1, &s_vbo );

    if( s_texture == 0 )
    {
        glGenTextures( 1, &s_texture );
        glBindTexture( GL_TEXTURE_2D, s_texture );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    }
}

static void upload_atlas_if_changed( const FontAtlas *atlas )
{
    if( atlas == s_uploaded_atlas )
        return;

    glBindTexture( GL_TEXTURE_2D, s_texture );
    /* GL_ALPHA8, not GL_R8/ARB_texture_rg: core since GL 1.1, needs no
     * extension query, and works identically across the GLX, GLFW/EGL,
     * and GLUT backends' compatibility-profile contexts. */
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA8, atlas->atlas_w, atlas->atlas_h,
                  0, GL_ALPHA, GL_UNSIGNED_BYTE, atlas->atlas_pixels );

    s_uploaded_atlas = atlas;
}

/* One 6-vertex (2-triangle) quad per glyph; interleaved (x,y,z,u,v). */
#define FLOATS_PER_VERTEX 5
#define VERTICES_PER_GLYPH 6

void font_render_gl_draw_text( const FontAtlas *atlas, const char *s )
{
    if( !atlas || !s || !s[0] )
        return;

    /* Matches the glBitmap/glCallLists path this replaces: if the raster
     * position is invalid (e.g. text positioned outside the view volume),
     * draw nothing rather than rendering at a stale/clipped-away anchor. */
    GLboolean raster_pos_valid;
    glGetBooleanv( GL_CURRENT_RASTER_POSITION_VALID, &raster_pos_valid );
    if( !raster_pos_valid )
        return;

    GLfloat anchor[4];
    glGetFloatv( GL_CURRENT_RASTER_POSITION, anchor );

    GLfloat colour[4];
    glGetFloatv( GL_CURRENT_COLOR, colour );

    GLint viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );

    ensure_gl_objects();
    upload_atlas_if_changed( atlas );

    size_t len = strlen( s );
    GLfloat *verts = (GLfloat *) malloc( sizeof(GLfloat) * FLOATS_PER_VERTEX * VERTICES_PER_GLYPH * len );
    if( !verts )
        return;

    size_t n_verts = 0;
    float  cursor_x = anchor[0];
    float  z = anchor[2];

    size_t i;
    for( i = 0; i < len; ++i )
    {
        unsigned char c = (unsigned char) s[i];
        if( c < FONT_ATLAS_FIRST_CODEPOINT ||
            c >= FONT_ATLAS_FIRST_CODEPOINT + FONT_ATLAS_N_GLYPHS )
        {
            cursor_x += atlas->advance_width;
            continue;
        }

        const stbtt_packedchar *g = &atlas->glyphs[c - FONT_ATLAS_FIRST_CODEPOINT];

        float glyph_w = (float) (g->x1 - g->x0);
        float glyph_h = (float) (g->y1 - g->y0);

        /* stbtt_packedchar's yoff/yoff2 are in stb_truetype's top-down
         * image-space convention (increasing downward from the baseline).
         * anchor.y / this ortho are bottom-up window space, so the
         * vertical placement must invert that offset -- getting this
         * backwards renders glyphs upside down / mispositioned. */
        float quad_x0 = cursor_x + g->xoff;
        float quad_x1 = quad_x0 + glyph_w;
        float quad_y1 = anchor[1] - g->yoff;
        float quad_y0 = quad_y1 - glyph_h;

        float u0 = (float) g->x0 / (float) atlas->atlas_w;
        float u1 = (float) g->x1 / (float) atlas->atlas_w;
        float v0 = (float) g->y0 / (float) atlas->atlas_h;
        float v1 = (float) g->y1 / (float) atlas->atlas_h;

        GLfloat quad[VERTICES_PER_GLYPH][FLOATS_PER_VERTEX] = {
            { quad_x0, quad_y0, z, u0, v1 },
            { quad_x1, quad_y0, z, u1, v1 },
            { quad_x1, quad_y1, z, u1, v0 },
            { quad_x0, quad_y0, z, u0, v1 },
            { quad_x1, quad_y1, z, u1, v0 },
            { quad_x0, quad_y1, z, u0, v0 },
        };

        memcpy( &verts[n_verts * FLOATS_PER_VERTEX], quad, sizeof(quad) );
        n_verts += VERTICES_PER_GLYPH;

        cursor_x += atlas->advance_width;
    }

    if( n_verts == 0 )
    {
        free( verts );
        cursor_x = anchor[0]; /* nothing drawn, still fall through to raster pos update below */
    }

    /* Save state we are about to change so callers see no lasting side
     * effects, matching the glUseProgram(0)-restores-fixed-function
     * convention already used elsewhere in this codebase. */
    GLboolean blend_was_enabled = glIsEnabled( GL_BLEND );
    GLint     prev_blend_src, prev_blend_dst;
    glGetIntegerv( GL_BLEND_SRC, &prev_blend_src );
    glGetIntegerv( GL_BLEND_DST, &prev_blend_dst );
    GLint prev_texture;
    glGetIntegerv( GL_TEXTURE_BINDING_2D, &prev_texture );

    /* GL_CURRENT_RASTER_POSITION (anchor) is an ABSOLUTE window-pixel
     * coordinate. If the caller has an active sub-viewport (e.g. a plot
     * widget restricted to part of the window via glViewport), the ortho
     * bounds must include the viewport's own origin (viewport[0..1]), not
     * just its size -- otherwise quads land at the wrong place entirely
     * (this is what made text vanish in restricted-viewport widgets like
     * the intensity/profile plot: its glyphs were being positioned
     * relative to the sub-viewport's corner instead of the window's). */
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( (double) viewport[0], (double) (viewport[0] + viewport[2]),
             (double) viewport[1], (double) (viewport[1] + viewport[3]),
             -1.0, 1.0 );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, s_texture );

    if( n_verts > 0 )
    {
        glUseProgram( s_program );

        glBindBuffer( GL_ARRAY_BUFFER, s_vbo );
        glBufferData( GL_ARRAY_BUFFER, (GLsizeiptr) (sizeof(GLfloat) * FLOATS_PER_VERTEX * n_verts),
                      verts, GL_DYNAMIC_DRAW );

        GLint loc_position = glGetAttribLocation( s_program, "position" );
        glVertexAttribPointer( loc_position, 3, GL_FLOAT, GL_FALSE,
                                FLOATS_PER_VERTEX * sizeof(GLfloat), (void *) 0 );
        glEnableVertexAttribArray( loc_position );

        GLint loc_texcoord = glGetAttribLocation( s_program, "texcoord" );
        glVertexAttribPointer( loc_texcoord, 2, GL_FLOAT, GL_FALSE,
                                FLOATS_PER_VERTEX * sizeof(GLfloat),
                                (void *) (3 * sizeof(GLfloat)) );
        glEnableVertexAttribArray( loc_texcoord );

        glUniform1i( glGetUniformLocation( s_program, "atlasTex" ), 0 );
        glUniform4f( glGetUniformLocation( s_program, "uColor" ),
                     colour[0], colour[1], colour[2], colour[3] );

        glDrawArrays( GL_TRIANGLES, 0, (GLsizei) n_verts );

        glDisableVertexAttribArray( loc_position );
        glDisableVertexAttribArray( loc_texcoord );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glUseProgram( 0 );

        free( verts );
    }

    glBindTexture( GL_TEXTURE_2D, (GLuint) prev_texture );

    if( !blend_was_enabled )
        glDisable( GL_BLEND );
    glBlendFunc( (GLenum) prev_blend_src, (GLenum) prev_blend_dst );

    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    /* Leave the raster position updated to the end-of-string position,
     * matching the auto-advance behaviour glBitmap/glCallLists gave the
     * old renderer, in case a caller chains further draws off of it. */
    glRasterPos4f( cursor_x, anchor[1], anchor[2], anchor[3] );
}
