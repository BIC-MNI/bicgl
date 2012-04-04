SET(PRINT_X_COLOURS print_X_colours)

SET(GLX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/GLX_windows/glx_windows.c 
          ${CMAKE_CURRENT_SOURCE_DIR}/GLX_windows/stored_font.c )

SET(GLUT_SRC 
  ${CMAKE_CURRENT_SOURCE_DIR}/GLUT_windows/glut_windows.c 
  ${CMAKE_CURRENT_SOURCE_DIR}/GLUT_windows/copy_x_colours.c)

SET(X_SRC   ${CMAKE_CURRENT_SOURCE_DIR}/X_windows/x_windows.c )

SET(GL_SRC 
         ${CMAKE_CURRENT_SOURCE_DIR}/IrisGL_graphics/colour_def.c 
         ${CMAKE_CURRENT_SOURCE_DIR}/IrisGL_graphics/draw.c 
         ${CMAKE_CURRENT_SOURCE_DIR}/IrisGL_graphics/event_loop.c 
         ${CMAKE_CURRENT_SOURCE_DIR}/IrisGL_graphics/lights.c 
         ${CMAKE_CURRENT_SOURCE_DIR}/IrisGL_graphics/render.c 
         ${CMAKE_CURRENT_SOURCE_DIR}/IrisGL_graphics/view.c 
         ${CMAKE_CURRENT_SOURCE_DIR}/IrisGL_graphics/windows.c )

SET(OPENGL_SRC 
             ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL_graphics/colour_def.c 
             ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL_graphics/draw.c 
             ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL_graphics/event_loop.c 
             ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL_graphics/lights.c 
             ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL_graphics/render.c 
             ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL_graphics/view.c 
             ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL_graphics/windows.c
        )

SET(G_SRC 
        ${CMAKE_CURRENT_SOURCE_DIR}/G_graphics/draw.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/G_graphics/draw_objects.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/G_graphics/event_loop.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/G_graphics/graphics_structs.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/G_graphics/lights.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/G_graphics/random_order.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/G_graphics/render.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/G_graphics/view.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/G_graphics/windows.c)


SET(OPENGL_X_SRC ${G_SRC} ${OPENGL_SRC} ${GLUT_SRC})


SET(BICGL_SRC 
  ${OPENGL_X_SRC}
)

