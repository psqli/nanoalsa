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
