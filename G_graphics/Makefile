TWO_D = -DTWO_D_ONLY
TWO_D =

PROTOTYPE_FILE = Include/g_graphics_prototypes.h

OPT = -O $(TWO_D)

OBJECTS = events.o \
          draw.o \
          graphics_structs.o

all: $(PROTOTYPE_FILE) $(OBJECTS)

include ../Makefile.include

INCLUDE = $(BIC_PL_INCLUDE) -IInclude \
           -I../X_windows/Include -I../GLX_windows/Include

LINTFLAGS = $(TWO_D) $(INCLUDE)

$(PROTOTYPE_FILE): $(OBJECTS:.o=.c)
	@echo "#ifndef  DEF_G_GRAPHICS_PROTOTYPES"         >  $@
	@echo "#define  DEF_G_GRAPHICS_PROTOTYPES"         >> $@
	@extract_functions -public $(OBJECTS:.o=.c)      >> $@
	@echo "#endif"                                   >> $@
