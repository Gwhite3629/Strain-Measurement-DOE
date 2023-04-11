//  File Header
//  Dynamic Strain Sensor
//  Grady White
//  1/3/22

#ifndef _FILE_H_
#define _FILE_H_

#include <windows.h>
#include <stdio.h>

#include "utils.h"

//  Generic function to find a string in a file and return its location.
int find_event(FILE *fd, // File to search
               fpos_t *prof_start, // Event location
               char *locator); // String to search for

//  Acquires data from config file and puts it into a struct
int get_config(Config *config, // Config struct
               char *name); // Config selection

// 
int init_datafile(FILE **datafile, time_t program_start_time);

//  Template for data write
int datafile_write(SUPER *input);

#endif //_FILE_H_