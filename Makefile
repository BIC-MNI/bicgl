first:
	@echo "Type either:"
	@echo "		make irisgl"
	@echo "		make opengl"
	@echo "		make mesa"

include $(SRC_DIRECTORY)/BIC_PL/Makefile.include

IRISGL_LIBNAME = irisgl_graphics
OPENGL_X_LIBNAME = opengl_x_graphics
MESA_LIBNAME = mesa_x_graphics

LIB_IRISGL = lib$(IRISGL_LIBNAME).a
LIB_IRISGL-O3 = lib$(IRISGL_LIBNAME)-O3.a
LINT_IRISGL = llib-l$(IRISGL_LIBNAME).ln
LIB_OPENGL_X = lib$(OPENGL_X_LIBNAME).a
LIB_OPENGL_X-O3 = lib$(OPENGL_X_LIBNAME)-O3.a
LINT_OPENGL_X = llib-l$(OPENGL_X_LIBNAME).ln
LIB_MESA = lib$(MESA_LIBNAME).a
LIB_MESA-O3 = lib$(MESA_LIBNAME)-O3.a
LINT_MESA = llib-l$(MESA_LIBNAME).ln

IRISGL_INCLUDES = -IInclude -IGL_graphics/Include
IRISGL_TARGETS = $(LIB_IRISGL)
IRISGL_-O3_TARGETS = $(LIB_IRISGL-O3)

OPENGL_INCLUDES = -IInclude \
                  -IOpenGL_graphics/Include \
                  -IGLX_windows/Include \
                  -IX_windows/Include
OPENGL_TARGETS = $(LIB_OPENGL_X)
OPENGL_-O3_TARGETS = $(LIB_OPENGL_X-O3)

MESA_INCLUDES = $(OPENGL_INCLUDES) $(MESA_INCLUDE) -DMESA_OPENGL
MESA_TARGETS = $(LIB_MESA)
MESA_-O3_TARGETS = $(LIB_MESA-O3)

clean_obj:
	\rm -f $(IRISGL_SRC:.c=.o) $(IRISGL_SRC:.c=.u) $(IRISGL_SRC:.c=.ln)
	\rm -f $(OPENGL_X_SRC:.c=.o) $(OPENGL_X_OBJECTS:.c=.u) \
               $(OPENGL_X_SRC:.c=.ln)

OPT = -O

#------  three principal targets

irisgl:
	make library "TARGETS=$(IRISGL_TARGETS)" "G_INCLUDE=$(IRISGL_INCLUDES)"\
                     "OPT=$(OPT)"

irisgl-O3:
	make library "TARGETS=$(IRISGL_-O3_TARGETS)" "G_INCLUDE=$(IRISGL_INCLUDES)"\
                     "OPT=$(OPT)"

opengl:
	make library "TARGETS=$(OPENGL_TARGETS)" "G_INCLUDE=$(OPENGL_INCLUDES)"\
                     "OPT=$(OPT)"

opengl-O3:
	make library "TARGETS=$(OPENGL_-O3_TARGETS)" "G_INCLUDE=$(OPENGL_INCLUDES)"\
                     "OPT=$(OPT)"

mesa:
	make library "TARGETS=$(MESA_TARGETS)" "G_INCLUDE=$(MESA_INCLUDES)"\
                     "OPT=$(OPT)"

mesa-O3:
	make library "TARGETS=$(MESA_-O3_TARGETS)" "G_INCLUDE=$(MESA_INCLUDES)"\
                     "OPT=$(OPT)"

#------  three lint targets

lint_irisgl:
	make lint "LINT_LIBRARY=$(LINT_IRISGL)" "G_INCLUDE=$(IRISGL_INCLUDES)"\
                     "OPT=$(OPT)"

lint_opengl:
	make lint "LINT_LIBRARY=$(LINT_OPENGL_X)" "G_INCLUDE=$(OPENGL_INCLUDES)"\
                     "OPT=$(OPT)"

lint_mesa:
	make lint "LINT_LIBRARY=$(LINT_MESA)" "G_INCLUDE=$(MESA_INCLUDES)"\
                     "OPT=$(OPT)"

INCLUDE = $(G_INCLUDE) $(BIC_PL_INCLUDE)

GLX_SRC = GLX_windows/glx_windows.c \
          GLX_windows/stored_font.c

X_SRC = X_windows/x_windows.c

