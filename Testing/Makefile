def:
	echo "Specify a target"

include ../Makefile.include

INCLUDE = -I. $(GRAPHICS_INCLUDE) $(BICPL_INCLUDE)
LIBS = $(BICPL_LIBS) $(IRISGL_GRAPHICS_LIBS)
LIBS-O3 = $(BICPL_LIBS-O3) $(IRISGL_GRAPHICS_LIBS-O3)
OPT = -O
DEFINE =

# ----------------------------------------------------------------------------

test_graphics_objects = test_graphics.o

test_graphics.irisgl: $(test_graphics_objects)
	$(CC) $(OPT) $(test_graphics_objects) -o $@ $(IRISGL_GRAPHICS_LIBS)

test_graphics.opengl: $(test_graphics_objects)
	$(CC) $(OPT) $(test_graphics_objects) -o $@ $(OPENGL_GRAPHICS_LIBS)

test_graphics-O3.opengl: $(test_graphics_objects:.o=.u)
	$(CC) -O3 $(test_graphics_objects:.o=.u) -o $@ $(OPENGL_GRAPHICS_LIBS-O3)

test_graphics.mesa: $(test_graphics_objects)
	$(CC) $(OPT) $(test_graphics_objects) -o $@ $(MESA_GRAPHICS_LIBS)

test_graphics-O3.mesa: $(test_graphics_objects:.o=.u)
	$(CC) -O3 $(OPT) $(test_graphics_objects:.o=.u) -o $@ $(MESA_GRAPHICS_LIBS-O3)

lint_test_graphics: $(test_graphics_objects:.o=.ln)
	$(LINT) -x -u $(LINTFLAGS) $(test_graphics_objects:.o=.ln) \
                                $(GRAPHICS_LINT_LIBS)

debug.mesa: debug.o
	$(CC) $(OPT) debug.o -o $@ $(MESA_GRAPHICS_LIBS)


# ----------------------------------------------------------------------------

new_test_graphics_objects = new_test_graphics.o

new_test_graphics.irisgl: $(new_test_graphics_objects)
	$(CC) $(OPT) $(new_test_graphics_objects) -o $@ $(IRISGL_GRAPHICS_LIBS)

new_test_graphics.opengl: $(new_test_graphics_objects)
	$(CC) $(OPT) $(new_test_graphics_objects) -o $@ $(OPENGL_GRAPHICS_LIBS)

new_test_graphics-O3.opengl: $(new_test_graphics_objects:.o=.u)
	$(CC) -O3 $(new_test_graphics_objects:.o=.u) -o $@ $(OPENGL_GRAPHICS_LIBS-O3)

new_test_graphics.mesa: new_test_graphics.c
	cc -fullwarn -n32 -mips4 -woff 1174 -c -g -I. -I/data/nil/david/Source/GLUT/include  -I/data/nil/david/Source/Mesa/include -I/usr/local/include      new_test_graphics.c -o new_test_graphics.o
	cc -fullwarn -n32 -mips4 -woff 1174 -g new_test_graphics.o -o $@ -L/data/nil/david/Source/GLUT/IRIX_6 -lglut -L/data/nil/david/Source/Mesa/IRIX_6 -lMesaGL  -lMesaGLU -lX11 -lXext  -lXmu -lm -lmalloc

new_test_graphics.mesa5: new_test_graphics.c
	cc -prototypes -fullwarn -c -g -I. -I/data/nil/david/Source/GLUT/include  -I/data/nil/david/Source/Mesa/include -I/usr/local/include      new_test_graphics.c -o new_test_graphics.o
	cc -prototypes -fullwarn -g new_test_graphics.o -o $@ -L/data/nil/david/Source/GLUT/IRIX_5 -lglut -L/data/nil/david/Source/Mesa/IRIX_5 -lMesaGL  -lMesaGLU -lX11 -lXext -lXmu  -lm


new_test_graphics.opengl5: new_test_graphics.c
	cc -prototypes -fullwarn -c -g -I. -I/data/nil/david/Source/GLUT/include  -I/usr/local/include      new_test_graphics.c -o new_test_graphics.o
	cc -prototypes -fullwarn -g new_test_graphics.o -o $@ -L/data/nil/david/Source/GLUT/IRIX_5 -lglut -lGL -lGLU -lX11 -lXext -lXmu  -lm


new_test_graphics-O3.mesa: $(new_test_graphics_objects:.o=.u)
	$(CC) -O3 $(OPT) $(new_test_graphics_objects:.o=.u) -o $@ $(MESA_GRAPHICS_LIBS-O3)

lint_new_test_graphics: $(new_test_graphics_objects:.o=.ln)
	$(LINT) -x -u $(LINTFLAGS) $(new_test_graphics_objects:.o=.ln) \
                                $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_colour_map_mode_objects = test_colour_map_mode.o

test_colour_map_mode.irisgl: $(test_colour_map_mode_objects)
	$(CC) $(OPT) $(test_colour_map_mode_objects) -o $@ $(IRISGL_GRAPHICS_LIBS)

test_colour_map_mode.opengl: $(test_colour_map_mode_objects)
	$(CC) $(OPT) $(test_colour_map_mode_objects) -o $@ $(OPENGL_GRAPHICS_LIBS)

test_colour_map_mode-O3.opengl: $(test_colour_map_mode_objects:.o=.u)
	$(CC) -O3 $(test_colour_map_mode_objects:.o=.u) -o $@ $(OPENGL_GRAPHICS_LIBS-O3)

