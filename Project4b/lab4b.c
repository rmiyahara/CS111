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

//Global Variables
int period = 1;
char degrees = 'F';
bool debug = false;
char* log_filename = NULL;

void debug_print(int mes) {
    switch(mes) {
        case 0: //Initialize
            printf("Debug mode activated!/nPeriod set to: %ds\nDegrees will be printed in: %c\n", period, degrees);
            break;
        case 1:
            printf("Log mode activated!\nLog will be printed in: %s\n", log_filename);
            break;
        default:
            printf("You shouldn't get here!\n");
    }
    return;
}

int main(int argc, char** argv) {
    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL 's'},
        {"log", required_argument, NULL, 'l'},
        {"debug", no_argument, NULL, 'd'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed
    int i; //Iterator

    //Set params
    while ((curr_param = getopt_long(argc, argv, "p:s:l:d", flags, NULL)) != -1) {
        switch (curr_param) {
            case 'p':
                period = atoi(optarg);
                break;
            case 's':
                degrees = optarg[0];
                if (degrees == 'c')
                    degrees = 'C';
                else if (degrees == 'f')
                    degrees = 'F';
                if (degrees != 'C' && degrees != 'F') {
                    fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab4b [--period=# --scale=[CF] --log=filename --debug]\n");
                    exit(1);
                }
                break;
            case 'l':
                log_filename = optarg;
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

    exit(0); //Successful exit
}