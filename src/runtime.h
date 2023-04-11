//  Runtime Header
//  Dynamic Strain Sensor
//  Grady White
//  1/3/22

#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include <windows.h>
#include <time.h>

int runtime(HANDLE dev, time_t program_start_time, FILE *datafile); // Serial Device

#endif //_RUNTIME_H_