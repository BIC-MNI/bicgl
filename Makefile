all: opengl

include ../Makefile.include

OPENGL_X_LIBNAME = opengl_x_graphics

LIB_OPENGL_X = lib$(OPENGL_X_LIBNAME).a

PRINT_X_COLOURS = print_X_colours


GLUT_INCLUDES = -I/usr/include/GL

OPENGL_INCLUDES = -IOpenGL_graphics/Include \
                  -IInclude \
                  -IGLUT_windows/Include $(GLUT_INCLUDES) $(X11_INCLUDES)
OPENGL_TARGETS = $(LIB_OPENGL_X)
OPENGL_-O3_TARGETS = $(LIB_OPENGL_X-O3)


clean_obj:
	\rm -f $(IRISGL_SRC:.c=.o) $(IRISGL_SRC:.c=.u) $(IRISGL_SRC:.c=.ln)
	\rm -f $(OPENGL_X_SRC:.c=.o) $(OPENGL_X_OBJECTS:.c=.u) \
               $(OPENGL_X_SRC:.c=.ln)

OPT = -O


opengl:
	make library "TARGETS=$(OPENGL_TARGETS)" "G_INCLUDE=$(OPENGL_INCLUDES)"\
                     "OPT=$(OPT)" "CC=$(CC)"

INCLUDE = $(G_INCLUDE) $(BIC_PL_INCLUDE)

GLX_SRC = GLX_windows/glx_windows.c \
          GLX_windows/stored_font.c

GLUT_SRC = GLUT_windows/glut_windows.c \
           GLUT_windows/copy_x_colours.c

X_SRC = X_windows/x_windows.c

GL_SRC = \
         IrisGL_graphics/colour_def.c \
         IrisGL_graphics/draw.c \
         IrisGL_graphics/event_loop.c \
         IrisGL_graphics/lights.c \
         IrisGL_graphics/render.c \
         IrisGL_graphics/view.c \
         IrisGL_graphics/windows.c

OPENGL_SRC = \
             OpenGL_graphics/colour_def.c \
             OpenGL_graphics/draw.c \
             OpenGL_graphics/event_loop.c \
             OpenGL_graphics/lights.c \
             OpenGL_graphics/render.c \
             OpenGL_graphics/view.c \
             OpenGL_graphics/windows.c

G_SRC = G_graphics/draw.c \
        G_graphics/draw_objects.c \
        G_graphics/event_loop.c \
        G_graphics/graphics_structs.c \
        G_graphics/lights.c \
        G_graphics/random_order.c \
        G_graphics/render.c \
        G_graphics/view.c \
        G_graphics/windows.c


#OPENGL_X_SRC = $(G_SRC) $(OPENGL_SRC) $(GLX_SRC) $(X_SRC)
OPENGL_X_SRC = $(G_SRC) $(OPENGL_SRC) $(GLUT_SRC)

OPENGL_X_OBJECTS = $(OPENGL_X_SRC:.c=.o)


#-----

PROTOTYPE_FILE = Include/graphics_prototypes.h

$(PROTOTYPE_FILE): $(G_SRC)
	@$(MAKE_PROTOTYPES) $@ $(G_SRC)

#-----

X_PROTOTYPE_FILE = X_windows/Include/x_window_prototypes.h

$(X_PROTOTYPE_FILE): $(X_SRC)
	@$(MAKE_PROTOTYPES) $@ $(X_SRC)

#-----

GLX_PROTOTYPE_FILE = GLX_windows/Include/glx_window_prototypes.h

$(GLX_PROTOTYPE_FILE): $(GLX_SRC)
	@$(MAKE_PROTOTYPES) $@ $(GLX_SRC)

#-----

GLUT_PROTOTYPE_FILE = GLUT_windows/Include/glut_window_prototypes.h

$(GLUT_PROTOTYPE_FILE): $(GLUT_SRC)
	@$(MAKE_PROTOTYPES) $@ $(GLUT_SRC)

#-----

OPENGL_PROTOTYPE_FILE = OpenGL_graphics/Include/opengl_graphics_prototypes.h

$(OPENGL_PROTOTYPE_FILE): $(OPENGL_SRC)
	@$(MAKE_PROTOTYPES) $@ $(OPENGL_SRC)

#-----

GL_PROTOTYPE_FILE = IrisGL_graphics/Include/irisgl_graphics_prototypes.h

$(GL_PROTOTYPE_FILE): $(GL_SRC)
	@$(MAKE_PROTOTYPES) $@ $(GL_SRC)

#-----

$(LIB_OPENGL_X): $(OPENGL_X_OBJECTS)
	\rm -f $@
	$(MAKE_LIBRARY) $@ $(OPENGL_X_OBJECTS)
	$(RANLIB) $@

$(LIB_OPENGL_X-O3): $(OPENGL_X_OBJECTS:.o=.u)
	\rm -f $@
	$(MAKE_LIBRARY) $@ $(OPENGL_X_OBJECTS:.o=.u)
	$(RANLIB) $@

$(LINT_OPENGL_X): $(OPENGL_X_SRC:.c=.ln)
	$(LINT) -x -u -o $(OPENGL_X_LIBNAME) $(OPENGL_X_SRC:.c=.ln)

library: \
         $(PROTOTYPE_FILE) \
         $(GLUT_PROTOTYPE_FILE) \
         $(OPENGL_PROTOTYPE_FILE) \
         $(GL_PROTOTYPE_FILE) \
         $(TARGETS)

lint: \
      $(PROTOTYPE_FILE) \
      $(GLUT_PROTOTYPE_FILE) \
      $(OPENGL_PROTOTYPE_FILE) \
      $(GL_PROTOTYPE_FILE) \
      $(LINT_LIBRARY)

$(OPENGL_X_SRC) $(GL_SRC) \
G_graphics/draw_polygons.include.c \
G_graphics/draw_quadmesh.include.c :
	@$(MAKE_SOURCE_LINKS)

G_graphics/draw.u \
G_graphics/draw.o: G_graphics/draw_polygons.include.c \
                               G_graphics/draw_quadmesh.include.c

$(PRINT_X_COLOURS): GLUT_windows/print_X_colours.c
	$(CC) $(X11_INCLUDES) GLUT_windows/print_X_colours.c -o $@ $(X11_LIBS)
