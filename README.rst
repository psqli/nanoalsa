========
NanoALSA
========

:Date: 2019-01-10

This library is under development. Do not expect any
stability. The API (Application Programming Interface) may
change at any time.

Simple, small, user space, file descriptor based library for
Pulse Code Modulation (PCM) interface of the Linux kernel
sound subsystem (Advanced Linux Sound Architecture, or
simply ALSA).

I.e. A simple library to record and play sound on Linux.

I.e. A smaller TinyALSA.

For a more complete library, yet simple, see tinyalsa. For
the standard ALSA library, see alsa-lib.

TinyALSA:
https://github.com/tinyalsa/tinyalsa

alsa-lib:
http://git.alsa-project.org/?p=alsa-lib.git


What is PCM?
============

PCM stands for Pulse Code Modulation, which is a digital
representation of a wave where amplitude is converted to
a number at regular intervals::

	 3|                                o
	 2|  sample                   o         o
	 1|   /
	 0|  o                   o                   o
	-1|
	-2|       o         o
	-3|            o
	     |    |    |    |    |    |    |    |    |

How to use it
=============

Compile the library with `make`.

TODO: How to link.

Using the API
-------------

For starting IO, do the following:

1. Call pcm_open() or directly open() the /dev/pcm* file.
2. Initialize hardware parameters structure with pcm_hw_params_init().
3. Set hardware parameters with pcm_set(). E.g.: ACCESS = RW, FORMAT =
   S16_LE, RATE = 44100, and CHANNELS = 2.
4. Send hardware parameters structure to Linux kernel with
   pcm_hw_params_setup().
5. Start IO with pcm_write()/pcm_read(), or write()/read().

API reference
-------------

pcm_open(card, device, flags)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Open PCM device file (e.g. /dev/snd/pcmC0D0p for card 0, device 0, and
playback).

Card and device are integers.

- flags (int):

  - For output/playback use PCM_OUTPUT. For input/capture use PCM_INPUT.
  - (optional) PCM_NONBLOCK: do not wait if IO is not possible.

Returns file descriptor on success, -1 on failure.

--------------------------------

pcm_hw_params_init(params)
~~~~~~~~~~~~~~~~~~~~~~~~~~

Initialize hardware parameters structure (params).

- params (struct pcm_hw_params): Has the parameters needed for
  configuring the hardware for starting IO. E.g. frames per second (rate),
  samples per frame, sample resolution (8-bit, 16-bit), and more. Use
  pcm_set() and pcm_set_range() for manipulating this structure.

Returns nothing.

--------------------------------

pcm_set(params, parameter, value)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Set `parameter` to `value` on hardware parameters (`params`) structure.

Main parameters are:

- PCM_ACCESS which can be:

  - PCM_ACCESS_RW
  - PCM_ACCESS_RW_SCATTERED
  - PCM_ACCESS_MMAP
  - PCM_ACCESS_MMAP_SCATTERED
- PCM_FORMAT which can be: PCM_FORMAT_(S|U)(8|16|32)(_LE|_BE), e.g.:

  - PCM_FORMAT_S8 (Signed 8-bit)
  - PCM_FORMAT_S16_LE (Signed 16-bit Little Endian)
  - PCM_FORMAT_U32_BE (Unsigned 32-bit Big Endian)
- PCM_RATE e.g.: 22050Hz, 44100Hz, 48000Hz
- PCM_CHANNELS e.g.: 1 (mono), 2 (stereo)
- PCM_PERIOD_SIZE
- PCM_BUFFER_SIZE

For PCM_FORMAT parameter, calling pcm_set() more than once with a DIFFERENT
value will leave the parameter with multiple values/bits set. For erasing
all the other bits just call pcm_set() twice with the SAME value.

Returns nothing.

--------------------------------

pcm_set_range(params, parameter, min, max)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Set a range in which the Linux kernel will choose the first valid value
(supported by hardware) for `parameter`.

- min (int): Minimum value.
- max (int): Maximum value.

Returns nothing.

--------------------------------

pcm_hw_params_setup(fd, params)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Send hardware parameters to device.

`fd` is the device's file descriptor.

Returns 0 on success, -1 on failure.
