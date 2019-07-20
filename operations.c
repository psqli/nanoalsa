// NanoALSA: User space PCM/sound library for Linux
//
// Copyright (C) 2018, 2019  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// 2018-09-03
//
// Get/set runtime information, get action timestamp.

// ALSA header
#include <sound/asound.h>

#include <sys/ioctl.h> // ioctl()
#include <time.h>      // struct timespec

#include "operations.h" // struct pcm_sync

// If requested, update hardware pointer. Get or set control
// structure. Get status structure.
int
pcm_sync(int fd, struct pcm_sync *sync, unsigned int flags)
{
	struct snd_pcm_sync_ptr tmp;

	// flip flags for their real meaning (get instead of set)
	flags ^= PCM_SET_APPL | PCM_SET_AVAIL_MIN;

	tmp.flags = flags;
	if (ioctl(fd, SNDRV_PCM_IOCTL_SYNC_PTR, &tmp) == -1)
		return -1;

	sync->control = tmp.c.control;
	sync->status  = tmp.s.status;

	return 0;
}

// Get timestamp of the last action (e.g. start/stop)
int
pcm_action_timestamp(int fd, struct timespec *ts)
{
	struct snd_pcm_status status;

	if (ioctl(fd, SNDRV_PCM_IOCTL_STATUS, &status) == -1)
		return -1;

	*ts = status.trigger_tstamp;

	return 0;
}
