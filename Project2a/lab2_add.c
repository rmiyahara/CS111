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
bool opt_yield = false;
char sync = 'f';
char* tag = "";
pthread_mutex_t lockerino;

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
        case 7:
            printf("Yield flag raised\n");
            break;
        case 8:
            printf("Sync \'%c\' set.\n", sync);
            break;
        case 9:
            printf("Label set to: %s\n", tag);
            break;
        case 10:
            printf("Counter at: %ld\n", counter);
            break;
        default:
            printf("You should never get here!\n");
    }
    return;
}


char* label() {
    if (opt_yield) {
        switch (sync) {
            case 'm':
                return "add-yield-m";
            case 's':
                return "add-yield-s";
            case 'c':
                return "add-yield-c";
            default:
                return "add-yield-none";
        }
    }
    else {
        switch (sync) {
            case 'm':
                return "add-m";
            case 's':
                return "add-s";
            case 'c':
                return "add-c";
            default:
                return "add-none";
        }
    }
}

//Crazy arithmetic functions
void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
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
    if (debug) debug_print(10);
}

void rip_locks() {
    pthread_mutex_destroy(&lockerino);
}

int main(int argc, char** argv) {

    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"threads", optional_argument, NULL, 't'},
        {"iterations", optional_argument, NULL, 'i'},
        {"yield", no_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
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
            case 'y':
                opt_yield = true;
                break;
            case 's':
                sync = optarg[0];
                if (sync != 'm' && sync != 's' && sync != 'c') {
                    fprintf(stderr, "Sync must be set to m, s, or c\n");
                    exit(1);
                }
                if (sync == 'm') {
                    atexit(rip_locks);
                }
                break;
            case 'd':
                debug = true;
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab2_add [--threads=#threads --iterations=#iterations --yield --sync=[m, s, c] --debug]\n");
                exit(1);
        }
    }
    if (debug) {
        int i;
        for (i = 0; i < 3; i++)
            debug_print(i);
    }
    if (debug && opt_yield) debug_print(7);
    if (debug && (sync != 'f')) debug_print(8);

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

    //Get data for CSV
    //Get name of test
    tag = label();
    if (debug) debug_print(9);
    //Get total number of operations performed
    long num_ops = thread_count * iteration_count * 2;
    //Get total run time (in nanoseconds)
    long runtime = start.tv_nsec - end.tv_nsec;
    long average_runtime = runtime / num_ops;

    //Print CSV
    printf("%s,%d,%d,%ld,%ld,%ld,%ld", tag, thread_count, iteration_count, num_ops, runtime, average_runtime, counter);

    exit(0); //Sucessful exit
}