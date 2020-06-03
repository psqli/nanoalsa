// Copyright (C) 2020  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// 2020-06-04

#define RIFF_TYPE_WAVE 0x45564157

#define CHUNK_INFO  0x20746d66
#define CHUNK_DATA  0x61746164

// info chunk
struct sound_info {
	uint16_t format;
	uint16_t channels;
	uint32_t rate;
	uint32_t bytes_per_second;
	uint16_t bytes_per_sample;
	uint16_t bits_per_sample;
};
