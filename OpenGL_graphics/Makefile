TWO_D = -DTWO_D_ONLY
TWO_D =

LIB_OPENGL    = libopengl$(TWO_D).a
LINT_OPENGL  = llib-opengl.ln
PROTOTYPE_FILE = Include/opengl_prototypes.h

OPT = -g $(TWO_D)

all: $(PROTOTYPE_FILE) $(LIB_OPENGL) $(LINT_OPENGL)

include Makefile.include
include Makefile.objects

INCLUDE = $(VOLUME_IO_INCLUDE) $(BIC_PL_INCLUDE) -IInclude \
           -I../X_windows/Include -I../GLX_windows/Include -I../Include

LINTFLAGS = $(TWO_D) $(INCLUDE)

list_all_objects:
	@echo $(OBJECTS) $(OBJECTS:.o=.ln)

$(PROTOTYPE_FILE): $(OBJECTS:.o=.c)
	@echo "#ifndef  DEF_OPENGL_PROTOTYPES"         >  $@
	@echo "#define  DEF_OPENGL_PROTOTYPES"         >> $@
	@extract_functions -public $(OBJECTS:.o=.c)      >> $@
	@echo "#endif"                                   >> $@

$(LIB_OPENGL): $(OBJECTS)
	@if( -e $@ ) \rm -f $@
	$(MAKE_LIBRARY) $@ $(OBJECTS)
	$(RANLIB) $@

$(LINT_OPENGL): $(OBJECTS:.o=.ln)
	@echo "--- Linting ---"
	$(LINT) -x -u -o opengl $(OBJECTS:.o=.ln)
