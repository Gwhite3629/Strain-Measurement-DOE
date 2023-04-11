//  Setup
//  Dynamic Strain Sensor
//  Grady White
//  1/5/22

#include <windows.h>
#include <stdio.h>

#include "utils.h"
#include "serial.h"

#define DIV "CH1:SCALE 2.0E-1\r"

int init(HANDLE fd, Config *config)
{
    int ret = 0;

    char vscale[64];
    char hscale[64];

    CHECK((ret = sprintf(vscale, "CH1:SCALE %s\r", config->VSCALE)));
    CHECK((ret = sprintf(hscale, "HORIZONTAL:SCALE %s\r", config->HSCALE)));

    CHECK((ret = write_port(fd, "CURSOR:SELect:SOUrce CH1\r", 25)));
    CHECK((ret = write_port(fd, "CH1:COUPLING AC\r", 16)));
    CHECK((ret = write_port(fd, vscale, strlen(vscale))));
    CHECK((ret = write_port(fd, hscale, strlen(hscale))));
    CHECK((ret = write_port(fd, "MATH:DEFINE \"FFT (CH1, HANNING)\"\r", 33)));
    CHECK((ret = write_port(fd, "DATA:SOURCE CH1\r", 16)));
    CHECK((ret = write_port(fd, "DATA:START 1\r", 13)));
    CHECK((ret = write_port(fd, "DATA:STOP 2500\r", 15)));  
    CHECK((ret = write_port(fd, "DATA:WIDth 1\r", 13)));
    CHECK((ret = write_port(fd, "DATA:ENCDG SRIBINARY\r", 21)));
    CHECK((ret = write_port(fd, "HEADER OFF\r", 11)));
    CHECK((ret = write_port(fd, "ACQUIRE:STATE ON\r", 17)));

exit:
    return ret;
}