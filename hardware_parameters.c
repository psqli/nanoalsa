// NanoALSA: User space PCM/sound library for Linux
//
// Copyright (C) 2018  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// Rewritten in 2018-06-08
//
// Manipulate hardware parameters structure.

// ALSA header
#include <sound/asound.h>

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

	*min = i->min;
	*max = i->max;
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
	//
	// 'integer' is set to keep the interval closed
	// (i.e. openmin = openmax = 0) after refinement.
	// See snd_interval_refine() in pcm_lib.c
	for (i = 0; i <= LAST_INTERVAL; i++) {
		p->intervals[i].min = 0;
		p->intervals[i].max = UINT_MAX;

		p->intervals[i].integer = 1;
	}

	// Request mask: ALSA only refines the parameters
	// (masks and intervals) that are in this mask.
	// As we want to refine all parameters, here it's
	// completely filled in.
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
