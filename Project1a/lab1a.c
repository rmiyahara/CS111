/*
NAME: Ryan Miyahara
EMAIL: rmiyahara144@gmail.com
ID: 804585999
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>

void debug_print(int message) {
    switch (message) {
        case 0: //Debug flag
            fprintf(stderr, "Debug mode has been enabled.\n");
            break;
        case 1: //Shell flag
            fprintf(stderr, "Shell mode has been enabled.\n");
            break;
        case 2: //Successful end
            fprintf(stderr, "Successful end. Shutdown on EOF. Exit code: 0.\n");
            break;
        case 3: //STDIN was not a terminal
            fprintf(stderr, "STDIN was not a terminal.\n")
            break;
        default:
            fprintf(stderr, "You shouldn't get here!\n");
    }
    return;
}

//Handler functions
void terminalerror_handler(bool debug) { //Called if STDIN does not refer to a terminal
    fprintf(stderr, "STDIN does not refer to a terminal.\nError number: %d\nError message: %s\n", errno, strerror(errno));
    if (debug)
        debug_print(3);
    exit(1);
}

//Terminal functions

int main(int argc, char** argv) {

    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"shell", 0, NULL, 's'},
        {"debug", 0, NULL, 'd'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed
    bool shell = false; //Set to true if the --shell flag is used
    bool debug = false; //Set to true if the --debug flag is used
    
    //Argument parsing section
    while ((curr_param = getopt_long(argc, argv, "sd", flags, NULL)) != -1) {
        switch (curr_param) {
            case 's':
                shell = true;
                break;
            case 'd':
                debug = true;
                debug_print(0);
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab1a [--shell --degbug]\n");
                exit(1);
        }
    }
    if (debug && shell)
        debug_print(1);

    if (!isatty(STDIN_FILENO))
        terminalerror_handler(debug);



    if (debug)
        debug_print(2);
    exit(0);
}