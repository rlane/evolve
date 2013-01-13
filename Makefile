CFLAGS := -Wall -Werror `sdl-config --cflags` `pkg-config --cflags glew`
LDLIBS := `sdl-config --libs` `pkg-config --libs glew`

all: evolve

evolve: evolve.c critter.c
