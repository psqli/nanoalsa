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

// According to C standard, section "6.7.2.2 Enumeration specifiers":
//     The identifiers in an enumerator list are declared as constants
//     that have type int [...]
// http://www.open-std.org/JTC1/SC22/WG14/www/docs/n1570.pdf
enum pcm_state {
	PCM_STATE_OPEN         = SNDRV_PCM_STATE_OPEN,
	PCM_STATE_SETUP        = SNDRV_PCM_STATE_SETUP,
	PCM_STATE_PREPARED     = SNDRV_PCM_STATE_PREPARED,
	PCM_STATE_RUNNING      = SNDRV_PCM_STATE_RUNNING,
	PCM_STATE_XRUN         = SNDRV_PCM_STATE_XRUN,
	PCM_STATE_DRAINING     = SNDRV_PCM_STATE_DRAINING,
	PCM_STATE_PAUSED       = SNDRV_PCM_STATE_PAUSED,
	PCM_STATE_SUSPENDED    = SNDRV_PCM_STATE_SUSPENDED,
	PCM_STATE_DISCONNECTED = SNDRV_PCM_STATE_DISCONNECTED,
};
typedef enum pcm_state pcm_state_t;

// synchronization flags
//
// For ALSA, SNDRV_PCM_SYNC_PTR_{APPL,AVAIL_MIN} mean to get
// instead of set. The flags are flipped in pcm_sync().
#define PCM_REQUEST_HW    SNDRV_PCM_SYNC_PTR_HWSYNC
#define PCM_SET_APPL      SNDRV_PCM_SYNC_PTR_APPL
#define PCM_SET_AVAIL_MIN SNDRV_PCM_SYNC_PTR_AVAIL_MIN

// Although the name of the status and control structures
// has "mmap", they may not be mmaped.
typedef struct snd_pcm_mmap_status  pcm_status_t;
typedef struct snd_pcm_mmap_control pcm_control_t;

struct pcm_sync {
	pcm_status_t  status;
	pcm_control_t control;
};
typedef struct pcm_sync pcm_sync_t;

int
pcm_sync(int fd, pcm_sync_t *sync, unsigned int flags);

int
pcm_action_timestamp(int fd, struct timespec *ts);

// Helpers for setting up hardware parameters on the PCM device
// ========================================================================

// alias to the name of structure
typedef struct snd_pcm_hw_params pcm_hw_params_t;

enum pcm_access {
	PCM_ACCESS_RW             = SNDRV_PCM_ACCESS_RW_INTERLEAVED,
	PCM_ACCESS_RW_SCATTERED   = SNDRV_PCM_ACCESS_RW_NONINTERLEAVED,
	PCM_ACCESS_MMAP           = SNDRV_PCM_ACCESS_MMAP_INTERLEAVED,
	PCM_ACCESS_MMAP_SCATTERED = SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED,
};
typedef enum pcm_access pcm_access_t;

enum pcm_format {
	PCM_FORMAT_S8     = SNDRV_PCM_FORMAT_S8,
	PCM_FORMAT_U8     = SNDRV_PCM_FORMAT_U8,
	PCM_FORMAT_S16_LE = SNDRV_PCM_FORMAT_S16_LE,
	PCM_FORMAT_S16_BE = SNDRV_PCM_FORMAT_S16_BE,
	PCM_FORMAT_U16_LE = SNDRV_PCM_FORMAT_U16_LE,
	PCM_FORMAT_U16_BE = SNDRV_PCM_FORMAT_U16_BE,
	PCM_FORMAT_S32_LE = SNDRV_PCM_FORMAT_S32_LE,
	PCM_FORMAT_S32_BE = SNDRV_PCM_FORMAT_S32_BE,
	PCM_FORMAT_U32_LE = SNDRV_PCM_FORMAT_U32_LE,
	PCM_FORMAT_U32_BE = SNDRV_PCM_FORMAT_U32_BE,
};
typedef enum pcm_format pcm_format_t;

