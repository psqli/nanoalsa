// NanoALSA: User space PCM/sound library for Linux
//
// Copyright (C) 2022  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// Header for ALSA's interface in Linux kernel
#include <sound/asound.h>

#include "nanoalsa.h"

// Hardware parameters manipulation
// ========================================================================

// set parameters
static void
hw_params_set_mask(struct snd_pcm_hw_params *p, int parameter,
                   unsigned int value);
static void
hw_params_set_interval(struct snd_pcm_hw_params *p, int parameter,
                       unsigned int min, unsigned int max);
static void
hw_params_set(struct snd_pcm_hw_params *p, int parameter, unsigned int value);

// get parameters
static unsigned int
hw_params_get_mask(struct snd_pcm_hw_params *p, int parameter,
                   unsigned int value);
static void
hw_params_get_interval(struct snd_pcm_hw_params *p, int parameter,
                       unsigned int *min, unsigned int *max);
static unsigned int
hw_params_get(struct snd_pcm_hw_params *p, int parameter, unsigned int value);

// initialize parameters structure
static void
hw_params_fill(struct snd_pcm_hw_params *p);

#include <limits.h>  // UINT_MAX
#include <stdint.h>  // int*_t
#include <string.h>  // memset()

// A mask parameter is an array of bits. Below, i is an
// index of a bit. As the computer can't manipulate a
// continuous array of bits, the actual array is of 32-bit
// entries.
//
// get_index(i) Returns the index for the actual array entry
//              that the bit belongs.
// get_mask(i)  Returns a mask with the bit set for the
//              actual array entry that it belongs.
#define get_index(i)       ((i) / 32)
#define get_mask(i)  (1 << ((i) % 32))

#define is_mask(parameter) \
	(parameter >= SNDRV_PCM_HW_PARAM_FIRST_MASK && \
	 parameter <= SNDRV_PCM_HW_PARAM_LAST_MASK)

#define is_interval(parameter) \
	(parameter >= SNDRV_PCM_HW_PARAM_FIRST_INTERVAL && \
	 parameter <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL)

static inline struct snd_mask*
get_mask_struct(struct snd_pcm_hw_params *p, unsigned int parameter)
{
	return &p->masks[parameter - SNDRV_PCM_HW_PARAM_FIRST_MASK];
}

