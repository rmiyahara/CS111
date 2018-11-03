/*
NAME: Ryan Miyahara
EMAIL: rmiyahara144@gmail.com
ID: 804585999
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>

//Global variables
int thread_count = 1;
int iteration_count = 1;
bool insert_flag = false;
bool delete_flag = false;
bool lookups_flag = false;
bool debug = false;

void debug_print(int mes) {
    switch(mes) {
        case 0:
            printf("Debug mode activated.\n");
            break;
        case 1:
            printf("%d threads specified.\n", thread_count);
            break;
        case 2:
            printf("%d iterations specified.\n", iteration_count);
            break;
        default:
            printf("You shouldn't get here!\n");
    }
    return;
}

int main(int argc, char** argv) {
    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"threads", optional_argument, NULL, 't'},
        {"iterations", optional_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
        {"debug", no_argument, NULL, 'd'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed
    
    //Set params
    while ((curr_param = getopt_long(argc, argv, "tiy:d", flags, NULL)) != -1) {
        switch (curr_param) {
            case 't':
                if (optarg)
                    thread_count = atoi(optarg);
                break;
            case 'i':
                if (optarg)
                    iteration_count = atoi(optarg);
                break;
            case 'y':
                int i;
                for (i = 0; i < strlen(optarg); i++) {
                    if (optarg[i] == 'i') insert_flag = true;
                    else if (optarg[i] == 'd') delete_flag = true;
                    else if (optarg[i] == 'l') lookups_flag = true;
                    else {
                        fprintf(stderr, "Please set --yield=[idl] or any combination of those 3.\n");
                        exit(1);
                    }
                }
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab2_list [--threads=#threads --iterations=#iterations --yield[idl] --debug]\n");
                exit(1);
        }
    }
    if (debug) {
        int i;
        for (i = 0; i < 3; i++)
            debug_print(i);
    }

    exit(0); //Successful exit
}