# Introduction:
This is a program to measure voltage froma Tektronix TDS1000 oscilloscope. The program is structured to measure according to user defined parameters that are set in the `config.cfg` file.

# Usage:
## Running:
The program can be launched by opening the executable file `mainstatic.exe`. This is the statically compiled version of the program and it should run on any Windows computer, though it has only been tested on Windows 10.

## Configuring:
The program config consists of several lines of numbers which define the measurement type and parameters. The file `config.cfg` contains a template at the top which defines the order of parameters.
The config file is set up so that multiple measurement profiles are available. This is done by putting a label followed by a newline and putting fields on seperate lines below. There is an example called `DEFAULT` in the `config.cfg` file.
The parameters and their function are described below:
|Parameter       |Function|
|----------------|--------|
|NUM_BARRIER     |Number of barrier thresholds.|
|THRESHOLD (n)   |Threshold value, must have th amount of thresholds defined in `NUM_BARRIER`.|
|WINDOW_SIZE     |The size of the RMS calculation window, recommended to leave at 2500.|
|PEAK_THRESHOLD  |The value for peak detection.|
|SAMPLE_FREQUENCY|Sampling frequency, used for debugging.|
|MEASURE_INTERVAL|The time interval for the timer measurements in seconds.|
|HSCALE          |The horizontal scale of the oscilloscope.*|
|VSCALE          |The vertical scale of the oscilloscope.|

*`HSCALE` is used internally and should be set appropriately, but it must be set manually on the oscilloscope. The reason for this is unknown.

# Concepts:

## RMS calculation:

The RMS measurement is intended to maintain constant measurement without varying the sample rate on peaks. The RMS measurement finds the root mean squared value of the last `WINDOW_SIZE` number of samples from the oscilloscope. The RMS is intended to give a good idea of strain magnitude over a time frame, and is thus good for trends and quite bad for spikes in activity.

## Time stamps:

Time stamps are the date and time that the measurement was taken. Time inside data files is in format of seconds since the beginning of program execution.

## Data:

Measurements are saved in specific cases.  
### 1. **Large variation of RMS**

Variations in the RMS mean that the strain has met one of several threshold values that are pre-defined and determined empirically. These values determine when a significant event is occuring. The relative significance is determined by the thresholds.

### 2. **Fixed increment in time**

Data should be saved in cases of obvious vibration, but general behaviour is also important. Data is saved sporadically to ensure data is plentiful.  

### 3. **Large spikes:**

Data is recorded when large spikes in magnitude are detected. These are expected to be frequent and will have thresholds associated with them. This method is to ensure that the rolling average doesn't miss any important fluctuations.  

# Code:
Specific comments are provided in each source and header file.

## config.cfg
This config file contains a basic control scheme for the program. Fields are defined at the top of the config file. Profiles can be created by placing a unique name in all caps under a profile and filling the fields according to the structure defined at the top of the file. When the program asks for the name of the config you will type the name of the desired profile. The default profile is called `DEFAULT`.

## main.c:

The purpose of the main function is to have an easy to access initialization to the program. In this file the serial device will be registered and configured. The runtime environment will also be initialized and given appropriate arguments.

## runtime.c:

The runtime environment in this function is responsible for dynamically acquiring and moving data. There are many locks in this file to ensure no data corruption. Testing of the memory management has shown that the code is stable over long periods without race conditions, deadlocks, or memory leaks.

## file.c

Controls file operations. Functions in this file are entirely responsible for every read and write to files in this program. Functions are:

**datafile_write**

This writes below the header in the primary data file.

**init_datafile**

This writes the header for the primary data file.

**get_config**

This loads the user config data for the selected profile into the config field of the `SUPER` struct.

**find_event**

This function searches for the first occurance of a string in the designated file and returns the files position as well as sets the file to that location.

## measurements.c

Controls data acquisition from the oscilloscope. Functions in this file are entirely responsible for high level control and operation of serial devices.
Functions are:

**get_curve**

Sends appropriate commands and retrieves raw voltage data from oscilloscope.

**full_measure**

Intermediate function which sends data from runtime to the data file.

**get_fft**

Deprecated function which was meant to get the FFT of the data from the oscilloscope. This function isn't necessary because the FFT can be calculated in post-processing.

## serial.c

General functions for serial control. Uses windows serial functionality.

## GPIB_prof.c

Simple GPIB controls. There is only one profile named `def`, this is the default configuration. Other configurations can easily be added by creating a function that looks like `def` in this file and adding a case to the `GPIB_conf` function in `serial.c`. Finally the profile is selected in `main.c`.

## setup.c

This file controls the initial instrument setup. This includes the instrument time scale and vertical scale.

## utils.c

This file has some utility functions which are used throughout. The RMS functions are contained here. One is used to work through the array and one is used on individual datum. The other functions are currently useless.

# Using data:

The data that is output comes in the form of a single ASCII text file. This can be inconvenient for large data sets. If the need arises the data file can be stripped to only include necessary data as well as a binary representation to significantly reduce size.
The data file contains several fields which are useful and some which currently aren't functional. The reason for why some don't work is currently unknown. Believe me I tried.

**Barrier**

This says which barrier was triggered last.

**Up(1) | Down(0)**

This defines which way the barrier was crossed.

**Measure type**

This is the type of measurment,

    1 = Barrier cross
    2 = Transient barrier cross
    3 = Timed measurement

**Time**

This field does not work. To get the time series use the oscilloscope window-size of 2500 and the horizontal scale value. For example every 2500 points take the time value from the time field, which is correct and increment that value by HSCALE/2500 for every point. The first value of every 2500 point set is correct. All time measured is relative to the measurement start date and time which is contained in the header and title of the data file.

**Magnitude**

This is the actual voltage data from the oscilloscope. An RMS calculation is provided inside the file, but this is raw data that can be manipulated to make an FFT or otherwise.

# Building:

This program relies on C standard libraries and POSIX standard libraries. Any compiler that can use POSIX libraries will correctly compile this program. Releases are built using MinGW-x64 and gcc. Windows releases are statically linked to ensure that it is portable to any windows computer. Linux or POSIX builds do not need to be statically linked.

Single line Windows compilation:
```
gcc -pthread -static -o DynamicSensor.exe main.c file.c GPIB_prof.c measurements.c runtime.c serial.c setup.c utils.c
```

Single line Linux compilation:
```
gcc -pthread -o DynamicSensor main.c file.c GPIB_prof.c measurement.c runtime.c serial.c setup.c utils.c
```
There is also a makefile to build static and dynamically linked versions of the program. This makefile is intended to work with the version of make that can be downloaded with the `Chocolatey` package manager.

    Acknowledgements:  This work was supported by the U.S. Department of Energy, Office of Science, Office of Basic Energy Sciences Established Program to Stimulate Competitive Research (EPSCoR) under Award DE SC0020126.

    Disclaimer:  This code was prepared as an account of work sponsored by an agency of the United States Government. Neither the United States Government nor any agency thereof, nor any of their employees, makes any warranty, express or implied, or assumes any legal liability or responsibility for the accuracy, completeness, or usefulness of any information, apparatus, product, or process disclosed, or represents that its use would not infringe privately owned rights. Reference herein to any specific commercial product, process, or service by trade name, trademark, manufacturer, or otherwise does not necessarily constitute or imply its endorsement, recommendation, or favoring by the United States Government or any agency thereof. The views and opinions of authors expressed herein do not necessarily state or reflect those of the United States Government or any agency thereof.


