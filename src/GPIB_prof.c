//  GPIB Control
//  Dynamic Strain Sensor
//  Grady White
//  1/3/22

#include <windows.h>
#include "serial.h"
#include "utils.h"

//Default profile
int def(HANDLE fd)
{
    int ret = 0;

    //CHECK((ret = write_port(fd, "++mods 1\r", 9)));
    CHECK((ret = write_port(fd, "++addr 1\r", 9)));
    //CHECK((ret = write_port(fd, "++eoi 0\r", 8)));
    //CHECK((ret = write_port(fd, "++eot_enable 1\r", 15)));
    //CHECK((ret = write_port(fd, "++eot_char 13\r", 14)));
    CHECK((ret = write_port(fd, "++ifc\r", 6)));
    CHECK((ret = write_port(fd, "++auto 1\r", 9)));

exit:
    return ret;
}