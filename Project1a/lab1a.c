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
int pipe0[2]; //Holds file descriptors of the pipe that goes from the terminal to the shell
int pipe1[2]; //Holds file descriptors of the pipe that goes from the shell to the terminal
pid_t child; //Holds the id for the child
char* buffer;//Buffer for large reads as mentioned in P1A.html

void debug_print(int message) {
    switch (message) {
        case 0: //Debug flag
            fprintf(stderr, "Debug mode has been enabled.\n");
            break;
        case 1: //Shell flag
            fprintf(stderr, "Shell mode has been enabled.\n");
            break;
        case 2: //Successful end
            fprintf(stderr, "Successful end. Shutdown on EOF.\n");
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
        case 6: //Buffer is being freed
            fprintf(stderr, "Buffer is about to be freed");
            break;
        case 7: //Newline detected
            fprintf(stderr, "Newline detected.\n");
            break;
        case 8: //^C reached
            fprintf(stderr, "^C detected.\n");
            break;
        case 9: //^D reached
            fprintf(stderr, "^D detected.\n");
            break;
        case 10: //Char detected
            fprintf(stderr, "Char detected.\n");
            break;
        default:
            fprintf(stderr, "You shouldn't get here!\n");
    }
    return;
}

//Handler functions
void terminalerror_handler() { //Called if STDIN does not refer to a terminal
    fprintf(stderr, "STDIN does not refer to a terminal.");
    exit(1);
}

void pipeerror_handler() { //Called if there is an error closing pipes
    fprintf(stderr, "Issue closing pipes.\nError message: %s\n Error number: %d\n",strerror(errno), errno);
    if (debug)
        debug_print(6);
    exit(1);
}

//Terminal functions
void hold_terminal() { //Holds the terminal's states
    if (debug)
        debug_print(4);

    if (tcgetattr(STDIN_FILENO, &holdme)) {
        //Info for this function found here: http://pubs.opengroup.org/onlinepubs/007904875/functions/tcgetattr.html
        fprintf(stderr, "Unable to save terminal modes.\nError message: %s\n Error number: %d\n",strerror(errno), errno);
        exit(1);
    }
    return;
}

void replace_terminal() { //Puts back the terminal's states
    if (debug)
        debug_print(5);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &holdme)) {
        //Info for this function found here: http://pubs.opengroup.org/onlinepubs/009695399/functions/tcsetattr.html
        fprintf(stderr, "Unable to replace terminal modes.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    return;
}

void open_pipes() {
    if (!(pipe(pipe0))) {
        fprintf(stderr, "Unable to open pipe from terminal to shell.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (!(pipe(pipe1))) {
        fprintf(stderr, "Unable to open pipe from shell to terminal.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
}

void shell_process() { //Run if shell flag is raised
    buffer = (char*)malloc(256 * sizeof(char));
    
    //Free buffer
    if (debug)
        debug_print(6);
    free(buffer);
    return;
}


void write_terminal() { //Run if shell flag isnt raised
    char hold; //Not using buffer because keyboards will return 1
    ssize_t curr_char; //Will hold the current char input

    while ((curr_char = read(STDIN_FILENO, &hold, 1))) {
        if (curr_char < 0) {
            fprintf(stderr, "Unable to read from source.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
            exit(1);
        }

        switch (hold){
            case '\r':
            case '\n':
                if (debug)
                    debug_print(7);
                char carriage_return[2] = {'\r', '\n'}; //P1A.html specifies all newlines be saved as <cr><lf>
                if (write(STDOUT_FILENO, &carriage_return, 2) < 0) {
                    fprintf(stderr, "Unable to write to output.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                break;
            case 0x03: //^C
                if (debug)
                    debug_print(8);
                break;
            case 0x04: //^D
                if (debug)
                    debug_print(9);
                return;
            default:
                if (debug)
                    debug_print(10);
                if (write(STDOUT_FILENO, &hold, 1) < 0) {
                    fprintf(stderr, "Unable to write to output.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
        }
    }
    return;
}

void input_setup() { //Called by main function to set terminal attributes
    if (debug)
        debug_print(3);

    hold_terminal();
    atexit(replace_terminal);
    tcgetattr(STDIN_FILENO, &alteredterminal);

    //Following modifications taken from P1A.html
    alteredterminal.c_iflag = ISTRIP;	/* only lower 7 bits*/
	alteredterminal.c_oflag = 0;		/* no processing	*/
	alteredterminal.c_lflag = 0;		/* no processing	*/

    //Referenced here: http://man7.org/linux/man-pages/man3/atexit.3.html
    if (tcsetattr(STDIN_FILENO, TCSANOW, &alteredterminal)) { //TCSANOW suggested by P1A.html
        fprintf(stderr, "Unable to set new terminal modes.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }

}

int main(int argc, char** argv) {

    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"shell", 0, NULL, 's'},
        {"debug", 0, NULL, 'd'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed
    
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

    //Check input and set up
    if (!isatty(STDIN_FILENO))
        terminalerror_handler();
    else
        input_setup();

    //Check shell flag and execute accordingly
    if (shell) {
        open_pipes();
        shell_process();
    }
    else
        write_terminal();

    //Successful finish
    if (debug)
        debug_print(2);
    exit(0);
}