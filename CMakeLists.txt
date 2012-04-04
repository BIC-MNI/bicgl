INCLUDE_DIRECTORIES(${GLUT_INCLUDE_DIR}  OpenGL_graphics/Include GLUT_windows/Include Include )


SET(PRINT_X_COLOURS print_X_colours)



SET(GLX_SRC GLX_windows/glx_windows.c 
          GLX_windows/stored_font.c )

SET(GLUT_SRC GLUT_windows/glut_windows.c 
           GLUT_windows/copy_x_colours.c)

SET(X_SRC   X_windows/x_windows.c )

SET(GL_SRC 
         IrisGL_graphics/colour_def.c 
         IrisGL_graphics/draw.c 
         IrisGL_graphics/event_loop.c 
         IrisGL_graphics/lights.c 
         IrisGL_graphics/render.c 
         IrisGL_graphics/view.c 
         IrisGL_graphics/windows.c )

SET(OPENGL_SRC 
             OpenGL_graphics/colour_def.c 
             OpenGL_graphics/draw.c 
             OpenGL_graphics/event_loop.c 
             OpenGL_graphics/lights.c 
             OpenGL_graphics/render.c 
             OpenGL_graphics/view.c 
             OpenGL_graphics/windows.c)

SET(G_SRC G_graphics/draw.c 
        G_graphics/draw_objects.c 
        G_graphics/event_loop.c 
        G_graphics/graphics_structs.c 
        G_graphics/lights.c 
        G_graphics/random_order.c 
        G_graphics/render.c 
        G_graphics/view.c 
        G_graphics/windows.c)


SET(OPENGL_X_SRC ${G_SRC} ${OPENGL_SRC} ${GLUT_SRC})


ADD_LIBRARY(bicgl 
${OPENGL_X_SRC}
)

TARGET_LINK_LIBRARIES(bicgl ${GLUT_LIBRARIES})
