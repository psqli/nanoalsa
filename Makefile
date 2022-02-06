
# 2022-02-06

libname = libnanoalsa

# Include files ending with .c
sources := $(wildcard *.c)
objects := $(patsubst %.c, %.o, $(sources))
static_library := $(libname).a
shared_library := $(libname).so

# -W  Control display of warnings.
CFLAGS += -Wall
# Generate Position Independent Code
CFLAGS += -fPIC
# Generate debug information for gdb
CFLAGS += -ggdb

# Link as a shared library
LDFLAGS = -shared

all: $(static_library) $(shared_library)

# PCM library

$(static_library): $(objects)
	$(AR) rcs $@ $(objects)

$(shared_library): $(objects)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(objects)

nanoalsa.o: nanoalsa.c nanoalsa.h

# Clean

.PHONY: clean
clean:
	-$(RM) $(objects) $(static_library) $(shared_library)
