/*
NAME: Ryan Miyahara
EMAIL: rmiyahara144@gmail.com
ID: 804585999
*/
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <mraa.h>
#include <mraa/aio.h>
#include "fcntl.h"

//Global Variables
int period = 1;
char degrees = 'F';
bool debug = false;
char* log_filename = NULL;
int log_fd;
mraa_aio_context temp;
mraa_gpio_context butt;

void debug_print(int mes) {
    switch(mes) {
        case 0: //Initialize
            printf("Debug mode activated!\nPeriod set to: %d\nDegrees will be printed in: %c\n", period, degrees);
            break;
        case 1: //Log mode
            printf("Log mode activated!\nLog will be printed in: %s\n", log_filename);
            break;
        case 2: //When start_sensors() is called
            printf("Initializing the sensors.\n");
            break;
        case 3: //When rip_sensors() is called
            printf("Goodbye sensors.\n");
            break;
        default:
            printf("You shouldn't get here!\n");
    }
    return;
}

void time_print() { //Uses localtime to print formatted time
    time_t timer;
    time(&timer);
    struct tm* time = localtime(&timer);
    fprintf(stdout, "%.2d:%.2d:%.2d \n", time->tm_hour, time->tm_min, time->tm_sec);
    return;
}

//Sensor Functions
void rip_sensors() { //Closes log_fd and sensors
    if (debug) debug_print(3);
    if (log_filename)
        close(log_fd);
    mraa_aio_close(temp);
    mraa_gpio_close(butt);
    return;
}

void start_sensors() { //Initializes temperature sensor and button
    if (debug) debug_print(2);

    //Open temperature sensor
    temp = mraa_aio_init(1);
    if (!temp) {
        fprintf(stderr, "Unable to open temperature sensor.\nError message: %s\nError number: %d\n", strerror(errno), errno);
        exit(1);
    }

    //Open button
    butt = mraa_gpio_init(60);
    if (!butt) {
        fprintf(stderr, "Unable to open the button.\nError message: %s\nError number: %d\n", strerror(errno), errno);
        exit(1);
    }

    atexit(rip_sensors);
    return;
}

int main(int argc, char** argv) {
    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
        {"debug", no_argument, NULL, 'd'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed
    //int i; //Iterator

    //Set params
    while ((curr_param = getopt_long(argc, argv, "p:s:l:d", flags, NULL)) != -1) {
        switch (curr_param) {
            case 'p':
                period = atoi(optarg);
                if (period <= 0) {
                    fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab4b [--period=seconds --scale=[CF] --log=filename --debug]\n");
                    exit(1);
                }
                break;
            case 's':
                degrees = optarg[0];
                if (degrees == 'c')
                    degrees = 'C';
                else if (degrees == 'f')
                    degrees = 'F';
                if (degrees != 'C' && degrees != 'F') {
                    fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab4b [--period=seconds --scale=[CF] --log=filename --debug]\n");
                    exit(1);
                }
                break;
            case 'l':
                log_filename = optarg;
                log_fd = creat(optarg, S_IRUSR | S_IWUSR);
                if (log_fd < 0) {
                    fprintf(stderr, "Unable to open log_fd.\nError message: %s\nError number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                break;
            case 'd':
                debug = true;
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab4b [--period=# --scale=[CF] --log=filename --debug]\n");
                exit(1);
        }
    }
    if (debug) debug_print(0);
    if (debug && log_filename) debug_print(1);

    start_sensors();
    time_print();

    exit(0); //Successful exit
}