//  Runtime Control
//  Dynamic Strain Sensor
//  Grady White
//  1/3/22

#include <stdbool.h>
#include <pthread.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdatomic.h>
#include <math.h>

#include "measurements.h"
#include "file.h"
#include "utils.h"
#include "setup.h"
#include "serial.h"

atomic_bool exit_runtime = 0;
atomic_bool timer = 0;
atomic_bool transient;
pthread_spinlock_t lock;

void *quit_condition(void *arg)
{
    const char *check = (char *)arg;
    char detect[64];

    while(!exit_runtime) {
        scanf("%s", detect);
        if (strcmp(detect, check) ==  0) {
            exit_runtime = 1;
        }
    }

    pthread_exit(NULL);
    return NULL;
}

void *time_measure(void *arg)
{
    int ret = 0;
    uint remaining = 15; // 15 seconds for a window size of 0.5sec
    uint interval = ((SUPER *)arg)->config->MEASURE_INTERVAL;

    //printf("Interval: %d\n", conf->MEASURE_INTERVAL);
    
    while(!exit_runtime) {
        usleep((interval)*1000000); // 285 seconds -> 5min-15sec
        while (remaining > 0) {
            pthread_spin_lock(&lock);
            ((SUPER *)arg)->type = 3; // Type 3 measurement, timed
            CHECK((ret = full_measure(((SUPER *)arg))));
            pthread_spin_unlock(&lock);
            printf("\n||Measured on time interval||\n\n");
            remaining--;
        }
        remaining = 15;
    }

exit:
    pthread_exit(NULL);
    return NULL;
}

void *find_peak(void *arg)
{
    pthread_spin_lock(&lock);
    for (int i = 0; i < CURVE_SIZE*BIT_SIZE; i++) {
        if (((SUPER *)arg)->peak->data[i] > ((SUPER *)arg)->peak->peak_threshold) {
            ((SUPER *)arg)->peak->detected = 1;
            break;
        }
    }
    pthread_spin_unlock(&lock);

    pthread_exit(NULL);
    return NULL;
}

void *transient_record(void *arg)
{
    uint remaining = 15; // 15, 15seconds at window size of 0.5sec
    uint minutes = 5;
    int ret = 0;
    while (minutes > 0) {
        while (remaining > 0) {
            usleep(10); // Allow main thread to get data
            pthread_spin_lock(&lock);
            ((SUPER *)arg)->type = 2;
            CHECK((ret = full_measure(((SUPER *)arg))));
            remaining--;
            pthread_spin_unlock(&lock);
        }
        usleep(45*1000000); // Sleep for 45 seconds, remaining part of minute
        minutes--;
    }
    transient = 0;

exit:
    pthread_exit(NULL);
    return NULL;
}

