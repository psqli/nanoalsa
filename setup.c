// NanoALSA: User space PCM/sound library for Linux
//
// Copyright (C) 2019  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// 2019-01-16
//
// Set hardware and software parameters.

// ALSA header
#include <sound/asound.h>

#include <sys/ioctl.h>

#include "setup.h"

#include "hardware_parameters.h"

// Hardware parameters
// ===================
//
// See hardware_parameters.c

// flags
#define PCM_NO_INTERRUPTS SNDRV_PCM_HW_PARAMS_NO_PERIOD_WAKEUP

void
pcm_hw_params_init(struct pcm_hw_params *hw)
{
	hw_params_fill(hw);
}

void
pcm_set(struct pcm_hw_params *hw, int parameter, int value)
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
int
pcm_get(struct pcm_hw_params *hw, int parameter, int value)
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
pcm_hardware_setup(int fd, struct pcm_hw_params *hw)
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
pcm_software_setup(int fd, struct pcm_sw_params *sw)
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
