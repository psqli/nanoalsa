// Copyright (C) 2020  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// 2020-06-03
//
// Read RIFF/wave files from stdin and write PCM data to stdout.
// If stdout is a sound device it's set up.
//
// E.g.: cat file.wav | ./stdplay > /dev/pcmC0D0p

#include <unistd.h> // read(), write()

#include "pcm.h"
#include "riff.h"
#include "riff_wave.h"

int
main()
{
	struct riff_header  r;
	if (riff_get_header(0, &r) || r.type != RIFF_TYPE_WAVE)
		return 1;

	struct chunk_header c;
	struct sound_info   i;
	if (read(0, &c, sizeof(c)) != sizeof(c) || c.id != CHUNK_INFO ||
	    read(0, &i, sizeof(i)) != sizeof(i) ||
	    read(0, &c, sizeof(c)) != sizeof(c) || c.id != CHUNK_DATA)
		return 1;

	struct pcm_hw_params p;
	pcm_hw_params_init(&p);
	pcm_set(&p, PCM_ACCESS,      PCM_ACCESS_RW);
	pcm_set(&p, PCM_SAMPLE_BITS, i.bits_per_sample);
	pcm_set(&p, PCM_RATE,        i.rate);
	pcm_set(&p, PCM_CHANNELS,    i.channels);
	// Do not fail on setup because user may be writing to a regular file.
	pcm_hw_params_setup(1, &p);

	char buffer[8192];
	while ( read(0, buffer, sizeof(buffer)) > 0 &&
	       write(1, buffer, sizeof(buffer)) > 0);

	return 0;
}
