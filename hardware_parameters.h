// NanoALSA: User space PCM/sound library for Linux
//
// Copyright (C) 2018  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// Rewritten in 2018-06-08

#ifndef HARDWARE_PARAMETERS_H
#define HARDWARE_PARAMETERS_H

// ALSA header
#include <sound/asound.h>

// set parameters
// ==============

void
hw_params_set_mask(struct snd_pcm_hw_params *p, int parameter,
                   unsigned int value);

void
hw_params_set_interval(struct snd_pcm_hw_params *p, int parameter,
                       unsigned int min, unsigned int max);

void
hw_params_set(struct snd_pcm_hw_params *p, int parameter, unsigned int value);

// get parameters
// ==============

unsigned int
hw_params_get_mask(struct snd_pcm_hw_params *p, int parameter,
                   unsigned int value);

void
hw_params_get_interval(struct snd_pcm_hw_params *p, int parameter,
                       unsigned int *min, unsigned int *max);

unsigned int
hw_params_get(struct snd_pcm_hw_params *p, int parameter, unsigned int value);

// initialize parameters structure
// ===============================

void
hw_params_fill(struct snd_pcm_hw_params *p);

#endif // HARDWARE_PARAMETERS_H
