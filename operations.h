// NanoALSA: User space PCM/sound library for Linux
//
// Copyright (C) 2018, 2019  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// 2018-09-08
//
// Macros for device states, synchronization flags, ioctls
// (e.g. start/stop, rewind/forward, read/write).

#ifndef OPERATIONS_H
#define OPERATIONS_H

// ALSA header
#include <sound/asound.h>

#include <sys/ioctl.h> // ioctl()
#include <time.h>      // struct timespec

// Synchronization
// ===============

// device states
#define PCM_STATE_OPEN          SNDRV_PCM_STATE_OPEN
#define PCM_STATE_SETUP         SNDRV_PCM_STATE_SETUP
#define PCM_STATE_PREPARED      SNDRV_PCM_STATE_PREPARED
#define PCM_STATE_RUNNING       SNDRV_PCM_STATE_RUNNING
#define PCM_STATE_XRUN          SNDRV_PCM_STATE_XRUN
#define PCM_STATE_DRAINING      SNDRV_PCM_STATE_DRAINING
#define PCM_STATE_PAUSED        SNDRV_PCM_STATE_PAUSED
#define PCM_STATE_SUSPENDED     SNDRV_PCM_STATE_SUSPENDED
#define PCM_STATE_DISCONNECTED  SNDRV_PCM_STATE_DISCONNECTED

// synchronization flags
//
// For ALSA, SNDRV_PCM_SYNC_PTR_{APPL,AVAIL_MIN} mean to get
// instead of set. The flags are flipped in pcm_sync().
#define PCM_REQUEST_HW    SNDRV_PCM_SYNC_PTR_HWSYNC
#define PCM_SET_APPL      SNDRV_PCM_SYNC_PTR_APPL
#define PCM_SET_AVAIL_MIN SNDRV_PCM_SYNC_PTR_AVAIL_MIN

// Although the name of the status and control structures
// has "mmap", they may not be mmaped.
#define pcm_status  snd_pcm_mmap_status
#define pcm_control snd_pcm_mmap_control

struct pcm_sync {
	struct pcm_status  status;
	struct pcm_control control;
};

int
pcm_sync(int fd, struct pcm_sync *sync, unsigned int flags);

// PCM ioctls
// ==========
//
// The user may use the pcm_ctl/ioctl
//   pcm_ctl(fd, PCM_ACTION_START)
// or the fancy named
//   pcm_start(fd)

// pcm_ctl() is ioctl()
#define pcm_ctl ioctl

// Actions
//
// The ioctls that are internally so-called "actions" are:
//
// PREPARE, START, DROP (STOP), PAUSE, RESET, DRAIN, RESUME,
// XRUN.
//
//   PAUSE takes an integer argument. If nonzero, pause.
//   Otherwise, unpause.
//
//   RESUME has effect only if power management is enabled.
//   The SUSPEND action is done by the kernel.
//
//   XRUN is the same as STOP, except that additional
//   debugging may be done and state is set to XRUN.

// clearer names for ioctls
#define PCM_ACTION_PREPARE SNDRV_PCM_IOCTL_PREPARE
#define PCM_ACTION_START   SNDRV_PCM_IOCTL_START
#define PCM_ACTION_STOP    SNDRV_PCM_IOCTL_DROP
#define PCM_ACTION_DRAIN   SNDRV_PCM_IOCTL_DRAIN
#define PCM_ACTION_XRUN    SNDRV_PCM_IOCTL_XRUN
#define PCM_ACTION_RESET   SNDRV_PCM_IOCTL_RESET
#define PCM_ACTION_RESUME  SNDRV_PCM_IOCTL_RESUME
#define PCM_ACTION_PAUSE   SNDRV_PCM_IOCTL_PAUSE

// fancy way
#define pcm_prepare(fd) ioctl(fd, PCM_ACTION_PREPARE)
#define pcm_start(fd)   ioctl(fd, PCM_ACTION_START)
#define pcm_stop(fd)    ioctl(fd, PCM_ACTION_STOP)
#define pcm_drain(fd)   ioctl(fd, PCM_ACTION_DRAIN)
#define pcm_xrun(fd)    ioctl(fd, PCM_ACTION_XRUN)
#define pcm_reset(fd)   ioctl(fd, PCM_ACTION_RESET)
#define pcm_resume(fd)  ioctl(fd, PCM_ACTION_RESUME)
#define pcm_pause(fd)   ioctl(fd, PCM_ACTION_PAUSE, 1)
#define pcm_unpause(fd) ioctl(fd, PCM_ACTION_PAUSE, 0)

// timestamp of an action
int
pcm_action_timestamp(int fd, struct timespec *ts);

// Operations
//
// There are no set of ioctls called "operations". The
// ioctls below are here because they do an operation that
// is not an action, does not require any translation of
// arguments, and does not setup parameters or perform IO.
//
// HWSYNC, REWIND, FORWARD, LINK, UNLINK.
//
//   REWIND and FORWARD take frames as argument.
//
//   LINK takes the file descriptor of a PCM.

// clearer names for ioctls
#define PCM_DO_HARDWARE_POINTER_UPDATE SNDRV_PCM_IOCTL_HWSYNC

#define PCM_DO_REWIND  SNDRV_PCM_IOCTL_REWIND
#define PCM_DO_FORWARD SNDRV_PCM_IOCTL_FORWARD

#define PCM_DO_LINK   SNDRV_PCM_IOCTL_LINK
#define PCM_DO_UNLINK SNDRV_PCM_IOCTL_UNLINK

// fancy way
#define pcm_hw_update(fd) ioctl(fd, PCM_DO_HARDWARE_POINTER_UPDATE)

#define pcm_appl_add(fd, frames) \
	(frames < 0 ? ioctl(fd, PCM_DO_REWIND, -frames) \
	            : ioctl(fd, PCM_DO_FORWARD, frames) )

#define pcm_link(fd, fd2) ioctl(fd, PCM_DO_LINK, fd2)
#define pcm_unlink(fd)    ioctl(fd, PCM_DO_UNLINK)

// read/write helpers
//
// Like read/write system calls, but size is in frames.

static inline int
pcm_write(int fd, void *buf, int frames)
{
	struct snd_xferi tmp = {.buf = buf, .frames = frames, .result = 0};
	return ioctl(fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &tmp) ? -1 :
	       (int) tmp.result;
}

static inline int
pcm_read(int fd, void *buf, int frames)
{
	struct snd_xferi tmp = {.buf = buf, .frames = frames, .result = 0};
	return ioctl(fd, SNDRV_PCM_IOCTL_READI_FRAMES, &tmp) ? -1 :
	       (int) tmp.result;
}

// scattered read/write helpers
//
// Unlike readv/writev, the second argument is an array of
// n_channels pointers to buffers. The third argument is the
// number of frames to transfer.

static inline int
pcm_write_scattered(int fd, void *bufs, int frames)
{
	struct snd_xfern tmp = {.bufs = bufs, .frames = frames, .result = 0};
	return ioctl(fd, SNDRV_PCM_IOCTL_WRITEN_FRAMES, &tmp) ? -1 :
	       (int) tmp.result;
}

static inline int
pcm_read_scattered(int fd, void *bufs, int frames)
{
	struct snd_xfern tmp = {.bufs = bufs, .frames = frames, .result = 0};
	return ioctl(fd, SNDRV_PCM_IOCTL_READN_FRAMES, &tmp) ? -1 :
	       (int) tmp.result;
}

#endif // OPERATIONS_H