GL_SRC = \
         GL_graphics/colour_def.c \
         GL_graphics/draw.c \
         GL_graphics/events.c \
         GL_graphics/lights.c \
         GL_graphics/render.c \
         GL_graphics/view.c \
         GL_graphics/windows.c

OPENGL_SRC = \
             OpenGL_graphics/colour_def.c \
             OpenGL_graphics/draw.c \
             OpenGL_graphics/events.c \
             OpenGL_graphics/lights.c \
             OpenGL_graphics/render.c \
             OpenGL_graphics/view.c \
             OpenGL_graphics/windows.c

G_SRC = G_graphics/draw.c \
        G_graphics/draw_objects.c \
        G_graphics/events.c \
        G_graphics/graphics_structs.c \
        G_graphics/lights.c \
        G_graphics/random_order.c \
        G_graphics/render.c \
        G_graphics/view.c \
        G_graphics/windows.c

IRISGL_SRC = $(G_SRC) $(GL_SRC)

OPENGL_X_SRC = $(G_SRC) $(OPENGL_SRC) $(GLX_SRC) $(X_SRC)

OPENGL_X_OBJECTS = $(OPENGL_X_SRC:.c=.o)

IRISGL_OBJECTS = $(IRISGL_SRC:.c=.o)

#-----

PROTOTYPE_FILE = Include/graphics_prototypes.h

$(PROTOTYPE_FILE): $(G_SRC)
	@$(MAKE_DIRECTORY)/create_prototypes.csh $@ $(G_SRC)

#-----

X_PROTOTYPE_FILE = X_windows/Include/x_window_prototypes.h

$(X_PROTOTYPE_FILE): $(X_SRC)
	@$(MAKE_DIRECTORY)/create_prototypes.csh $@ $(X_SRC)

#-----

GLX_PROTOTYPE_FILE = GLX_windows/Include/glx_window_prototypes.h

$(GLX_PROTOTYPE_FILE): $(GLX_SRC)
	@$(MAKE_DIRECTORY)/create_prototypes.csh $@ $(GLX_SRC)

#-----

OPENGL_PROTOTYPE_FILE = OpenGL_graphics/Include/opengl_graphics_prototypes.h

$(OPENGL_PROTOTYPE_FILE): $(OPENGL_SRC)
	@$(MAKE_DIRECTORY)/create_prototypes.csh $@ $(OPENGL_SRC)

#-----

GL_PROTOTYPE_FILE = GL_graphics/Include/irisgl_graphics_prototypes.h

$(GL_PROTOTYPE_FILE): $(GL_SRC)
	@$(MAKE_DIRECTORY)/create_prototypes.csh $@ $(GL_SRC)

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

$(LIB_MESA): $(OPENGL_X_OBJECTS)
	\rm -f $@
	$(MAKE_LIBRARY) $@ $(OPENGL_X_OBJECTS)
	$(RANLIB) $@

$(LIB_MESA-O3): $(OPENGL_X_OBJECTS:.o=.u)
	\rm -f $@
	$(MAKE_LIBRARY) $@ $(OPENGL_X_OBJECTS:.o=.u)
	$(RANLIB) $@

$(LINT_MESA): $(OPENGL_X_SRC:.c=.ln)
	$(LINT) -x -u -o $(MESA_LIBNAME) $(OPENGL_X_SRC:.c=.ln)

$(LIB_IRISGL): $(IRISGL_OBJECTS)
	\rm -f $@
	$(MAKE_LIBRARY) $@ $(IRISGL_OBJECTS)
	$(RANLIB) $@

$(LIB_IRISGL-O3): $(IRISGL_OBJECTS:.o=.u)
	\rm -f $@
	$(MAKE_LIBRARY) $@ $(IRISGL_OBJECTS:.o=.u)
	$(RANLIB) $@

$(LINT_IRISGL): $(IRISGL_SRC:.c=.ln)
	$(LINT) -x -u -o $(IRISGL_LIBNAME) $(IRISGL_SRC:.c=.ln)

library: \
         $(PROTOTYPE_FILE) \
         $(X_PROTOTYPE_FILE) \
         $(GLX_PROTOTYPE_FILE) \
         $(OPENGL_PROTOTYPE_FILE) \
         $(GL_PROTOTYPE_FILE) \
         $(TARGETS)

lint: \
      $(PROTOTYPE_FILE) \
      $(X_PROTOTYPE_FILE) \
      $(GLX_PROTOTYPE_FILE) \
      $(OPENGL_PROTOTYPE_FILE) \
      $(GL_PROTOTYPE_FILE) \
      $(LINT_LIBRARY)
