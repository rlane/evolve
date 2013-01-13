CFLAGS := -Wall -Werror `sdl-config --cflags` `pkg-config --cflags glew glu`
LDLIBS := `sdl-config --libs` `pkg-config --libs glew glu`

all: evolve

evolve: evolve.c critter.c renderer.c
