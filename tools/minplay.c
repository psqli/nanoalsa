// Copyright (C) 2020  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// 2020-06-05
//
// Read RIFF/wave files from stdin and write PCM data to stdout.
// If stdout is a sound device it's set up.
//
// NOTE: Do not depend on nanoalsa. Compile with: gcc -o minplay minplay.c
//
// E.g.: cat file.wav | ./minplay > /dev/pcmC0D0p
// Playback is resumed after resuming from Stopped state.

#include <stdint.h>    // uint32_t
#include <sys/ioctl.h> // ioctl()
#include <unistd.h>    // read(), write()

#include <sound/asound.h>

struct riff_header  { uint32_t magic, size, type; };
struct chunk_header { uint32_t id,    size; };

struct sound_info {
	uint16_t format, channels;
	uint32_t rate, bytes_per_second;
	uint16_t bytes_per_sample, bits_per_sample;
};

#define RIFF_MAGIC 0x46464952

int
main()
{
	struct riff_header h;
	if (read(0, &h, sizeof(h)) != sizeof(h) || h.magic != RIFF_MAGIC)
		return 1;

	struct chunk_header c; // info chunk header
	if (read(0, &c, sizeof(c)) != sizeof(c))
		return 1;

	struct sound_info i;
	if (read(0, &i, sizeof(i)) != sizeof(i))
		return 1;

	struct chunk_header d; // data chunk header
	if (read(0, &d, sizeof(d)) != sizeof(d))
		return 1;

	struct snd_pcm_hw_params p;
	int j;
	for (j = 0; j < sizeof(p);       j++) *((char*) &p       + j) = 0x00;
	for (j = 0; j < sizeof(p.masks); j++) *((char*) &p.masks + j) = 0xff;
	for (j = 0; j < sizeof(p.intervals) / sizeof(struct snd_interval); j++)
		p.intervals[j] = (struct snd_interval) { 0, -1, 0, 0, 0, 0 };

	p.masks[SNDRV_PCM_HW_PARAM_ACCESS - SNDRV_PCM_HW_PARAM_FIRST_MASK]
	  .bits[0] = 1 << SNDRV_PCM_ACCESS_RW_INTERLEAVED;

	p.intervals[SNDRV_PCM_HW_PARAM_SAMPLE_BITS - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL]
	  = (struct snd_interval) { i.bits_per_sample, i.bits_per_sample, 0, 0, 0, 0 };

	p.intervals[SNDRV_PCM_HW_PARAM_RATE - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL]
	  = (struct snd_interval) { i.rate, i.rate, 0, 0, 0, 0 };

	p.intervals[SNDRV_PCM_HW_PARAM_CHANNELS - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL]
	  = (struct snd_interval) { i.channels, i.channels, 0, 0, 0, 0 };

	if (ioctl(1, SNDRV_PCM_IOCTL_HW_PARAMS, &p) == -1)
		return 1;

	char buffer[16384];
	while (    read(0, buffer, sizeof(buffer)) > 0 &&
	        ( write(1, buffer, sizeof(buffer)) > 0 ||
	          ioctl(1, SNDRV_PCM_IOCTL_PREPARE) == 0 ) );

	return 0;
}
