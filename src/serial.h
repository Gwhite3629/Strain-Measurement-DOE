//  Serial Header
//  Dynamic Strain Sensor
//  Grady White
//  1/3/22

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdint.h>
#include <stddef.h>
#include <windows.h>

void print_error(char *context); // Error context

HANDLE open_port(const char *dev); // Name of device

int read_port(HANDLE port, // Serial device
            void *buf, // Read buffer
            size_t size); // Number of bytes to read

int write_port(HANDLE port, // Serial Device
               const char *buf, // Write buffer
               size_t size); // Number of bytes to write

int GPIB_conf(HANDLE port, // Serial Device
              int profile); // Profile selection

#endif //_SERIAL_H_