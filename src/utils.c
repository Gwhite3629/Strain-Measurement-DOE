//  Utility Functions
//  Dynamic Strain Sensor
//  Grady White
//  1/7/22

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "utils.h"

double RMS(float *window, uint windowSize, float *sum, uint *pos, float *input)
{
    double avg;
    for (int i = 0; i < CURVE_SIZE*BIT_SIZE; i++) {
        avg = calc_RMS(window, windowSize, sum, (*pos), input[i]);
        (*pos)++;
        if ((*pos) == (windowSize-1))
            (*pos) = 0;
        //printf("pos: %f\n", window[(*pos)]);
    }
    return sqrt(avg);
}

double calc_RMS(float *window, uint windowSize, float *sum, uint pos, float next)
{
    // Update sum
    (*sum) = (*sum) - window[pos] + ((next*next)/windowSize);
    //printf("Sum: %3.3f, prev: %f, next: %f\n", (*sum), (window[pos]), (float)(fabs(next)));
    // Update window
    window[pos] = (next*next)/windowSize;
    // Average
    return (*sum);
}

float ReverseFloat(const float inFloat)
{
   float retVal;
   char *floatToConvert = ( char* ) & inFloat;
   char *returnFloat = ( char* ) & retVal;

   // swap the bytes into a temporary buffer
   returnFloat[0] = floatToConvert[3];
   returnFloat[1] = floatToConvert[2];
   returnFloat[2] = floatToConvert[1];
   returnFloat[3] = floatToConvert[0];

   return retVal;
}

float *linspace(float start, float stop, int num)
{
    int ret = 0;
    float *array = NULL;
    MEM(array, num, float);

    float dif = (stop-start)/num;

    for (int i = 0; i < num; i++) {
        array[i] = start + dif*i;
    }

exit:
    if (ret == 0)
        return array;
    else
        return NULL;
}