int runtime(HANDLE dev, time_t program_start_time, FILE *datafile)
{
    int ret = 0;
    uint pos = 0;
    int barrier = 0; // Current threshold
    SUPER *input;
    uint remaining = 15;

    static float *window = NULL;
    float *time_series = NULL;

    float sum = 0;

    clock_t start_time = 0;
    clock_t end_time = 0;

    pthread_t exit_thr;
    pthread_t peak_thr;
    pthread_t time_thr;
    pthread_t tran_thr;

    MEM(input, 1, SUPER);
    MEM(input->time_series, CURVE_SIZE*BIT_SIZE, float);

    input->program_start_time = program_start_time;
    input->datafile = datafile;
    CHECK((ret = pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE)));

    // Get config data
    char name[64] = "DEFAULT";
    printf("Enter config name:\n");
    scanf("%s", name);
    MEM(input->config, 1, Config);
    get_config(input->config, name);

    printf("Intermediate interval: %u\n", input->config->MEASURE_INTERVAL);

    float max = strtof(input->config->HSCALE, NULL);

    time_series = linspace(0, max, CURVE_SIZE*BIT_SIZE);
    memcpy(input->time_series, time_series, CURVE_SIZE*BIT_SIZE*sizeof(float));
    free(time_series);

    CHECK((ret = write_port(dev, "ACQUIRE:STATE OFF\r", 18)));
    CHECK((ret = init(dev, input->config)));
    printf("Setup complete\n");

    HANDLE_ERR((ret = pthread_create(&exit_thr, NULL, &quit_condition, "quit")), "pthread_create");
    HANDLE_ERR((ret = pthread_create(&time_thr, NULL, &time_measure, input)), "pthread_create");

    // Setup window
    int pos_max = input->config->AVERAGE_DATA.WINDOW_SIZE;
    MEM(window, pos_max, float);
    for (int i = 0; i<pos_max; i++) {
        window[i] = 0.0;
    }
    MEM(input->peak, 1, PEAK);
    MEM(input->peak->data, CURVE_SIZE*BIT_SIZE, float);
    input->peak->peak_threshold = input->config->PEAK_THRESHOLD;

    printf("Entering runtime\n");

    // Main runtime loop
    while(!exit_runtime) {
        start_time = clock();
        //Get most recent strain

        pthread_spin_lock(&lock);
        memset(input->peak->data, 0, CURVE_SIZE*BIT_SIZE);
        input->peak->detected = 0;
        pthread_spin_unlock(&lock);
        //printf("Setting peak\n");

        pthread_spin_lock(&lock);
        //printf("Getting data\n");
        CHECK((ret = get_curve(dev, &(input->peak->data))));
        //printf("Got data: %f\n", input->peak->data[0]);
        pthread_spin_unlock(&lock);

        HANDLE_ERR((ret = pthread_create(&peak_thr, NULL, &find_peak, input)), "pthread_create");
        // Average the strain

        pthread_spin_lock(&lock);
        input->avg = RMS(window, pos_max, &sum, &pos, input->peak->data);
        printf("Average %lf\n", input->avg);
        pthread_spin_unlock(&lock);

        if (pos == (input->config->AVERAGE_DATA.WINDOW_SIZE-1))
            pos = 0;

        //printf("Joining thread\n");
        pthread_join(peak_thr, NULL);
        //printf("Thread joined\n");

        /*if (input->peak->detected == 1) {
            pthread_spin_lock(&lock);
            CHECK((ret = full_measure(input)));
            pthread_spin_unlock(&lock);
            printf("\033[0;34m");
            printf("Measured peak\n");
            printf("\033[0;37m");
        }*/

        // Measurement conditions (non-timer)
        if ((input->avg > input->config->AVERAGE_DATA.AVERAGE_THRESHOLDS[barrier]) & (barrier < (input->config->AVERAGE_DATA.NUM_BARRIER - 1)) & !(timer)) {
            if ((input->config->AVERAGE_DATA.NUM_BARRIER - 1)) {
                barrier++;
                printf("\033[0;32m");
                printf("Increased barrier\n");
                printf("\033[0;37m");
            }
            pthread_spin_lock(&lock);
            input->crossing = 1;
            input->type = 1;
            timer = 1;
            input->crossed = input->config->AVERAGE_DATA.AVERAGE_THRESHOLDS[barrier-1];
            CHECK((ret = full_measure(input)));
            pthread_spin_unlock(&lock);
            printf("\033[0;34m");
            printf("Measured avg barrier\n");
            printf("\033[0;37m");
        } else if ((input->avg < input->config->AVERAGE_DATA.AVERAGE_THRESHOLDS[barrier-1]) & !(timer)) {
            if (barrier > 0)
                barrier--;
            pthread_spin_lock(&lock);
            input->crossing = 0;
            input->type = 1;
            timer = 1;
            input->crossed = input->config->AVERAGE_DATA.AVERAGE_THRESHOLDS[barrier+1];
            CHECK((ret = full_measure(input)));
            pthread_spin_unlock(&lock);
            printf("\033[0;31m");
            printf("Decreased barrier\n");
            printf("\033[0;37m");
        } else if (timer) {
            pthread_spin_lock(&lock);
            CHECK((ret = full_measure(input)));
            pthread_spin_unlock(&lock);
            printf("Remaining: %d\n", remaining);
            remaining--;
            if (remaining == 0) {
                transient = 1;
                HANDLE_ERR((ret = pthread_create(&tran_thr, NULL, &transient_record, input)), "pthread_create");
                timer = 0;
                remaining = 15;
            }
        }   /*else if (barrier > (input->config->AVERAGE_DATA.NUM_BARRIER - 2)) {
            pthread_spin_lock(&lock);
            input->crossing = 1;
            input->type = 1;
            CHECK((ret = full_measure(input)));
            pthread_spin_unlock(&lock);
            printf("\033[0;33m");
            printf("Measured fft barrier\n");
            printf("\033[0;37m");
        }*/

        end_time = clock();
        printf("Loop took: %f seconds\n\n", (float)(end_time-start_time)/CLOCKS_PER_SEC);
        usleep((int)(1000000*1));
    }

exit:

    pthread_spin_destroy(&lock);

    if (window)
        free(window);
    if (input->config->AVERAGE_DATA.AVERAGE_THRESHOLDS)
        free(input->config->AVERAGE_DATA.AVERAGE_THRESHOLDS);
    if (input->config)
        free(input->config);
    if (input->peak->data)
        free(input->peak->data);
    if (input->peak)
        free(input->peak);
    if (input)
        free(input);

    return ret;
}