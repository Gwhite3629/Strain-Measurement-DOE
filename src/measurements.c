//  Measurements Control
//  Dynamic Strain Sensor
//  Grady White
//  1/3/22

/*
Functions in this file will send commands and queries.
Currently it is unclear if these should be sent over 2 GPIB channels.
Potentially one channel to send queries and one channel to receive data.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <math.h>

#include "utils.h"
#include "serial.h"
#include "file.h"

// Acquire entire data curve every 5ms
int get_curve(HANDLE fd, float **output)
{
    int ret = 0;
    int8_t *data = NULL;
    float y_off = 0;
    float y_mult = 0;
    float y_zero = 0;
    char c[120];

    //CHECK((ret = write_port(fd, "*CLS\r", 5)));
    //CHECK((ret = write_port(fd, "ACQUIRE:STATE ON\r", 17)));
    //CHECK((ret = write_port(fd, "*OPC\r", 5)));
    //CHECK((ret = write_port(fd, "HEADER OFF\r", 11)));

    MEM(data, (CURVE_SIZE*BIT_SIZE), int8_t);
    //printf("Allocated\n");
    CHECK((ret = write_port(fd, "CURVE?\r", 7)));
    //printf("Written\n");
    //memset(c, '\0', 120);
    CHECK((ret = read_port(fd, c, 6)));
    //printf("Header: %s\n", c);
    //printf("Bytes read: %d\n", ret);
    CHECK((ret = read_port(fd, data, (CURVE_SIZE*BIT_SIZE))));
    //printf("Read\nData:\n");

    //printf("Bytes read: %d\n", ret);

    //memset(c, '\0', 120);
    CHECK((ret = read_port(fd, c, 2)));

    /*for (int i = 0; i<ret; i++) {
        printf("Garbage: %d\n", (int8_t)c[i]);
    }*/

    //printf("Bytes read: %d\n", ret);

    //memset(c, '\0', 120);
    CHECK((ret = write_port(fd, "WFMPRE:YOFF?;YMULT?;YZERO?\r", 27)));
    CHECK((ret = read_port(fd, c, 20)));

    //printf("Bytes read: %d\n", ret);

    //printf("C: %s\n", c);

    sscanf(c, "%e;%e;%e", &y_off, &y_mult, &y_zero);

    //printf("y_off %e\ny_mult %e\ny_zero %e\n", (y_off), (y_mult), (y_zero));

    for(int i = 0; i < (CURVE_SIZE*BIT_SIZE); i++) {
        (*output)[i] = ((data[i]-(y_off)) * (y_mult)) + (y_zero);
        //printf(" |%f, %d| ", (*output)[i], data[i]);
    }

    //printf("\n");

exit:
    if (data)
        free(data);

    return ret;
}

int full_measure(SUPER *input)
{
    int ret = 0;
    //printf("Starting file operations\n");
    CHECK((ret = datafile_write(input)));

exit:
    return ret;
}

int get_fft(SUPER *input)
{
    int ret = 0;
    FILE *fft;
    char *filename = NULL;
    time_t cur_time;
    char *datestring = NULL;
    struct tm *datetime;

    time(&cur_time);

    datetime = localtime(&cur_time);

    MEM(datestring, 64, char);
    MEM(filename, 128, char);

    strftime(datestring, 64, "%a,%b-%d-%Y-at-%H-%M-%SEST", datetime);

    MEM(filename, 80, char);

    sprintf(filename, "FFTDataFor%s", datestring);

    VALID((fft = fopen(filename, "w")), "Failed to open file");

    for(int i = 0; i < (CURVE_SIZE*BIT_SIZE); i++) {
        CHECK((ret = fprintf(fft, "%f, %f\n", input->time_series[i], input->peak->data[i])));
    }

exit:
    if (fft)
        fclose(fft);
    if (datestring)
        free(datestring);
    if (filename)
        free(filename);

    return ret;
}