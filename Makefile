first:  all

include $(LIB_DIRECTORY)/BIC_PL/Makefile.include

IRISGL_LIBNAME = irisgl_graphics
OPENGL_X_LIBNAME = opengl_x_graphics

LIB_IRISGL = lib$(IRISGL_LIBNAME).a
LINT_IRISGL = llib-l$(IRISGL_LIBNAME).ln
LIB_OPENGL_X = lib$(OPENGL_X_LIBNAME).a
LINT_OPENGL_X = llib-l$(OPENGL_X_LIBNAME).ln

IRISGL_INCLUDES = -IInclude -IGL_graphics/Include
IRISGL_TARGETS = $(LIB_IRISGL) $(LINT_IRISGL)

OPENGL_INCLUDES = -IInclude \
                  -IOpenGL_graphics/Include \
                  -IGLX_windows/Include \
                  -IX_windows/Include
OPENGL_TARGETS = $(LIB_OPENGL_X) $(LINT_OPENGL_X)

clean_obj:
	\rm -f $(IRISGL_OBJECTS) $(IRISGL_OBJECTS:.o=.ln)
	\rm -f $(OPENGL_X_OBJECTS) $(OPENGL_X_OBJECTS:.o=.ln)

#------ these two lines must be chosen

OPT = -g

#TARGETS = $(IRISGL_TARGETS)
#G_INCLUDE = $(IRISGL_INCLUDES)

TARGETS = $(OPENGL_TARGETS)
G_INCLUDE = $(OPENGL_INCLUDES)

INCLUDE = $(G_INCLUDE) $(BIC_PL_INCLUDE)

GLX_OBJECTS = GLX_windows/glx_windows.o

X_OBJECTS = X_windows/x_windows.o

GL_OBJECTS = GL_graphics/draw.o \
             GL_graphics/events.o \
             GL_graphics/lights.o \
             GL_graphics/render.o \
             GL_graphics/view.o \
             GL_graphics/windows.o

OPENGL_OBJECTS = OpenGL_graphics/draw.o \
                 OpenGL_graphics/events.o \
                 OpenGL_graphics/lights.o \
                 OpenGL_graphics/render.o \
                 OpenGL_graphics/view.o \
                 OpenGL_graphics/windows.o

G_OBJECTS = G_graphics/draw.o \
            G_graphics/draw_objects.o \
            G_graphics/events.o \
            G_graphics/graphics_structs.o \
            G_graphics/lights.o \
            G_graphics/random_order.o \
            G_graphics/render.o \
            G_graphics/view.o \
            G_graphics/windows.o

GL_INCLUDES = -IInclude -IGL_graphics/Include

OPENGL_INCLUDES = -IInclude -IOpenGL_graphics/Include \
                  -IGLX_windows/Include \
                  -IX_windows/Include

IRISGL_OBJECTS = $(G_OBJECTS) $(GL_OBJECTS)

OPENGL_X_OBJECTS = $(G_OBJECTS) $(OPENGL_OBJECTS) $(GLX_OBJECTS) $(X_OBJECTS)

#-----

PROTOTYPE_FILE = Include/graphics_prototypes.h

$(PROTOTYPE_FILE): $(G_OBJECTS:.o=.c)
	@echo "#ifndef  DEF_GRAPHICS_PROTOTYPES"         >  $@
	@echo "#define  DEF_GRAPHICS_PROTOTYPES"         >> $@
	@extract_functions -public $(G_OBJECTS:.o=.c)      >> $@
	@echo "#endif"                                   >> $@

#-----

X_PROTOTYPE_FILE = X_windows/Include/x_window_prototypes.h

$(X_PROTOTYPE_FILE): $(X_OBJECTS:.o=.c)
	@echo "#ifndef  DEF_X_WINDOW_PROTOTYPES"         >  $@
	@echo "#define  DEF_X_WINDOW_PROTOTYPES"         >> $@
	@extract_functions -public $(X_OBJECTS:.o=.c)    >> $@
	@echo "#endif"                                   >> $@

#-----

GLX_PROTOTYPE_FILE = GLX_windows/Include/glx_window_prototypes.h

$(GLX_PROTOTYPE_FILE): $(GLX_OBJECTS:.o=.c)
	@echo "#ifndef  DEF_GLX_WINDOW_PROTOTYPES"         >  $@
	@echo "#define  DEF_GLX_WINDOW_PROTOTYPES"         >> $@
	@extract_functions -public $(GLX_OBJECTS:.o=.c)      >> $@
	@echo "#endif"                                   >> $@

#-----

OPENGL_PROTOTYPE_FILE = OpenGL_graphics/Include/opengl_graphics_prototypes.h

$(OPENGL_PROTOTYPE_FILE): $(OPENGL_OBJECTS:.o=.c)
	@echo "#ifndef  DEF_OPENGL_GRAPHICS_PROTOTYPES"         >  $@
	@echo "#define  DEF_OPENGL_GRAPHICS_PROTOTYPES"         >> $@
	@extract_functions -public $(OPENGL_OBJECTS:.o=.c)      >> $@
	@echo "#endif"                                   >> $@

#-----

GL_PROTOTYPE_FILE = GL_graphics/Include/irisgl_graphics_prototypes.h

$(GL_PROTOTYPE_FILE): $(GL_OBJECTS:.o=.c)
	@echo "#ifndef  DEF_GL_GRAPHICS_PROTOTYPES"         >  $@
	@echo "#define  DEF_GL_GRAPHICS_PROTOTYPES"         >> $@
	@extract_functions -public $(GL_OBJECTS:.o=.c)      >> $@
	@echo "#endif"                                   >> $@

#-----

$(LIB_OPENGL_X): $(OPENGL_X_OBJECTS)
	@if( -e $@ ) \rm -f $@
	$(MAKE_LIBRARY) $@ $(OPENGL_X_OBJECTS)
	$(RANLIB) $@

$(LINT_OPENGL_X): $(OPENGL_X_OBJECTS:.o=.ln)
	@echo "--- Linting ---"
	$(LINT) -x -u -o $(OPENGL_X_LIBNAME) $(OPENGL_X_OBJECTS:.o=.ln)

$(LIB_IRISGL): $(IRISGL_OBJECTS)
	@if( -e $@ ) \rm -f $@
	$(MAKE_LIBRARY) $@ $(IRISGL_OBJECTS)
	$(RANLIB) $@

$(LINT_IRISGL): $(IRISGL_OBJECTS:.o=.ln)
	@echo "--- Linting ---"
	$(LINT) -x -u -o $(IRISGL_LIBNAME) $(IRISGL_OBJECTS:.o=.ln)

all: \
     $(PROTOTYPE_FILE) \
     $(X_PROTOTYPE_FILE) \
     $(GLX_PROTOTYPE_FILE) \
     $(OPENGL_PROTOTYPE_FILE) \
     $(GL_PROTOTYPE_FILE) \
     $(TARGETS)

