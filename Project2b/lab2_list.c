/*
NAME: Ryan Miyahara
EMAIL: rmiyahara144@gmail.com
ID: 804585999
*/

#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

//Global variables
int thread_count = 1;
int iteration_count = 1;
int list_count = 1;
bool insert_flag = false;
bool delete_flag = false;
bool lookups_flag = false;
bool debug = false;
int opt_yield = 0;
char* yield_tag;
char sync_tag = 'f';
pthread_mutex_t lockerino;
int spin_lockerino = 0;
SortedList_t* sortedlist;
SortedListElement_t* element;
int element_count = -1;

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
        case 3:
            printf("Started the timer.\n");
            break;
        case 4:
            printf("Ended the timer.\n");
            break;
        case 5:
            printf("Created mutex.\n");
            break;
        case 6:
            printf("Goodbye mutex.\n");
            break;
        case 7:
            printf("Started threads\n");
            break;
        case 8:
            printf("Waited on the threads.\n");
            break;
        default:
            printf("You shouldn't get here!\n");
    }
    return;
}

void rip_locks() {
    pthread_mutex_destroy(&lockerino);
    if(debug) debug_print(6);
}

void sig_handler(int mes) {
    if (mes == SIGSEGV) {
        fprintf(stderr, "SIGSEGV error.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(2);
    }
}

void label(char* hold) {
    if (!opt_yield)
        strcat(hold, "none");
    else
        strcat(hold, yield_tag);
    switch (sync_tag) {
        case 's':
            strcat(hold, "-s");
            break;
        case 'm':
            strcat(hold, "-m");
            break;
        default:
            strcat(hold, "-none");
    }
}

void* quick_maths(void* spawnid){
    signal(SIGSEGV, sig_handler);
    int hold = *((int*)spawnid);
    SortedListElement_t* insert_me;
    int length = 0;
    int i;
    for(i = hold; i < element_count; i += thread_count){ //Insert
        switch (sync_tag) {
            case 's':
                while(__sync_lock_test_and_set(&spin_lockerino, 1) == 1);
                SortedList_insert(sortedlist, &element[i]);
                __sync_lock_release(&spin_lockerino);
                break;
            case 'm':
                pthread_mutex_lock(&lockerino);
                SortedList_insert(sortedlist, &element[i]);
                pthread_mutex_unlock(&lockerino);
                break;
            default:
                SortedList_insert(sortedlist, &element[i]);
        }
    }
    
    switch (sync_tag) { //Length
            case 's':
                while(__sync_lock_test_and_set(&spin_lockerino, 1));
                length = SortedList_length(sortedlist);
                if (length < 0){
                    fprintf(stderr, "Length error.\n");
                    exit(2);
                }
                __sync_lock_release(&spin_lockerino);
                break;
            case 'm':
                pthread_mutex_lock(&lockerino);
                length = SortedList_length(sortedlist);
                if (length < 0){
                    fprintf(stderr, "Length error.\n");
                    exit(2);
                }
                pthread_mutex_unlock(&lockerino);
                break;
            default:
                length = SortedList_length(sortedlist);
                if (length < 0){
                    fprintf(stderr, "Length error.\n");
                    exit(2);
                }

    }
    hold = *((int*)spawnid);
    for(i = hold; i < element_count; i += thread_count) { //Lookup and delete
        switch (sync_tag) {
            case 's':
                while(__sync_lock_test_and_set(&spin_lockerino, 1));
                insert_me = SortedList_lookup(sortedlist, element[i].key);
                if(insert_me == NULL) {
                    fprintf(stderr, "Lookup error.\n");
                    exit(2);
                }
                if (SortedList_delete(insert_me) < 0) {
                    fprintf(stderr, "Delete error.\n");
                    exit(2);
                }
                __sync_lock_release(&spin_lockerino);
                break;
            case 'm':
                pthread_mutex_lock(&lockerino);
                insert_me = SortedList_lookup(sortedlist, element[i].key);
                if(insert_me == NULL){
                    fprintf(stderr, "Lookup error.\n");
                    exit(2);
                }
                if (SortedList_delete(insert_me) < 0) {
                    fprintf(stderr, "Delete error.\n");
                    exit(2);
                }
                pthread_mutex_unlock(&lockerino);
                break;
            default:
                insert_me = SortedList_lookup(sortedlist, element[i].key);
                if(insert_me == NULL){
                    fprintf(stderr, "Lookup error.\n");
                    exit(2);
                }
                if (SortedList_delete(insert_me) < 0) {
                    fprintf(stderr, "Delete error.\n");
                    exit(2);
                }
        }
    }
    return NULL;
}

void dothething() {
    //Start threads
    pthread_t spawn[thread_count]; //Holds all threads
    int spawnid[thread_count]; //Holds all thread ids
    int i;
    for (i = 0; i < thread_count; i++) {
        spawnid[i] = i;
        if (pthread_create(&spawn[i], NULL, quick_maths, &spawnid[i]) < 0) {
            fprintf(stderr, "Unable to create thread %d.\nError message: %s\n Error holdber: %d\n", i, strerror(errno), errno);
            exit(2);
        }
    }
    if (debug) debug_print(7);

    //Wait for threads
    for (int i = 0; i < thread_count; i++) {
        if (pthread_join(spawn[i], NULL) < 0) {
            fprintf(stderr, "Unable to wait for thread %d.\nError message: %s\n Error number: %d\n", i, strerror(errno), errno);
            exit(2);
        }
    }
    if (debug) debug_print(8);
}

int main(int argc, char** argv) {
    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"threads", optional_argument, NULL, 't'},
        {"iterations", optional_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {"lists", required_argument, NULL, 'l'},
        {"debug", no_argument, NULL, 'd'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed
    int i; //Iterator
    
    //Set params
    while ((curr_param = getopt_long(argc, argv, "tiy:s:d", flags, NULL)) != -1) {
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
                for (i = 0; i < (int)strlen(optarg); i++) {
                    if (optarg[i] == 'i') {
                        insert_flag = true;
                        opt_yield = opt_yield | INSERT_YIELD;
                    }
                    else if (optarg[i] == 'd') {
                        delete_flag = true;
                        opt_yield = opt_yield | DELETE_YIELD;
                    }
                    else if (optarg[i] == 'l') {
                        lookups_flag = true;
                        opt_yield = opt_yield | LOOKUP_YIELD;
                    }
                    else {
                        fprintf(stderr, "Please set --yield=[idl] or any combination of those 3.\n");
                        exit(1);
                    }
                }
                yield_tag = optarg;
                break;
            case 's':
                sync_tag = optarg[0];
                if (sync_tag == 'm') {
                    if(pthread_mutex_init(&lockerino, NULL) < 0) {
                        fprintf(stderr, "Mutex could not be initialized.\n");
                        exit(2);
                    }
                    atexit(rip_locks);
                }
                if (sync_tag != 'm' && sync_tag != 's') {
                    fprintf(stderr, "Please set --sync=[ms].\n");
                    exit(1);
                }
                break;
            case 'l':
                if (optarg)
                    list_count = atoi(optarg);
                break;
            case 'd':
                debug = true;
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab2_list [--threads=#threads --iterations=#iterations --yield[idl] --debug]\n");
                exit(1);
        }
    }
    if (debug) {
        for (i = 0; i < 3; i++)
            debug_print(i);
    }
    if (debug && sync_tag == 'm') debug_print(5);

    //Prepare list
    //Start list
    sortedlist = (SortedList_t*)malloc(sizeof(SortedList_t));
    sortedlist->prev = sortedlist;
    sortedlist->next = sortedlist;
    sortedlist->key = NULL;

    //Start elements
    element_count = thread_count * iteration_count;
    element = (SortedListElement_t*)malloc(sizeof(SortedListElement_t) * element_count);
    srand((unsigned int) time(NULL));
    
    //Make elements
    for (i = 0; i < element_count; i++) {
        char* random = (char*)malloc(sizeof(char) * 2);
        random[0] = ((char)rand() % 26) + 'a';
        random[1] = '\0';
        element[i].key = random;
    }

    //Mark start
    struct timespec start;
    if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
        fprintf(stderr, "Unable to set start time.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(3);

    dothething();

    //Mark end
    struct timespec end;
    if (clock_gettime(CLOCK_MONOTONIC, &end) < 0) {
        fprintf(stderr, "Unable to set end time.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(4);

    //Get data for CSV
    //Get name of test
    char tag[16] = "list-";
    label(tag);
    //Get total number of operations performed
    long num_ops = thread_count * iteration_count * 3;
    //Get total run time (in nanoseconds)
    long long runtime = (end.tv_sec - start.tv_sec) * 1000000000L;
    runtime -= start.tv_nsec;
    runtime += end.tv_nsec;
    long long average_runtime = runtime / num_ops;

    //Print CSV
    printf("%s,%d,%d,%d,%ld,%lld,%lld\n", tag, thread_count, iteration_count, 1, num_ops, runtime, average_runtime);
    
    //Free dynamic memory
    for(i = 0; i < element_count; i++)
        free((void*)element[i].key);
    free(element);
    free(sortedlist);

    exit(0); //Successful exit
}