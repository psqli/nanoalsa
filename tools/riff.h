// Copyright (C) 2018  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// 2018-12-15
//
// Functions to read RIFF (Resource Interchange File Format)
// files.
//
// File structure:
//
// +-------------+
// |riff_header  |  File header
// +-------------+
// |chunk_header | \
// +-------------+  Chunk
// |data         | /
// +-------------+
// There may be multiple chunks.
//
// The RIFF header has a magic number (to know that the
// file is a RIFF one), the size of the file, and the type
// of content it has (e.g. wave).
//
// The chunk header has the type of the chunk and the chunk
// size.
//
// A wave file, for example, have one chunk for sound
// parameters, another for sound data.

#include <errno.h>  // errno variable
#include <stdint.h> // uint32_t
#include <unistd.h> // lseek(), read()

#define RIFF_MAGIC 0x46464952

// File header
struct riff_header {
	uint32_t magic;
	uint32_t size;
	uint32_t type;
};

// Chunk header
struct chunk_header {
	uint32_t id;   // what is the chunk about
	uint32_t size; // size of the chunk
};

// errno is used for error checking
#define riff_for_each_chunk(fd, header) \
	for (errno = 0, lseek(fd, sizeof(struct riff_header), SEEK_SET); \
	     !errno && read(fd, header, sizeof(*(header))) == \
	               sizeof(*(header)); \
	     errno = 0, lseek(fd, (header)->size, SEEK_CUR))

static int
riff_get_header(int fd, struct riff_header *header)
{
	if (read(fd, header, sizeof(*header)) != sizeof(*header))
		return -1;

	if (header->magic != RIFF_MAGIC)
		return -1;

	return 0;
}

// Seek to the chunk chunk_id
static uint32_t
riff_seek(int fd, uint32_t chunk_id)
{
	struct chunk_header header;

	riff_for_each_chunk (fd, &header) {
		if (header.id == chunk_id)
			break;
	}

	if (errno)
		return -1;

	return header.size;
}
