// NanoALSA: User space PCM/sound library for Linux
//
// Copyright (C) 2019  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// 2019-01-16
//
// Macros for hardware parameters.

#ifndef SETUP_H
#define SETUP_H

// ALSA header
#include <sound/asound.h>

// Hardware parameters
// ===================

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
pcm_set(struct pcm_hw_params *hw, int parameter, int value);

void
pcm_set_range(struct pcm_hw_params *hw, int parameter,
              unsigned int min, unsigned int max);

int
pcm_get(struct pcm_hw_params *hw, int parameter, int value);

void
pcm_get_range(struct pcm_hw_params *hw, int parameter,
              unsigned int *min, unsigned int *max);

int
pcm_hw_params_refine(int fd, struct pcm_hw_params *hw);

int
pcm_hw_params_setup(int fd, struct pcm_hw_params *hw);

// Software parameters
// ===================

// alias to the name of structure
#define pcm_sw_params snd_pcm_sw_params

// timestamp clock type
#define PCM_CLOCK_REALTIME      SNDRV_PCM_TSTAMP_TYPE_GETTIMEOFDAY
#define PCM_CLOCK_MONOTONIC     SNDRV_PCM_TSTAMP_TYPE_MONOTONIC
#define PCM_CLOCK_MONOTONIC_RAW SNDRV_PCM_TSTAMP_TYPE_MONOTONIC_RAW

void
pcm_sw_params_init(struct pcm_sw_params *sw, struct pcm_hw_params *hw);

int
pcm_software_setup(int fd, struct pcm_sw_params *sw);

#endif // SETUP_H
