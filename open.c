// NanoALSA: User space PCM/sound library for Linux
//
// Copyright (C) 2018  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// 2018-09-01
//
// Open PCM device.

// ALSA header
#include <sound/asound.h>

#include <linux/limits.h> // PATH_MAX
#include <stdio.h>        // snprintf()
#include <sys/types.h>    // open()
#include <sys/stat.h>     // open()
#include <fcntl.h>        // open()

#include "open.h"

#define PCM_DEV_PATH "/dev/snd/"

int
pcm_open(int card, int device, int flags)
{
	char path[PATH_MAX];

	snprintf(path, sizeof(path),
	         PCM_DEV_PATH "pcmC%uD%u%c", card, device,
	         (flags & 1) == PCM_INPUT ? 'c' : 'p');

	return open(path, O_RDWR | (flags & PCM_NONBLOCK ? O_NONBLOCK : 0));
}
