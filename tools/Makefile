
# 2019-01-10

# Include files ending with .c
sources := $(wildcard *.c)
objects := $(patsubst %.c, %.o, $(sources))

# -W  Control display of warnings.
#CFLAGS += -Wall
# Generate Position Independent Code
CFLAGS += -fPIC
# Generate debug information for gdb
CFLAGS += -ggdb
# -I  Add a search path for headers.
CFLAGS += -I..

# -L          Add a search path for libraries.
LDFLAGS += -L..
# -Wl,-rpath  Add a search path to runtime linker.
LDFLAGS += -Wl,-rpath=.
LDFLAGS += -Wl,-rpath=..

# Libraries to link
LDLIBS = -lnanoalsa

# Additional path(s) to search for prerequisites
VPATH = ..

all: waveplay stdplay minplay

# Play wave (.wav) files

waveplay: waveplay.o

waveplay.o: waveplay.c pcm.h riff.h riff_wave.h

stdplay: stdplay.o

stdplay.o: stdplay.c pcm.h riff.h riff_wave.h

minplay: minplay.o

minplay.o: minplay.c

# Clean

.PHONY: clean
clean:
	-$(RM) $(objects)
