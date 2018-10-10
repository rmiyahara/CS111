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
#include <pthread.h>
#include <poll.h>

//Global Variables
bool debug = false; //Set to true if the --debug flag is used
bool shell = false; //Set to true if the --shell flag is used
struct termios holdme; //Will hold the terminal's mode
struct termios alteredterminal; //Holds terminal's mode after changes have been made
int pipe0[2]; //Holds file descriptors of the incoming shell
int pipe1[2]; //Holds file descriptors of the outgoing shell
pid_t child; //Holds the id for the child
char* buffer;//Buffer for large reads as mentioned in P1.html

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
        case 3:  //Input has begun setup
            fprintf(stderr, "Changing terminal modes.\n");
            break;
        case 4: //Terminal saved
            fprintf(stderr, "Terminal's mode has been saved. \n");
            break;
        case 5: //Terminal replaced
            fprintf(stderr, "Terminal's mode has been replaced. \n");
            break;
        case 6:
            fprintf(stderr, "")
        default:
            fprintf(stderr, "You shouldn't get here!\n");
    }
    return;
}

//Handler functions
void terminalerror_handler() { //Called if STDIN does not refer to a terminal
    fprintf(stderr, "STDIN does not refer to a terminal.\nError number: %d\nError message: %s\n", errno, strerror(errno));
    exit(1);
}

void pipeerror_handler() { //Called if there is an error closing pipes
    fprintf(stderr, "Pipe's unable to be closed.\nError number: %d\nError message: %s\n", errno, strerror(errno));
    if (debug)
        debug_print(6);
    exit(1);
}

void ctrlc_handler() { //Used to 

}

//Terminal functions
void hold_terminal() { //TODO: Holds the terminal's states
    if (debug)
        debug_print(4);

    if (tcgetattr(STDIN_FILENO, &holdme)) {
        //Info for this function found here: http://pubs.opengroup.org/onlinepubs/007904875/functions/tcgetattr.html
        fprintf(stderr, "Unable to save terminal's modes. \nError number: %d\nError message: %s\n", errno, strerror(errno));
        exit(1);
    }
    return;
}

void replace_terminal() { //Puts back the terminal's states
    if (debug)
        debug_print(5);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &holdme)) {
        //Info for this function found here: http://pubs.opengroup.org/onlinepubs/009695399/functions/tcsetattr.html
        fprintf(stderr, "Unable to save terminal's modes. \nError number: %d\nError message: %s\n", errno, strerror(errno));
        exit(1);
    }
    return;
}

void input_setup() { //Called by main function to set terminal attributes
    if (debug)
        debug_print(3);

    hold_terminal();
    tcgetattr(STDIN_FILENO, &alteredterminal);

    //Following modifications taken from P1.html
    alteredterminal.c_iflag = ISTRIP;	/* only lower 7 bits*/
	alteredterminal.c_oflag = 0;		/* no processing	*/
	alteredterminal.c_lflag = 0;		/* no processing	*/

    if (tcsetattr(STDIN_FILENO, TCSANOW, &alteredterminal)) {
        fprintf(stderr, "Unable to save modified terminal's modes. \nError number: %d\nError message: %s\n", errno, strerror(errno));
        exit(1);
    }

}

void close_pipes() { //Closes all pipes from pipe1 and pipe2
    int i;
    for (i = 0; i < 2; i++) {
        close(pipe0[i]);
        close(pipe1[i]);
    }
}

int main(int argc, char** argv) {

    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"shell", 0, NULL, 's'},
        {"debug", 0, NULL, 'd'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed
    buffer = (char*)malloc(256 * sizeof(char));
    
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
    


    free(buffer);
    if (debug)
        debug_print(2);
    exit(0);
}