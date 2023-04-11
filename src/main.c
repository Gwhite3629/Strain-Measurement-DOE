//  Start of runtime program
//  Dynamic Strain Sensor
//  Grady White
//  1/26/22

#include <windows.h>
#include <time.h>

#include "utils.h"
#include "file.h"
#include "serial.h"
#include "setup.h"
#include "runtime.h"

int main(void)
{
    int ret = 0;
    HANDLE fd = NULL;
    char dev[64];
    time_t program_start_time = 0;
    FILE *datafile = NULL;

    time(&program_start_time);

    printf("Enter Device Name: ");
    scanf("%s", dev);

    CHECK((ret = init_datafile(&datafile, program_start_time)));
    if (datafile == NULL)
        printf("File couldn't be openned\n");

    VALID((fd = open_port(dev)), "Could not open port");
    printf("Opened\n");
    CHECK((ret = GPIB_conf(fd, 0)));
    printf("GPIB configured\n");
    CHECK((ret = runtime(fd, program_start_time, datafile)));
    printf("Completed\n");
    CHECK((ret = write_port(fd, "ACQUIRE:STATE OFF\r", 18)));

exit:
    if(datafile)
        fclose(datafile);
    if(fd)
        CloseHandle(fd);

    return ret;
}