enum pcm_hw_param {
	PCM_ACCESS       = SNDRV_PCM_HW_PARAM_ACCESS,       // mask (pcm_access_t)
	PCM_FORMAT       = SNDRV_PCM_HW_PARAM_FORMAT,       // mask (pcm_format_t)
	PCM_RATE         = SNDRV_PCM_HW_PARAM_RATE,         // interval
	PCM_CHANNELS     = SNDRV_PCM_HW_PARAM_CHANNELS,     // interval
	PCM_PERIOD_SIZE  = SNDRV_PCM_HW_PARAM_PERIOD_SIZE,  // interval
	PCM_BUFFER_SIZE  = SNDRV_PCM_HW_PARAM_BUFFER_SIZE,  // interval

	// Optional variants of other parameters
	PCM_SAMPLE_BITS  = SNDRV_PCM_HW_PARAM_SAMPLE_BITS,  // interval (variant of FORMAT)
	PCM_FRAME_BITS   = SNDRV_PCM_HW_PARAM_FRAME_BITS,   // interval (variant of CHANNELS)
	PCM_PERIOD_TIME  = SNDRV_PCM_HW_PARAM_PERIOD_TIME,  // interval (variant of PERIOD_SIZE)
	PCM_PERIOD_BYTES = SNDRV_PCM_HW_PARAM_PERIOD_BYTES, // interval (variant of PERIOD_SIZE)
	PCM_BUFFER_TIME  = SNDRV_PCM_HW_PARAM_BUFFER_TIME,  // interval (variant of BUFFER_SIZE)
	PCM_BUFFER_BYTES = SNDRV_PCM_HW_PARAM_BUFFER_BYTES, // interval (variant of BUFFER_SIZE)
	PCM_PERIODS      = SNDRV_PCM_HW_PARAM_PERIODS,      // interval (variant of BUFFER_SIZE)

	// Parameters handled by NanoALSA (see nanoalsa.c)
	PCM_INTERRUPT = SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 1,
};
typedef enum pcm_hw_param pcm_param_t;

void
pcm_hw_params_init(pcm_hw_params_t *hw);

void
pcm_set(pcm_hw_params_t *hw, pcm_param_t parameter, unsigned int value);

void
pcm_set_range(pcm_hw_params_t *hw, pcm_param_t parameter,
              unsigned int min, unsigned int max);

unsigned int
pcm_get(pcm_hw_params_t *hw, pcm_param_t parameter, unsigned int value);

void
pcm_get_range(pcm_hw_params_t *hw, pcm_param_t parameter,
              unsigned int *min, unsigned int *max);

static inline unsigned int
pcm_get_min(pcm_hw_params_t *hw, pcm_param_t parameter)
{
	unsigned int min, max;
	pcm_get_range(hw, parameter, &min, &max);
	return min;
}

static inline unsigned int
pcm_get_max(pcm_hw_params_t *hw, pcm_param_t parameter)
{
	unsigned int min, max;
	pcm_get_range(hw, parameter, &min, &max);
	return max;
}

int
pcm_hw_params_refine(int fd, pcm_hw_params_t *hw);

int
pcm_hw_params_setup(int fd, pcm_hw_params_t *hw);

// Helpers for setting up software parameters on the PCM device
// ========================================================================

// depends on pcm_hw_params_t (hardware parameters structure)

// alias to the name of structure
typedef struct snd_pcm_sw_params pcm_sw_params_t;

// timestamp clock type
enum pcm_clock_type_t {
	PCM_CLOCK_REALTIME      = SNDRV_PCM_TSTAMP_TYPE_GETTIMEOFDAY,
	PCM_CLOCK_MONOTONIC     = SNDRV_PCM_TSTAMP_TYPE_MONOTONIC,
	PCM_CLOCK_MONOTONIC_RAW = SNDRV_PCM_TSTAMP_TYPE_MONOTONIC_RAW,
};
typedef enum pcm_clock_type_t pcm_clock_type_t;

void
pcm_sw_params_init(pcm_sw_params_t *sw, pcm_hw_params_t *hw);

int
pcm_sw_params_setup(int fd, pcm_sw_params_t *sw);


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