test_colour_map_mode.mesa: $(test_colour_map_mode_objects)
	$(CC) $(OPT) $(test_colour_map_mode_objects) -o $@ $(MESA_GRAPHICS_LIBS)

test_colour_map_mode-O3.mesa: $(test_colour_map_mode_objects:.o=.u)
	$(CC) -O3 $(OPT) $(test_colour_map_mode_objects:.o=.u) -o $@ $(MESA_GRAPHICS_LIBS-O3)

lint_test_colour_map_mode: $(test_colour_map_mode_objects:.o=.ln)
	$(LINT) -x -u $(LINTFLAGS) $(test_colour_map_mode_objects:.o=.ln) \
                                $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_viewport_objects = test_viewport.o

test_viewport.irisgl: $(test_viewport_objects)
	$(CC) $(test_viewport_objects) -o $@ $(IRISGL_GRAPHICS_LIBS)

test_viewport.opengl: $(test_viewport_objects)
	$(CC) $(test_viewport_objects) -o $@ $(OPENGL_GRAPHICS_LIBS)

test_viewport.mesa: $(test_viewport_objects)
	$(CC) $(test_viewport_objects) -o $@ $(MESA_GRAPHICS_LIBS)

lint_test_viewport: $(test_viewport_objects:.o=.ln)
	$(LINT) -x -u $(LINTFLAGS) $(test_viewport_objects:.o=.ln) \
                                $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_colour_map_objects = test_colour_map.o

test_colour_map.irisgl: $(test_colour_map_objects)
	$(CC) $(test_colour_map_objects) -o $@ $(IRISGL_GRAPHICS_LIBS)

test_colour_map.mesa: $(test_colour_map_objects)
	$(CC) $(test_colour_map_objects) -o $@ $(MESA_GRAPHICS_LIBS)

lint_test_colour_map: $(test_colour_map_objects:.o=.ln)
	$(LINT) -x -u $(LINTFLAGS) $(test_colour_map_objects:.o=.ln) \
                                $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

example_graphics_objects = example_graphics.o

example_graphics: $(example_graphics_objects)
	$(CC) $(example_graphics_objects) -o $@ $(LIBS)

lint_example_graphics: $(example_graphics_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(example_graphics_objects:.o=.ln) \
                                $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_pixels_objects = test_pixels.o

test_pixels: $(test_pixels_objects)
	$(CC) $(test_pixels_objects) -o $@ $(LIBS)

lint_test_pixels: $(test_pixels_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_pixels_objects:.o=.ln) \
                         $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_minc_objects = test_minc.o

test_minc: $(test_minc_objects)
	$(CC) $(test_minc_objects) -o $@ $(LIBS)

lint_test_minc: $(test_minc_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_minc_objects:.o=.ln) \
                         $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_volume_objects = test_volume.o

test_volume: $(test_volume_objects)
	$(CC) $(test_volume_objects) -o $@ $(IRISGL_GRAPHICS_LIBS)

lint_test_volume: $(test_volume_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_volume_objects:.o=.ln) \
                         $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_rgb_volume_objects = test_rgb_volume.o

test_rgb_volume: $(test_rgb_volume_objects)
	$(CC) $(test_rgb_volume_objects) -o $@ $(LIBS)

lint_test_rgb_volume: $(test_rgb_volume_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_rgb_volume_objects:.o=.ln) \
                         $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

two_windows_objects = two_windows.o

two_windows: $(two_windows_objects)
	$(CC) $(two_windows_objects) -o $@ $(LIBS)

lint_two_windows: $(two_windows_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(two_windows_objects:.o=.ln) \
                         $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_polygons_objects = test_polygons.o

test_polygons: $(test_polygons_objects)
	$(CC) $(test_polygons_objects) -o $@ $(LIBS)

lint_test_polygons: $(test_polygons_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_polygons_objects:.o=.ln) $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_overlays_objects = test_overlays.o

test_overlays: $(test_overlays_objects)
	$(CC) $(test_overlays_objects) -o $@ $(LIBS)

lint_test_overlays: $(test_overlays_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_overlays_objects:.o=.ln) $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

test_colours_objects = test_colours.o

test_colours: $(test_colours_objects)
	$(CC) $(test_colours_objects) -o $@ $(LIBS)

lint_test_colours: $(test_colours_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(test_colours_objects:.o=.ln) $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

film_loop_objects = film_loop.o

film_loop: $(film_loop_objects)
	$(CC) $(film_loop_objects) -o $@ $(LIBS)

lint_film_loop: $(film_loop_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(film_loop_objects:.o=.ln) \
                                $(GRAPHICS_LINT_LIBS)

# ----------------------------------------------------------------------------

add_text_to_pixmap_objects = add_text_to_pixmap.o

add_text_to_pixmap: $(add_text_to_pixmap_objects)
	$(CC) $(add_text_to_pixmap_objects) -o $@ $(LIBS)

add_text_to_pixmap-O3: $(add_text_to_pixmap_objects:.o=.u)
	$(CC) -O3 $(add_text_to_pixmap_objects:.o=.u) -o $@ $(LIBS-O3)

lint_add_text_to_pixmap: $(add_text_to_pixmap_objects:.o=.ln)
	$(LINT) -u $(LINTFLAGS) $(add_text_to_pixmap_objects:.o=.ln) \
                                $(GRAPHICS_LINT_LIBS)
# ----------------------------------------------------------------------------
