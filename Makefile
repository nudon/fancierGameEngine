## This is a simple Makefile with lost of comments 
## Check Unix Programming Tools handout for more info.

# Define what compiler to use and the flags.
CC=gcc
CCFLAGS= -g -Wall  `xml2-config --cflags`
SDL_CONFIG  = `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf
LIBXML_CONFIG = `xml2-config --cflags --libs` 
LINKFLAGS = -lm $(SDL_CONFIG) $(LIBXML_CONFIG)
ENDFLAGS = $(LINKFLAGS)
OUTPUT = test
OBJECTS = geometry.o collider.o myVector.o myList.o myMatrix.o text_driver.o graphics.o physics.o body.o game_state.o poltergeist.o input.o parallax.o compound.o events.o map_io.o plane.o

all: 	$(OBJECTS)
	$(CC) $(ENDFLAGS) -o  $(OUTPUT) -g $(OBJECTS)

map_io.o : map_io.c
	$(CC) -c $(CCFLAGS) $(LIBXML_CONFIG) $<

# Compile all .c files into .o files
# % matches all (like * in a command)
# $< is the source file (.c file)
%.o : %.c
	$(CC) -c $(CCFLAGS) $<



# Build test_mystring if necessary

clean:
	rm -f core *.o $(OUTPUT)

