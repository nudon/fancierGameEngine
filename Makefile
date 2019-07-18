CC=gcc
CCFLAGS= -g -O0 -Wall
SDL_CONFIG  = `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf
LIBXML_CONFIG = `xml2-config --cflags --libs` 
LINKFLAGS = -lm $(SDL_CONFIG) $(LIBXML_CONFIG)
ENDFLAGS = $(LINKFLAGS)
OUTPUT = test
OBJECTS = geometry.o collider.o myVector.o myList.o myMatrix.o text_driver.o graphics.o physics.o body.o game_state.o poltergeist.o input.o parallax.o compound.o events.o map_io.o plane.o picture.o attributes.o gi.o creations.o map.o util.o hash_table.o

SRCDIR = ./src/

all: 	$(addprefix $(SRCDIR),$(OBJECTS))
	cd $(SRCDIR) && $(CC) $(ENDFLAGS) -o  ../$(OUTPUT) -g $(OBJECTS)

$(SRCDIR)map_io.o : $(SRCDIR)map_io.c
	$(CC) -c -o $@ $(CCFLAGS) $(LIBXML_CONFIG) $<

$(SRCDIR)%.o : $(SRCDIR)%.c
	$(CC) -c -o $@ $(CCFLAGS) $<

clean:
	cd $(SRCDIR) && rm -f core *.o $(OUTPUT)