enum pcm_ioctl {
	PCM_ACTION_PREPARE = SNDRV_PCM_IOCTL_PREPARE,
	PCM_ACTION_START   = SNDRV_PCM_IOCTL_START,
	PCM_ACTION_STOP    = SNDRV_PCM_IOCTL_DROP,
	PCM_ACTION_DRAIN   = SNDRV_PCM_IOCTL_DRAIN,
	PCM_ACTION_XRUN    = SNDRV_PCM_IOCTL_XRUN,
	PCM_ACTION_RESET   = SNDRV_PCM_IOCTL_RESET,
	PCM_ACTION_RESUME  = SNDRV_PCM_IOCTL_RESUME,
	PCM_ACTION_PAUSE   = SNDRV_PCM_IOCTL_PAUSE,

	PCM_MMAP_SYNC_POSITION = SNDRV_PCM_IOCTL_HWSYNC,

	PCM_DO_REWIND  = SNDRV_PCM_IOCTL_REWIND,
	PCM_DO_FORWARD = SNDRV_PCM_IOCTL_FORWARD,

	PCM_DO_LINK   = SNDRV_PCM_IOCTL_LINK,
	PCM_DO_UNLINK = SNDRV_PCM_IOCTL_UNLINK,
};
typedef enum pcm_ioctl pcm_ioctl_t;

static inline int pcm_prepare(int fd) { return ioctl(fd, PCM_ACTION_PREPARE); }
static inline int pcm_start(int fd)   { return ioctl(fd, PCM_ACTION_START); }
static inline int pcm_stop(int fd)    { return ioctl(fd, PCM_ACTION_STOP); }
static inline int pcm_drain(int fd)   { return ioctl(fd, PCM_ACTION_DRAIN); }
static inline int pcm_xrun(int fd)    { return ioctl(fd, PCM_ACTION_XRUN); }
static inline int pcm_reset(int fd)   { return ioctl(fd, PCM_ACTION_RESET); }
static inline int pcm_resume(int fd)  { return ioctl(fd, PCM_ACTION_RESUME); }
static inline int pcm_pause(int fd)   { return ioctl(fd, PCM_ACTION_PAUSE, 1); }
static inline int pcm_unpause(int fd) { return ioctl(fd, PCM_ACTION_PAUSE, 0); }

// Useful only when status (pcm_status_t) is mmapped as it does not return
// the updated position (for that, see pcm_sync() which returns it).
static inline int pcm_mmap_sync_pos(int fd) { return ioctl(fd, PCM_MMAP_SYNC_POSITION); }

// Increment or decrement application position
static inline int pcm_move_app_pos(int fd, int frames) {
	return frames < 0 ? ioctl(fd, PCM_DO_REWIND, -frames)
	                  : ioctl(fd, PCM_DO_FORWARD, frames);
}

static inline int pcm_link(int fd, int fd2) { return ioctl(fd, PCM_DO_LINK, fd2); }
static inline int pcm_unlink(int fd) { return ioctl(fd, PCM_DO_UNLINK); }

// IO helpers (i.e. read/write system calls, but size is in frames)
// ================================================================

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

// Unlike writev, the second argument is an array of n_channels pointers to
// buffers, and the third argument is the number of frames to transfer.
static inline int
pcm_write_scattered(int fd, void **bufs, int frames)
{
	struct snd_xfern tmp = {.bufs = bufs, .frames = frames, .result = 0};
	return ioctl(fd, SNDRV_PCM_IOCTL_WRITEN_FRAMES, &tmp) ? -1 :
	       (int) tmp.result;
}

// Unlike readv, the second argument is an array of n_channels pointers to
// buffers, and the third argument is the number of frames to transfer.
static inline int
pcm_read_scattered(int fd, void **bufs, int frames)
{
	struct snd_xfern tmp = {.bufs = bufs, .frames = frames, .result = 0};
	return ioctl(fd, SNDRV_PCM_IOCTL_READN_FRAMES, &tmp) ? -1 :
	       (int) tmp.result;
}

#endif // NANOALSA_H
