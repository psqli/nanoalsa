// Copyright (C) 2018-2020  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// 2018-12-15
//
// Play wave (.wav) files.
//
// Wave files (.wav) are RIFF (Resource Interchange File
// Format) files with information about the audio and the
// audio itself.
//
// Example of a file:
//
// +-------------+
// |riff_header  |  File header
// +-------------+
// |chunk_header | \
// +-------------+  Information chunk (struct sound_info)
// |sound_info   | /
// +-------------+
// |chunk_header | \
// +-------------+  Data chunk
// |sound data   | /
// +-------------+

#include <fcntl.h>    // open()
#include <signal.h>   // signal()
#include <stdio.h>    // perror()
#include <stdlib.h>   // malloc()
#include <string.h>   // strcmp()
#include <sys/stat.h> // open()
#include <unistd.h>   // read()

#include "pcm.h"
#include "riff.h"
#include "riff_wave.h"

static volatile sig_atomic_t        keep_running = 1;
static void on_sigint(int signum) { keep_running = 0; }

// Put sound parameters in `cfg`, seek to the sound data
// and return its length.
static int
wave_setup(int fd, struct pcm_hw_params *cfg)
{
	struct riff_header riff;
	struct sound_info info;
	int length;

	if (riff_get_header(fd, &riff) == -1)
		return -1;

	if (riff.type != RIFF_TYPE_WAVE)
		return -1;

	// riff_seek() goes to the start of a chunk and
	// returns its size

	// go to the start of information chunk
	if (riff_seek(fd, CHUNK_INFO) < sizeof(info))
		return -1;

	// read information chunk
	if (read(fd, &info, sizeof(info)) != sizeof(info))
		return -1;

	pcm_set(cfg, PCM_SAMPLE_BITS, info.bits_per_sample);
	pcm_set(cfg, PCM_RATE,        info.rate);
	pcm_set(cfg, PCM_CHANNELS,    info.channels);

	// go to the start of sound data
	length = riff_seek(fd, CHUNK_DATA);
	if (length == -1)
		return -1;

	return length;
}

static int
waveplay(char *device, char *file)
{
	int file_fd, sound_fd;
	struct pcm_hw_params cfg;
	void *buffer;

	int period_bytes;

	pcm_hw_params_init(&cfg);
	pcm_set(&cfg, PCM_PERIOD_SIZE, 4096);
	pcm_set(&cfg, PCM_ACCESS, PCM_ACCESS_RW);

	// Open file
	file_fd = open(file, O_RDONLY);
	if (file_fd == -1) {
		perror(file);
		return -1;
	}

	// Get wave file parameters and seek to data
	if (wave_setup(file_fd, &cfg) == -1) {
		perror("Invalid riff/wave file");
		return -1;
	}

	// Open PCM device
	sound_fd = open(device, O_RDWR);
	if (sound_fd == -1) {
		perror(device);
		return -1;
	}

	// Set PCM device parameters
	if (pcm_hw_params_setup(sound_fd, &cfg) == -1) {
		perror("Error while setting PCM hardware parameters");
		return -1;
	}

	period_bytes  = pcm_get(&cfg, PCM_PERIOD_BYTES, 0);
	buffer = malloc(period_bytes);

	// do playback
	while (keep_running && read(file_fd, buffer, period_bytes) > 0)
		write(sound_fd, buffer, period_bytes);

	// drain before exit
	pcm_drain(sound_fd);

	// On underrun the state becomes SETUP and write
	// error is EPIPE.

	free(buffer);
	close(sound_fd);
	close(file_fd);

	return 0;
}

static const char *usage =
"usage: cmd [pcm_device_file] <wav_file>\n"
"Default PCM device: /dev/snd/pcmC0D0p (PCM Card 0, Device 0, playback)\n"
"Since it's a playback program, only playback devices will work :-)\n";

int
main(int argc, char **argv)
{
	char *device = "/dev/snd/pcmC0D0p";
	char *file;

	signal(SIGINT, on_sigint);

	argv++;
	if (argc > 2)
		device  = *argv, argv++, argc--;
	if (argc < 2) {
		fputs(usage, stderr);
		return 1;
	}

	file = *argv;

	return waveplay(device, file) == -1;
}
