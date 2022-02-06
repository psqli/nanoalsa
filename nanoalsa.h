// NanoALSA: User space PCM/sound library for Linux
//
// Copyright (C) 2022  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

#ifndef NANOALSA_H
#define NANOALSA_H

// Header for ALSA's interface in Linux kernel
#include <sound/asound.h>

// System call wrappers for time and position synchronization
// ========================================================================

// Macros for device states, synchronization flags, ioctls
// (e.g. start/stop, rewind/forward, read/write).

#include <sys/ioctl.h> // ioctl()
#include <time.h>      // struct timespec

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

int
pcm_action_timestamp(int fd, struct timespec *ts);

// Helpers for setting up the PCM device
// ========================================================================

// alias to the name of structure
#define pcm_hw_params snd_pcm_hw_params

// Masks
// -----

// ACCESS
#define PCM_ACCESS                SNDRV_PCM_HW_PARAM_ACCESS
#define PCM_ACCESS_RW             SNDRV_PCM_ACCESS_RW_INTERLEAVED
#define PCM_ACCESS_RW_SCATTERED   SNDRV_PCM_ACCESS_RW_NONINTERLEAVED
#define PCM_ACCESS_MMAP           SNDRV_PCM_ACCESS_MMAP_INTERLEAVED
#define PCM_ACCESS_MMAP_SCATTERED SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED

// FORMAT
#define PCM_FORMAT         SNDRV_PCM_HW_PARAM_FORMAT
#define PCM_FORMAT_S8      SNDRV_PCM_FORMAT_S8
#define PCM_FORMAT_U8      SNDRV_PCM_FORMAT_U8
#define PCM_FORMAT_S16_LE  SNDRV_PCM_FORMAT_S16_LE
#define PCM_FORMAT_S16_BE  SNDRV_PCM_FORMAT_S16_BE
#define PCM_FORMAT_U16_LE  SNDRV_PCM_FORMAT_U16_LE
#define PCM_FORMAT_U16_BE  SNDRV_PCM_FORMAT_U16_BE
#define PCM_FORMAT_S32_LE  SNDRV_PCM_FORMAT_S32_LE
#define PCM_FORMAT_S32_BE  SNDRV_PCM_FORMAT_S32_BE
#define PCM_FORMAT_U32_LE  SNDRV_PCM_FORMAT_U32_LE
#define PCM_FORMAT_U32_BE  SNDRV_PCM_FORMAT_U32_BE

// Intervals
// ---------

// Originals
#define PCM_RATE         SNDRV_PCM_HW_PARAM_RATE
#define PCM_CHANNELS     SNDRV_PCM_HW_PARAM_CHANNELS
#define PCM_PERIOD_SIZE  SNDRV_PCM_HW_PARAM_PERIOD_SIZE
#define PCM_BUFFER_SIZE  SNDRV_PCM_HW_PARAM_BUFFER_SIZE

// Variants
#define PCM_SAMPLE_BITS  SNDRV_PCM_HW_PARAM_SAMPLE_BITS
#define	PCM_FRAME_BITS   SNDRV_PCM_HW_PARAM_FRAME_BITS
#define PCM_PERIOD_TIME  SNDRV_PCM_HW_PARAM_PERIOD_TIME
#define PCM_PERIOD_BYTES SNDRV_PCM_HW_PARAM_PERIOD_BYTES
#define	PCM_BUFFER_TIME  SNDRV_PCM_HW_PARAM_BUFFER_TIME
#define	PCM_BUFFER_BYTES SNDRV_PCM_HW_PARAM_BUFFER_BYTES
#define PCM_PERIODS      SNDRV_PCM_HW_PARAM_PERIODS

// Others
// ------

// this is a flag in hardware parameters structure (see setup.c)
#define PCM_INTERRUPT SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 1

// Functions
// ---------

void
pcm_hw_params_init(struct pcm_hw_params *hw);

void
pcm_set(struct pcm_hw_params *hw, int parameter, unsigned int value);

void
pcm_set_range(struct pcm_hw_params *hw, int parameter,
              unsigned int min, unsigned int max);

unsigned int
pcm_get(struct pcm_hw_params *hw, int parameter, unsigned int value);

void
pcm_get_range(struct pcm_hw_params *hw, int parameter,
              unsigned int *min, unsigned int *max);

static inline unsigned int
pcm_get_min(struct pcm_hw_params *hw, int parameter)
{
	unsigned int min, max;
	pcm_get_range(hw, parameter, &min, &max);
	return min;
}

static inline unsigned int
pcm_get_max(struct pcm_hw_params *hw, int parameter)
{
	unsigned int min, max;
	pcm_get_range(hw, parameter, &min, &max);
	return max;
}

int
pcm_hw_params_refine(int fd, struct pcm_hw_params *hw);

int
pcm_hw_params_setup(int fd, struct pcm_hw_params *hw);

// Software parameters
// -------------------

// alias to the name of structure
#define pcm_sw_params snd_pcm_sw_params

// timestamp clock type
#define PCM_CLOCK_REALTIME      SNDRV_PCM_TSTAMP_TYPE_GETTIMEOFDAY
#define PCM_CLOCK_MONOTONIC     SNDRV_PCM_TSTAMP_TYPE_MONOTONIC
#define PCM_CLOCK_MONOTONIC_RAW SNDRV_PCM_TSTAMP_TYPE_MONOTONIC_RAW

void
pcm_sw_params_init(struct pcm_sw_params *sw, struct pcm_hw_params *hw);

int
pcm_sw_params_setup(int fd, struct pcm_sw_params *sw);


// Helper for opening Linux PCM device
// ========================================================================

// first bit of flags
#define PCM_INPUT  0
#define PCM_OUTPUT 1

#define PCM_NONBLOCK (1 << 1)

int
pcm_open(int card, int device, int flags);

// PCM ioctls, actions, and IO helpers
// ========================================================================

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

#endif // NANOALSA_H
