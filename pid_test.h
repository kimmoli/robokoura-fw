#ifndef _PID_TEST_H
#define _PID_TEST_H

#include "pid.h"

extern void start_pid(float p, float i, float d);
extern pidc_t pid;
extern bool autoaxis;

#endif
