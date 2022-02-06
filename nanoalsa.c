// NanoALSA: User space PCM/sound library for Linux
//
// Copyright (C) 2022  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// Header for ALSA's interface in Linux kernel
#include <sound/asound.h>

#include "nanoalsa.h"

// NOTE: It's possible to copy the source code of any section in this file
// and use as a separate file. The only extra step is to add ALSA's header
// included above.

// Hardware parameters manipulation
// ========================================================================

// set parameters
void
hw_params_set_mask(struct snd_pcm_hw_params *p, int parameter,
                   unsigned int value);
void
hw_params_set_interval(struct snd_pcm_hw_params *p, int parameter,
                       unsigned int min, unsigned int max);
void
hw_params_set(struct snd_pcm_hw_params *p, int parameter, unsigned int value);

// get parameters
unsigned int
hw_params_get_mask(struct snd_pcm_hw_params *p, int parameter,
                   unsigned int value);
void
hw_params_get_interval(struct snd_pcm_hw_params *p, int parameter,
                       unsigned int *min, unsigned int *max);
unsigned int
hw_params_get(struct snd_pcm_hw_params *p, int parameter, unsigned int value);

// initialize parameters structure
// -------------------------------
void
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

//       Set
// ================

void
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

void
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

void
hw_params_set(struct snd_pcm_hw_params *p, int parameter, unsigned int value)
{
	if (is_mask(parameter))
		hw_params_set_mask(p, parameter, value);
	else if (is_interval(parameter))
		hw_params_set_interval(p, parameter, value, value);
}

//       Get
// ================

// Return 1 if value is set, otherwise 0
//
// E.g.: parameter = FORMAT, value = Signed 16-bit Little-Endian
unsigned int
hw_params_get_mask(struct snd_pcm_hw_params *p, int parameter,
                   unsigned int value)
{
	struct snd_mask *m = get_mask_struct(p, parameter);

	return m->bits[get_index(value)] & get_mask(value);
}

// After HW_PARAMS ioctl, min == max
void
hw_params_get_interval(struct snd_pcm_hw_params *p, int parameter,
                       unsigned int *min, unsigned int *max)
{
	struct snd_interval *i = get_interval_struct(p, parameter);

	// return closed interval
	*min = i->min + i->openmin;
	*max = i->max - i->openmax;
}

unsigned int
hw_params_get(struct snd_pcm_hw_params *p, int parameter, unsigned int value)
{
	unsigned int ret, tmp;

	if (is_mask(parameter))
		ret = hw_params_get_mask(p, parameter, value);
	else if (is_interval(parameter))
		hw_params_get_interval(p, parameter, &ret, &tmp);

	return ret;
}

//       Fill
// ================

// define the relative LAST_MASK and LAST_INTERVAL
#define LAST_MASK \
	(SNDRV_PCM_HW_PARAM_LAST_MASK - SNDRV_PCM_HW_PARAM_FIRST_MASK)
#define LAST_INTERVAL \
	(SNDRV_PCM_HW_PARAM_LAST_INTERVAL - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL)

// Set all parameters to all values (i.e. fill all masks
// with ones and fill all intervals with full range).
//
// In HW_REFINE and HW_PARAMS ioctls, ALSA removes the
// values that are not allowed by hardware or that are not
// coherent to the values of other parameters that set (or
// are influenced by) the same setting. If no values remain
// after refinement, EINVAL error is returned.
void
hw_params_fill(struct snd_pcm_hw_params *p)
{
	int i;

	// clear struct
	memset(p, 0, sizeof(*p));

	// fill all masks with ones
	memset(p->masks, 0xff, sizeof(p->masks));

	// Fill all intervals with the lowest value in min
	// and the highest value in max.
	for (i = 0; i <= LAST_INTERVAL; i++) {
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

// flags
#define PCM_NO_INTERRUPTS SNDRV_PCM_HW_PARAMS_NO_PERIOD_WAKEUP

void
pcm_hw_params_init(struct pcm_hw_params *hw)
{
	hw_params_fill(hw);
}

void
pcm_set(struct pcm_hw_params *hw, int parameter, unsigned int value)
{
	switch (parameter) {
	case PCM_INTERRUPT:
		hw->flags = value ? hw->flags |  PCM_NO_INTERRUPTS
		                  : hw->flags & ~PCM_NO_INTERRUPTS;
		return;
	}

	hw_params_set(hw, parameter, value);
}

void
pcm_set_range(struct pcm_hw_params *hw, int parameter,
              unsigned int min, unsigned int max)
{
	hw_params_set_interval(hw, parameter, min, max);
}

// for masks, return 1 if value is set, 0 otherwise
unsigned int
pcm_get(struct pcm_hw_params *hw, int parameter, unsigned int value)
{
	switch (parameter) {
	case PCM_INTERRUPT:
		return hw->flags & PCM_NO_INTERRUPTS;
	}

	return hw_params_get(hw, parameter, value);
}

void
pcm_get_range(struct pcm_hw_params *hw, int parameter,
              unsigned int *min, unsigned int *max)
{
	hw_params_get_interval(hw, parameter, min, max);
}

int
pcm_hw_params_refine(int fd, struct pcm_hw_params *hw)
{
	return ioctl(fd, SNDRV_PCM_IOCTL_HW_REFINE, hw);
}

int
pcm_hw_params_setup(int fd, struct pcm_hw_params *hw)
{
	// send hardware parameters to ALSA in kernel
	if (ioctl(fd, SNDRV_PCM_IOCTL_HW_PARAMS, hw) == -1)
		return -1;

	return ioctl(fd, SNDRV_PCM_IOCTL_PREPARE);
}

// Software parameters
// ===================

void
pcm_sw_params_init(struct pcm_sw_params *sw, struct pcm_hw_params *hw)
{
	// enable timestamp by default
	sw->tstamp_mode = SNDRV_PCM_TSTAMP_ENABLE;
	sw->tstamp_type = PCM_CLOCK_REALTIME;

	sw->avail_min = pcm_get(hw, PCM_PERIOD_SIZE, 0);

	sw->start_threshold = 1;
	sw->stop_threshold = pcm_get(hw, PCM_BUFFER_SIZE, 0);
	sw->silence_threshold = 0;
	sw->silence_size = 0;

	sw->period_step = 1; /* deprecated? */
}

int
pcm_sw_params_setup(int fd, struct pcm_sw_params *sw)
{
	// must use TTSTAMP ioctl before 2.0.12 protocol
#ifdef SNDRV_PCM_IOCTL_TTSTAMP
	if (ioctl(fd, SNDRV_PCM_IOCTL_TTSTAMP, &sw->tstamp_type) == -1)
		return -1;
#endif

	// send software parameters to ALSA in kernel
	if (ioctl(fd, SNDRV_PCM_IOCTL_SW_PARAMS, sw) == -1)
		return -1;

	return 0;
}

// Helper for opening Linux PCM device
// ========================================================================

#include <linux/limits.h> // PATH_MAX
#include <stdio.h>        // snprintf()
#include <sys/types.h>    // open()
#include <sys/stat.h>     // open()
#include <fcntl.h>        // open()

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
