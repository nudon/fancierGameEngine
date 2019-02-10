## This is a simple Makefile with lost of comments 
## Check Unix Programming Tools handout for more info.

# Define what compiler to use and the flags.
CC=gcc
CCFLAGS= -g -Wall
SDL_CONFIG  = `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf
LINKFLAGS = -lm $(SDL_CONFIG)
ENDFLAGS = $(LINKFLAGS)
OUTPUT = test
OBJECTS = geometry.o collider.o myVector.o myList.o myMatrix.o text_driver.o graphics.o physics.o body.o game_state.o poltergeist.o input.o parallax.o compound.o

all: 	$(OBJECTS)
	$(CC) $(ENDFLAGS) -o  $(OUTPUT) -g $(OBJECTS)

# Compile all .c files into .o files
# % matches all (like * in a command)
# $< is the source file (.c file)
%.o : %.c
	$(CC) -c $(CCFLAGS) $<



# Build test_mystring if necessary

clean:
	rm -f core *.o $(OUTPUT)

