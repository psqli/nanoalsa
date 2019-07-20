
# 2019-01-09

libname = libnanoalsa

# Include files ending with .c
sources := $(wildcard *.c)
objects := $(patsubst %.c, %.o, $(sources))

# -W  Control display of warnings.
CFLAGS += -Wall
# Generate Position Independent Code
CFLAGS += -fPIC
# Generate debug information for gdb
CFLAGS += -ggdb

# Link as a shared library
LDFLAGS = -shared

all: static_library shared_library

# PCM library

static_library: $(objects)
	$(AR) rcs $(libname).a $(objects)

shared_library: $(objects)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(libname).so $(objects)

hardware_parameters.o: hardware_parameters.c

setup.o: setup.c operations.h setup.h

operations.o: operations.c operations.h

open.o: open.c open.h

# Clean

.PHONY: clean
clean:
	-$(RM) $(objects)
