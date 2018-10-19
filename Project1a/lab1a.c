/*
NAME: Ryan Miyahara
EMAIL: rmiyahara144@gmail.com
ID: 804585999
SLIPDAYS: 2
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
#include <sys/wait.h>
#include <poll.h>

//Global Variables
bool debug = false; //Set to true if the --debug flag is used
bool shell_flag = false; //Set to true if the --shell flag is used
char* shell = "/bin/bash"; //Set to specified shell, default is /bin/bash
struct termios holdme; //Will hold the terminal's mode
struct termios alteredterminal; //Holds terminal's mode after changes have been made
struct pollfd poll_helper[2]; //Holds events and revents for polling
int pipe0[2]; //Holds file descriptors of the pipe that goes from the terminal to the shell
int pipe1[2]; //Holds file descriptors of the pipe that goes from the shell to the terminal
pid_t child; //Holds the id for the child
char* buffer;//Buffer for large reads as mentioned in P1A.html
char carriage_return[2] = {'\r', '\n'}; //P1A.html specifies all newlines be saved as <cr><lf>

void debug_print(int message) {
    switch (message) {
        case 0: //Debug flag
            fprintf(stderr, "Debug mode has been enabled.\r\n");
            break;
        case 1: //Shell flag
            fprintf(stderr, "Shell mode has been enabled.\r\n");
            break;
        case 2: //Successful end
            fprintf(stderr, "Successful end. Shutdown on EOF.\r\n");
            break;
        case 3:  //Input has begun setup
            fprintf(stderr, "Changing terminal modes.\r\n");
            break;
        case 4: //Terminal saved
            fprintf(stderr, "Terminal's mode has been saved.\r\n");
            break;
        case 5: //Terminal replaced
            fprintf(stderr, "Terminal's mode has been replaced.\r\n");
            break;
        case 6: //Buffer is being freed
            fprintf(stderr, "Buffer is about to be freed.\r\n");
            break;
        case 7: //Newline detected
            fprintf(stderr, "Newline detected.\r\n");
            break;
        case 8: //^C reached
            fprintf(stderr, "^C detected.\r\n");
            break;
        case 9: //^D reached
            fprintf(stderr, "^D detected.\r\n");
            break;
        case 10: //Char detected
            fprintf(stderr, "Char detected.\r\n");
            break;
        case 11: //Pipes opened
            fprintf(stderr, "Both pipes have been opened.\r\n");
            break;
        case 12: //Parent process
            fprintf(stderr, "Parent process has begun.\r\n");
            break;
        case 13: //Child process
            fprintf(stderr, "Child process has begun.\r\n");
            break;
        case 14: //Pipes successfully changed by child process
            fprintf(stderr, "Child process has changed the duped and closed pipes.\r\n");
            break;
        case 15: //Shell has been executed
            fprintf(stderr, "Shell has been executed by the child process.\r\n");
            break;
        case 16: //Shell specified
            fprintf(stderr, "Shell has been specified in parameters.\r\n");
            break;
        case 17: //Shell about to be set
            fprintf(stderr, "%s will be the shell.\r\n", shell);
            break;
        case 18: //Polling beginning
            fprintf(stderr, "Polling has begun.\r\n");
            break;
        case 19: //Successful shell ending
            fprintf(stderr, "Successful shell ending.\r\n");
            break;
        case 20: //Pipes successfully changed by parent process
            fprintf(stderr, "Parent process has changed the duped and closed pipes.\r\n");
            break;
        default:
            fprintf(stderr, "You shouldn't get here!\r\n");
    }
    return;
}

//Handler functions
void terminalerror_handler() { //Called if STDIN does not refer to a terminal
    fprintf(stderr, "STDIN does not refer to a terminal.");
    exit(1);
}

void pipe_handler(int pipe_status) { //Called to check if an error has occured when duping or closing a pipe
    if (pipe_status < 0) { //Pipe_status will be set to -1 on error
        fprintf(stderr, "Unable to close pipes in child process.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
}

void pollerror_handler() { //Called if the parent process has an error polling
    int s;
    int thread = waitpid(child, &s, 0);
    if (thread < 0) {
        fprintf(stderr, "Unable to wait for child to update.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    int signal = (0x007F & s);
    int status = ((0xFF00 & s) >> 8);
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", signal, status);
    if (debug) debug_print(19);
    pipe_handler(close(pipe1[0])); //Close output pipe
}

//Terminal functions
void hold_terminal() { //Holds the terminal's states
    if (debug) debug_print(4);

    if (tcgetattr(STDIN_FILENO, &holdme)) {
        //Info for this function found here: http://pubs.opengroup.org/onlinepubs/007904875/functions/tcgetattr.html
        fprintf(stderr, "Unable to save terminal modes.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    return;
}

void replace_terminal() { //Puts back the terminal's states
    if (debug) debug_print(5);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &holdme)) {
        //Info for this function found here: http://pubs.opengroup.org/onlinepubs/009695399/functions/tcsetattr.html
        fprintf(stderr, "Unable to replace terminal modes.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    return;
}

void input_setup() { //Called by main function to set terminal attributes
    if (debug) debug_print(3);

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

//Pipe functions
void open_pipes() {
    if ((pipe(pipe0)) < 0) {
        fprintf(stderr, "Unable to open pipe from terminal to shell.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if ((pipe(pipe1)) < 0) {
        fprintf(stderr, "Unable to open pipe from shell to terminal.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(11);
}

void parent_pipes() {
    pipe_handler(close(pipe0[0]));
    pipe_handler(close(pipe1[1]));
    if (debug) debug_print(20);
}

void parenttochild_pipes() {
    if(pipe(pipe0) < 0) {
        fprintf(stderr, "Unable to open pipe from parent to child.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if(pipe(pipe1) < 0){
        fprintf(stderr, "Unable to open pipe from parent to child.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
}

void child_pipes() {
    //Help with close function: https://linux.die.net/man/2/close
    //Help with dup2 function: http://man7.org/linux/man-pages/man2/dup.2.html
    pipe_handler(close(pipe0[1]));
    pipe_handler(close(pipe1[0]));
    pipe_handler(dup2(pipe0[0], STDIN_FILENO));
    pipe_handler(dup2(pipe1[1], STDOUT_FILENO));
    pipe_handler(dup2(pipe1[1], STDERR_FILENO));
    pipe_handler(close(pipe0[0]));
    pipe_handler(close(pipe1[1]));
    if (debug) debug_print(14);
    return;
}

//Shell functions
void through_pipe0() {
    //Variable initiation
    char curr_char; //Holds current char we are processing
    ssize_t n = read(STDIN_FILENO, buffer, 256); //How many bytes are read
    if (n < 0){
        fprintf(stderr, "Error reading fron terminal. Error: %d, Message: %s\n", errno, strerror(errno));
        exit(1);
    }
    int i; //Itterator for loop
    for(i = 0; i < n; i++) {
        curr_char = buffer[i];
        switch(curr_char){
            case 0x04:
                pipe_handler(close(pipe0[1]));
                if (debug) debug_print(9);
                break;
            case 0x03:
                if ((kill(child, SIGINT)) < 0){
                    fprintf(stderr, "Unable to kill child process.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                if (debug) debug_print(8);
                break;
            case '\r':
            case '\n'://map <cr> & <lf> to <cr><lf>
                if ((write(STDOUT_FILENO, &carriage_return, 2)) < 0) {
                    fprintf(stderr, "Unable to write through pipe0.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                if ((write(pipe0[1], &carriage_return[1], 1)) < 0) {
                    fprintf(stderr, "Unable to write through pipe0 (terminal to shell).\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                break;
            default:
                if ((write(STDOUT_FILENO, &curr_char, 1)) < 0) {
                    fprintf(stderr, "Unable to write through pipe0.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                if ((write(pipe0[1], &curr_char, 1)) < 0) {
                    fprintf(stderr, "Unable to write through pipe0.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                if (debug) debug_print(10);
        }
    }
}

void through_pipe1() {
    char curr_char; //Holds current char we are processing
    ssize_t n = read(poll_helper[1].fd, buffer, 256); //How many bytes are read
    if(n < 0){
        fprintf(stderr, "Error reading from input. Error: %d, Message: %s\n", errno, strerror(errno));
        exit(1);
    }
    int i; //Itterator for loop
    for(i = 0; i < n; i++){
        curr_char = buffer[i];
        switch(curr_char){
            case '\r':
            case '\n':
                if((write(STDOUT_FILENO, &carriage_return, 2)) < 0) {
                    fprintf(stderr, "Error writing from input. Error: %d, Message: %s\n", errno, strerror(errno));
                    exit(1);
                }
                break;
            default:
                if((write(STDOUT_FILENO, &curr_char, 1)) < 0){
                    fprintf(stderr, "Unable to write through pipe1.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                if (debug) debug_print(10);
        }
    }
}

void shell_terminal() {
    buffer = (char*)malloc(256 * sizeof(char));

    parenttochild_pipes();
    child = fork();
    if (child < 0) {
        fprintf(stderr, "Unable to fork.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }

    if (child) { //Parent process
        if (debug) debug_print(12);
        parent_pipes();

        //Set up poll_helper and a main loop as reccomended by P1A.html
        poll_helper[0].fd = 0;
        poll_helper[1].fd = pipe1[0];
        poll_helper[0].events = POLLIN | POLLHUP | POLLERR;
        poll_helper[1].events = POLLIN | POLLHUP | POLLERR;
        
        int poll_hold = poll(poll_helper, 2, 0);
        while(1) {
            if (poll_hold < 0) {
                fprintf(stderr, "Unable to poll.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                exit(1);
            }
            else if (poll_hold > 0) {
                if(poll_helper[0].revents & (POLLHUP | POLLERR)) {
                    if((kill(child, SIGINT)) < 0){
                        fprintf(stderr, "Unable to kill the child process.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                        exit(1);
                    }
                    pollerror_handler();
                    return;
                }
                if(poll_helper[1].revents & (POLLHUP | POLLERR)) {
                    pollerror_handler();
                    return;
                }
                if(poll_helper[0].revents & POLLIN)
                    through_pipe0();
                if(poll_helper[1].revents & POLLIN)
                    through_pipe1();
            }
            poll_hold = poll(poll_helper, 2, 0);
        }
    }
    else { //Child procss has a PID of 0
        if (debug) debug_print(13);
        child_pipes();
        char* shell_arguments[2] = {
            shell,
            NULL
        };//Arguments must be terminated by a NULL byte: https://linux.die.net/man/3/execvp
        if (debug) debug_print(17);
        if ((execvp(shell, shell_arguments)) < 0) {
            fprintf(stderr, "Unable to execute shell.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
            exit(1);
        }
        if (debug) debug_print(15);
    }
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
                if (debug) debug_print(7);
                if (write(STDOUT_FILENO, &carriage_return, 2) < 0) {
                    fprintf(stderr, "Unable to write to output.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                break;
            case 0x03: //^C
                if (debug) debug_print(8);
                break;
            case 0x04: //^D
                if (debug) debug_print(9);
                return;
            default:
                if (debug) debug_print(10);
                if (write(STDOUT_FILENO, &hold, 1) < 0) {
                    fprintf(stderr, "Unable to write to output.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
        }
    }
    return;
}

int main(int argc, char** argv) {

    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"shell", 1, NULL, 's'},
        {"debug", 0, NULL, 'd'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed
    
    //Argument parsing section
    while ((curr_param = getopt_long(argc, argv, "s:d", flags, NULL)) != -1) {
        switch (curr_param) {
            case 's':
                shell_flag = true;
                if (strcmp(optarg, "--debug") == 0) {
                    fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab1a [--shell=program --degbug]\nTry setting program to \"\\bin\\bash\"\n");
                    exit(1);
                }
                break;
            case 'd':
                debug = true;
                debug_print(0);
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab1a [--shell=program --degbug]\nTry setting program to \"\\bin\\bash\"\n");
                exit(1);
        }
    }
    if (debug && shell_flag)
        debug_print(1);

    //Check input and set up
    if (!isatty(STDIN_FILENO))
        terminalerror_handler();
    else
        input_setup();

    //Check shell flag and execute accordingly
    if (shell_flag) {
        open_pipes();
        shell_terminal();
    }
    else
        write_terminal();

    //Successful finish
    if (debug) debug_print(2);
    exit(0);
}