/*
  * UAE - The Un*x Amiga Emulator
  *
  * Definitions for accessing cycle counters on a given machine, if possible.
  *
  * Copyright 1998 Bernd Schmidt
  */

#ifndef MACHDEP_RPT_H
#define MACHDEP_RPT_H

#include <sys/time.h>

typedef unsigned long frame_time_t;
static int64_t fs_get_monotonic_time(void);
extern int64_t g_uae_epoch;

static inline frame_time_t read_processor_time() {
    // with frame_time_t being int, the value will wrap around in
    // about 23 days..
    return (fs_get_monotonic_time() - g_uae_epoch);
}

static int64_t fs_get_monotonic_time(void) {
    struct timespec ts;
    clock_gettime (CLOCK_MONOTONIC, &ts);
    return (((int64_t) ts.tv_sec) * 1000000) + (ts.tv_nsec / 1000);
}

#endif // MACHDEP_RPT_H
