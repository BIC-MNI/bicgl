include $(C_DEV_DIRECTORY)/Make/Makefile.include
include ../Makefile.include

INCLUDE = -I. $(GRAPHICS_INCLUDE) $(C_UTILS_INCLUDE)
OPT = -g
DEFINE =

all: test_graphics test_minc test_pixels test_polygons \
     test_viewport test_volume two_windows \
     lint_test_graphics lint_test_minc lint_test_pixels lint_test_polygons \
     lint_test_viewport lint_test_volume lint_two_windows

# ----------------------------------------------------------------------------

test_graphics_objects = test_graphics.o

test_graphics: $(test_graphics_objects)
	$(CC) $(test_graphics_objects) -o $@ $(GRAPHICS_LIBS)

lint_test_graphics: $(test_graphics_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_graphics_objects:.o=.ln) \
                                $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_pixels_objects = test_pixels.o

test_pixels: $(test_pixels_objects)
	$(CC) $(test_pixels_objects) -o $@ $(GRAPHICS_LIBS)

lint_test_pixels: $(test_pixels_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_pixels_objects:.o=.ln) \
                         $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_minc_objects = test_minc.o

test_minc: $(test_minc_objects)
	$(CC) $(test_minc_objects) -o $@ $(GRAPHICS_LIBS)

lint_test_minc: $(test_minc_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_minc_objects:.o=.ln) \
                         $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_volume_objects = test_volume.o

test_volume: $(test_volume_objects)
	$(CC) $(test_volume_objects) -o $@ $(GRAPHICS_LIBS)

lint_test_volume: $(test_volume_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_volume_objects:.o=.ln) \
                         $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_viewport_objects = test_viewport.o

test_viewport: $(test_viewport_objects)
	$(CC) $(test_viewport_objects) -o $@ $(GRAPHICS_LIBS)

lint_test_viewport: $(test_viewport_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_viewport_objects:.o=.ln) \
                         $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

two_windows_objects = two_windows.o

two_windows: $(two_windows_objects)
	$(CC) $(two_windows_objects) -o $@ $(GRAPHICS_LIBS)

lint_two_windows: $(two_windows_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(two_windows_objects:.o=.ln) \
                         $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_polygons_objects = test_polygons.o

test_polygons: $(test_polygons_objects)
	$(CC) $(test_polygons_objects) -o $@ $(GRAPHICS_LIBS)

lint_test_polygons: $(test_polygons_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_polygons_objects:.o=.ln) $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_overlays_objects = test_overlays.o

test_overlays: $(test_overlays_objects)
	$(CC) $(test_overlays_objects) -o $@ $(GRAPHICS_LIBS)

lint_test_overlays: $(test_overlays_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_overlays_objects:.o=.ln) $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_colours_objects = test_colours.o

test_colours: $(test_colours_objects)
	$(CC) $(test_colours_objects) -o $@ $(GRAPHICS_LIBS)

lint_test_colours: $(test_colours_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_colours_objects:.o=.ln) $(GRAPHICS_LINT_LIBS)