static inline struct snd_interval*
get_interval_struct(struct snd_pcm_hw_params *p, unsigned int parameter)
{
	return &p->intervals[parameter - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
}

static void
hw_params_set_mask(struct snd_pcm_hw_params *p, int parameter,
                   unsigned int value)
{
	struct snd_mask *m = get_mask_struct(p, parameter);

	// if value is set, mask is assumed as filled by
	// hw_params_fill(), then it is cleared.
	if (m->bits[get_index(value)] & get_mask(value))
		memset(m, 0x00, sizeof(*m));

	m->bits[get_index(value)] |= get_mask(value);
}

static void
hw_params_set_interval(struct snd_pcm_hw_params *p, int parameter,
                       unsigned int min, unsigned int max)
{
	struct snd_interval *i = get_interval_struct(p, parameter);

	// close interval
	i->openmin = 0;
	i->openmax = 0;
	i->integer = 1;

	// TODO: can a refined interval be empty?

	i->min = min;
	i->max = max;
}

static void
hw_params_set(struct snd_pcm_hw_params *p, int parameter, unsigned int value)
{
	if (is_mask(parameter))
		hw_params_set_mask(p, parameter, value);
	else if (is_interval(parameter))
		hw_params_set_interval(p, parameter, value, value);
}

// Return 1 if value is set, otherwise 0
//
// E.g.: parameter = FORMAT, value = Signed 16-bit Little-Endian
static unsigned int
hw_params_get_mask(struct snd_pcm_hw_params *p, int parameter,
                   unsigned int value)
{
	struct snd_mask *m = get_mask_struct(p, parameter);

	return m->bits[get_index(value)] & get_mask(value);
}

// After HW_PARAMS ioctl, min == max
static void
hw_params_get_interval(struct snd_pcm_hw_params *p, int parameter,
                       unsigned int *min, unsigned int *max)
{
	struct snd_interval *i = get_interval_struct(p, parameter);

	// return closed interval
	*min = i->min + i->openmin;
	*max = i->max - i->openmax;
}

static unsigned int
hw_params_get(struct snd_pcm_hw_params *p, int parameter, unsigned int value)
{
	unsigned int ret, tmp;

	if (is_mask(parameter))
		ret = hw_params_get_mask(p, parameter, value);
	else if (is_interval(parameter))
		hw_params_get_interval(p, parameter, &ret, &tmp);
	else
		return 0;

	return ret;
}

static const int INTERVAL_COUNT = SNDRV_PCM_HW_PARAM_LAST_INTERVAL -
                                  SNDRV_PCM_HW_PARAM_FIRST_INTERVAL;

// Set all parameters to all values (i.e. fill all masks
// with ones and fill all intervals with full range).
//
// In HW_REFINE and HW_PARAMS ioctls, ALSA removes the
// values that are not allowed by hardware or that are not
// coherent to the values of other parameters that set (or
// are influenced by) the same setting. If no values remain
// after refinement, EINVAL error is returned.
static void
hw_params_fill(struct snd_pcm_hw_params *p)
{
	int i;

	// clear struct
	memset(p, 0, sizeof(*p));

	// fill all masks with ones
	memset(p->masks, 0xff, sizeof(p->masks));

	// Fill all intervals with the lowest value in min
	// and the highest value in max.
	for (i = 0; i <= INTERVAL_COUNT; i++) {
		p->intervals[i].min = 0;
		p->intervals[i].max = UINT_MAX;
	}

	// Refine mask: ALSA only refines the parameters
	// (masks and intervals) that are in this mask. To
	// refine all parameters, here it's completely
	// filled in. This is ignored in HW_PARAMS ioctl.
	p->rmask = UINT_MAX;

	// Changed mask: ALSA returns the parameters it has
	// changed in this mask.
	p->cmask = 0;

	// Additional information flags returned by ALSA.
	// See SNDRV_PCM_INFO_* flags in sound/asound.h
	p->info = UINT_MAX;

	// deprecated
	p->msbits = 0;   // sample bits
	p->rate_num = 0; // rate
	p->rate_den = 0; // always 1
}

// System call wrappers for time and position synchronization
// ========================================================================

#include <sys/ioctl.h> // ioctl()
#include <time.h>      // struct timespec

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

// Helpers for setting up the PCM device
// ========================================================================

#include <sys/ioctl.h>

// hw_params flags
#define PCM_NO_INTERRUPTS SNDRV_PCM_HW_PARAMS_NO_PERIOD_WAKEUP

void
pcm_params_init(pcm_params_t *p)
{
	hw_params_fill(&p->hw_params);

	pcm_sw_params_t *sw = &p->sw_params;
	memset(sw, 0, sizeof(*sw));
	// TODO set to kernel defaults
	// In HW_PARAMS_SETUP ioctl:
	//   - boundary is set by the kernel
	// After HW_PARAMS_SETUP, but before calling SW_PARAMS_SETUP:
	//   - if zero, stop_threshold defaults to the returned buffer_size
	//   - if zero, avail_min defaults to the returned period_size
	// Here, we set:
	sw->start_threshold = 1;
	sw->period_step = 1; // is deprecated?
}

void
pcm_set(pcm_params_t *params, pcm_param_t parameter, unsigned long value)
{
	pcm_hw_params_t *hw = &params->hw_params;
	pcm_sw_params_t *sw = &params->sw_params;
	switch (parameter) {
	default:
		// unsigned long value must be truncated to unsigned int (this is expected)
		hw_params_set(hw, parameter, value);
		break;
	case PCM_INTERRUPT:
		hw->flags = value ? hw->flags |  PCM_NO_INTERRUPTS
		                  : hw->flags & ~PCM_NO_INTERRUPTS;
		break;

	// Software parameters
	case PCM_TSTAMP_TYPE:
		// enable timestamp automatically when this setting is set
		// Otherwise, timestamp is disabled.
		sw->tstamp_mode = SNDRV_PCM_TSTAMP_ENABLE;
		sw->tstamp_type = value;
		break;
	case PCM_AVAIL_MIN:         sw->avail_min         = value; break;
	case PCM_START_THRESHOLD:   sw->start_threshold   = value; break;
	case PCM_XRUN_THRESHOLD:    sw->stop_threshold    = value; break;
	case PCM_SILENCE_THRESHOLD: sw->silence_threshold = value; break;
	case PCM_SILENCE_SIZE:      sw->silence_size      = value; break;
	}
}

void
pcm_set_range(pcm_params_t *params, pcm_param_t parameter,
              unsigned int min, unsigned int max)
{
	if (parameter <= PCM_LAST_MASK || parameter > PCM_LAST_INTERVAL)
		pcm_set(params, parameter, min);
	else
		hw_params_set_interval(&params->hw_params, parameter, min, max);
}

// for masks, return 1 if value is set, 0 otherwise
unsigned long
pcm_get(pcm_params_t *params, pcm_param_t parameter, unsigned int value)
{
	pcm_hw_params_t *hw = &params->hw_params;
	pcm_sw_params_t *sw = &params->sw_params;
	switch (parameter) {
	default:                    return hw_params_get(hw, parameter, value);
	case PCM_INTERRUPT:         return hw->flags & PCM_NO_INTERRUPTS;
	case PCM_TSTAMP_TYPE:       return sw->tstamp_mode ? sw->tstamp_type : UINT_MAX;
	case PCM_AVAIL_MIN:         return sw->avail_min;
	case PCM_START_THRESHOLD:   return sw->start_threshold;
	case PCM_XRUN_THRESHOLD:    return sw->stop_threshold;
	case PCM_SILENCE_THRESHOLD: return sw->silence_threshold;
	case PCM_SILENCE_SIZE:      return sw->silence_size;
	}
}

void
pcm_get_range(pcm_params_t *params, pcm_param_t parameter,
              unsigned int *min, unsigned int *max)
{
	hw_params_get_interval(&params->hw_params, parameter, min, max);
}

int
pcm_params_refine(int fd, pcm_params_t *params)
{
	return ioctl(fd, SNDRV_PCM_IOCTL_HW_REFINE, &params->hw_params);
}

int
pcm_params_setup(int fd, pcm_params_t *params)
{
	// send hardware parameters to ALSA in kernel
	if (ioctl(fd, SNDRV_PCM_IOCTL_HW_PARAMS, &params->hw_params) == -1)
		return -1;

	if (!pcm_get(params, PCM_AVAIL_MIN, 0))
		pcm_set(params, PCM_AVAIL_MIN, pcm_get(params, PCM_PERIOD_SIZE, 0));
	if (!pcm_get(params, PCM_XRUN_THRESHOLD, 0))
		pcm_set(params, PCM_XRUN_THRESHOLD, pcm_get(params, PCM_BUFFER_SIZE, 0));

	// must use TTSTAMP ioctl before 2.0.12 protocol
#ifdef SNDRV_PCM_IOCTL_TTSTAMP
	if (ioctl(fd, SNDRV_PCM_IOCTL_TTSTAMP, &params->sw_params.tstamp_type) == -1)
		return -1;
#endif

	// send software parameters to ALSA in kernel
	if (ioctl(fd, SNDRV_PCM_IOCTL_SW_PARAMS, &params->sw_params) == -1)
		return -1;

	return ioctl(fd, SNDRV_PCM_IOCTL_PREPARE);
}

// Helper for opening Linux PCM device
// ========================================================================

#include <linux/limits.h> // PATH_MAX
#include <stdio.h>        // snprintf()
#include <sys/types.h>    // open()
#include <sys/stat.h>     // open()
#include <fcntl.h>        // open()

#ifndef PCM_DEV_PATH
#define PCM_DEV_PATH "/dev/snd/"
#endif

int
pcm_open(int card, int device, int flags)
{
	char path[PATH_MAX];

	snprintf(path, sizeof(path), PCM_DEV_PATH "pcmC%uD%u%c", card, device,
	         (flags & 1) == PCM_INPUT ? 'c' : 'p');

	return open(path, O_RDWR | (flags & PCM_NONBLOCK ? O_NONBLOCK : 0));
}
