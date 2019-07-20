// NanoALSA: User space PCM/sound library for Linux
//
// Copyright (C) 2018  Ricardo Biehl Pasquali
//
// License: See LICENSE file at the root of this repository.

// 2018-09-03
//
// Flags for pcm_open().

#ifndef OPEN_H
#define OPEN_H

// first bit of flags
#define PCM_INPUT  0
#define PCM_OUTPUT 1

#define PCM_NONBLOCK (1 << 1)

int
pcm_open(int card, int device, int flags);

#endif // OPEN_H
