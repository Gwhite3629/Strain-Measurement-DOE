//  File Control
//  Dynamic Strain Sensor
//  Grady White
//  1/3/22

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int find_event(FILE *fd, fpos_t *prof_start, char *locator)
{
    int ret = 0;
    int flag = 0;
    int found = 0;
    char *buffer = NULL;

    MEM(buffer, strlen(locator) + 1, char);

    buffer[strlen(locator)] = '\0';

    do {
        buffer[flag] = fgetc(fd);
        if (buffer[flag] == locator[flag]) {
            if (flag == (int)strlen(locator) - 1)
                found++;
            else
                flag++;
        } else
            flag = 0;
        //printf("buffer: %s\n", buffer);
    } while (found < 1);

    fgetpos(fd, prof_start);
    
exit:
    free(buffer);
    return ret;
}

int get_config(Config *config, char *name)
{
    int ret = 0;

    FILE *config_file = NULL;
    fpos_t *config_start = NULL;

    MEM(config_file, sizeof(FILE), FILE);

    MEM(config_start, sizeof(fpos_t), fpos_t);

    VALID((config_file = fopen("config.cfg", "r")), FILE_ERROR);

    find_event(config_file, config_start, name);

    //fsetpos(config_file, config_start);

    fscanf(config_file, "%d\n", &(config->AVERAGE_DATA.NUM_BARRIER));

    MEM(config->AVERAGE_DATA.AVERAGE_THRESHOLDS, config->AVERAGE_DATA.NUM_BARRIER, float);

    for (int i = 0; i < config->AVERAGE_DATA.NUM_BARRIER; i++) {
        fscanf(config_file, "%f\n", &(config->AVERAGE_DATA.AVERAGE_THRESHOLDS[i]));
        printf("Threshold%d: %f\n", i, config->AVERAGE_DATA.AVERAGE_THRESHOLDS[i]);
    }
    fscanf(config_file, "%u", &(config->AVERAGE_DATA.WINDOW_SIZE));
    fscanf(config_file, "%f", &(config->PEAK_THRESHOLD));
    fscanf(config_file, "%u", &(config->SAMPLE_FREQUENCY));
    fscanf(config_file, "%u", &(config->MEASURE_INTERVAL));
    fscanf(config_file, "%s", config->HSCALE);
    fscanf(config_file, "%s", config->VSCALE);
    printf("Recorded Interval: %u\n", config->MEASURE_INTERVAL);

exit:
    if (config_start)
        free(config_start);
    if (config_file)
        fclose(config_file);
    return ret;
}

int init_datafile(FILE **datafile, time_t program_start_time)
{
    int ret = 0;
    char *filename = NULL;
    char *datestring = NULL;
    struct tm *datetime;

    datetime = localtime(&program_start_time);

    MEM(datestring, 64, char);
    MEM(filename, 128, char);

    strftime(datestring, 64, "%a,%b-%d-%Y-at-%H-%M-%SEST", datetime);

    sprintf(filename, "TestDataFor%s", datestring);

    //printf("%s\n", filename);

    VALID(((*datafile) = fopen(filename, "w")), "Error could not create file");
    //printf("ret: %d\n", ret);
    CHECK((ret = fprintf((*datafile), "Dynamic strain test\n")));
    //printf("ret: %d\n", ret);
    CHECK((ret = fprintf((*datafile), "Test start date: %s\n", datestring)));
    //printf("ret: %d\n", ret);
    CHECK((ret = fprintf((*datafile), "All times are offset from program start date\n")));
    //printf("ret: %d\n", ret);
    CHECK((ret = fprintf((*datafile), "<Barrier>, <Up(1) | Down(0)>,  <Measure type>, <Time>, <Magnitude>\n")));
    //printf("ret: %d\n", ret);

exit:
    if (datestring)
        free(datestring);
    if (filename)
        free(filename);

    return ret;
}

int datafile_write(SUPER *input)
{
    int ret = 0;
    time_t cur_time = 0;
    int measure_time = 0;

    time(&cur_time);
    //printf("Started time\n");
    measure_time = (int)difftime(cur_time, input->program_start_time);
    //printf("Finished time\n");

    if (input->datafile == NULL)
        printf("File no longer valid\n");

    for(int i = 0; i < (CURVE_SIZE*BIT_SIZE); i++) {
        CHECK((ret = fprintf(input->datafile, "%f, %d, %d, %f, %f\n", input->crossed, input->crossing, input->type, measure_time+input->time_series[i], input->peak->data[i])));
    }
    //printf("Printed data\n");

exit:

    return ret;
}