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

//Global Variables
bool debug = false;
long counter = 0;
int thread_count = 1;
int iteration_count = 1;

void debug_print(int mes) {
    switch (mes) {
        case 0:
            printf("Debug mode activated.\n");
            break;
        case 1:
            printf("%d threads specified.\n", thread_count);
            break;
        case 2:
            printf("%d iterations specified.\n", iteration_count);
            break;
        case 3:
            printf("Started the timer.\n");
            break;
        case 4:
            printf("Ended the timer.\n");
            break;
        case 5:
            printf("Started threads\n");
            break;
        case 6:
            printf("Waited on the threads.\n");
            break;
        default:
            printf("You should never get here!\n");
    }
    return;
}

//Crazy arithmetic functions
void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    *pointer = sum;
    }

void quick_maths(long long *pointer) {
    add(pointer, 1);
    add(pointer, -1);
}

//Thread functions
void dothething() {
    //Start threads
    pthread_t spawn[thread_count];
    int i;
    for (i = 0; i < thread_count; i++) {
        if (pthread_create(&spawn[i], NULL, (void*) &quick_maths, &counter) < 0) {
            fprintf(stderr, "Unable to create thread %d.\nError message: %s\n Error number: %d\n", i, strerror(errno), errno);
            exit(1);
        }
    }
    if (debug) debug_print(5);

    //Wait for threads
    for (int i = 0; i < thread_count; i++) {
        if (pthread_join(spawn[i], NULL) < 0) {
            fprintf(stderr, "Unable to wait for thread %d.\nError message: %s\n Error number: %d\n", i, strerror(errno), errno);
            exit(1);
        }
    }
    if (debug) debug_print(6);
}

int main(int argc, char** argv) {

    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"threads", optional_argument, NULL, 't'},
        {"iterations", optional_argument, NULL, 'i'},
        {"debug", no_argument, NULL, 'd'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed
    
    //Set params
    while ((curr_param = getopt_long(argc, argv, "s:d", flags, NULL)) != -1) {
        switch (curr_param) {
            case 't':
                if (optarg)
                    thread_count = atoi(optarg);
                break;
            case 'i':
                if (optarg)
                    iteration_count = atoi(optarg);
                break;
            case 'd':
                debug = true;
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab1a [--shell=program --degbug]\nTry setting program to \"\\bin\\bash\"\n");
                exit(1);
        }
    }
    if (debug) {
        int i;
        for (int i = 0; i < 3; i++)
            debug_print(i);
    }

    //Mark start
    struct timespec start;
    if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
        fprintf(stderr, "Unable to set start time.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(3);

    dothething(); //Adds and subtracts threads

    //Mark end
    struct timespec end;
    if (clock_gettime(CLOCK_MONOTONIC, &end) < 0) {
        fprintf(stderr, "Unable to set end time.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(4);

    printf("Counter: %d\n", counter);

    exit(0); //Sucessful exit
}