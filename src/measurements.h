//  Measurements Header
//  Dynamic Strain Sensor
//  Grady White
//  1/3/22

#ifndef _MEASUREMENTS_H_
#define _MEASUREMENTS_H_

#include <stdint.h>
#include <windows.h>

#include "utils.h"

int full_measure(SUPER *input);

int get_curve(HANDLE fd, float **output);

int get_fft(SUPER *input);

#endif //_MEASUREMENTS_H_