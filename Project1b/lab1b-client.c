/*
NAME: Ryan Miyahara
EMAIL: rmiyahara144@gmail.com
ID: 804585999
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <getopt.h>
#include <mcrypt.h>
#include <termios.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


//Global Variable section
bool log_flag = false; //Set to true if --log flag is used
int log; //Used to write to filename when log flag is used
bool encrypt_flag = false; //Set to true if --encrypt flag is used
int port_num; //Set to port number passed into execution
bool debug = false; //Used to help debug the program
struct termios holdme; //Will hold the terminal's mode
struct termios alteredterminal; //Holds terminal's mode after changes have been made
struct sockaddr_in addy_server; //Holds internet address of server referenced here: http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
struct hostent* server; //Holds multiple server attributes referenced here: http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
int pipe0[2]; //Holds file descriptors of the pipe that goes from the terminal to the shell
int pipe1[2]; //Holds file descriptors of the pipe that goes from the shell to the terminal
int socket_fd; //Holds file descriptor of socket
pid_t child; //Holds the id for the child

void debug_print(int message) {
    switch (message) {
        case 0:
            fprintf(stderr, "Debug flag has been raised!\r\n");
            break;
        case 1:
            fprintf(stderr, "Port flag has been raised!\r\n");
            break;
        case 2:
            fprintf(stderr, "Log flag has been raised!\r\n");
            break;
        case 3:
            fprintf(stderr, "Encrypt flag has been raised!\r\n");
            break;
        case 4:
            fprintf(stderr, "Input is being set up.\r\n");
            break;
        case 5:
            fprintf(stderr, "Terminal has been saved.\r\n");
            break;
        case 6:
            fprintf(stderr, "Terminal has been replaced.\r\n");
            break;
        case 7:
            fprintf(stderr,"Socket has been created.\r\n");
            break;
        case 8:
            fprintf(stderr, "Server has been got.\r\n");
            break;
        default:
            fprintf(stderr, "You shouldn't get here!\r\n");
    }
    return;
}

//Socket functions
void set_socket() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        fprintf(stderr, "Unable to open socket.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(7);
    return;
}

void set_server() {
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr, "Unable to get server.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(8);
    return;
}

void socket_setup() {
    set_socket();
    set_server();
    
    return;
}

//Terminal functions
void hold_terminal() { //Holds the terminal's states
    if (tcgetattr(STDIN_FILENO, &holdme)) {
        //Info for this function found here: http://pubs.opengroup.org/onlinepubs/007904875/functions/tcgetattr.html
        fprintf(stderr, "Unable to save terminal modes.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(5);
    return;
}

void replace_terminal() { //Puts back the terminal's states
    if (tcsetattr(STDIN_FILENO, TCSANOW, &holdme)) {
        //Info for this function found here: http://pubs.opengroup.org/onlinepubs/009695399/functions/tcsetattr.html
        fprintf(stderr, "Unable to replace terminal modes.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(6);
    return;
}

void input_setup() { //Called by main function to set terminal attributes
    if (debug) debug_print(4);

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
    return;
}

int main(int argc, char** argv) {

    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"port", 1, NULL, 'p'},
        {"log", 1, NULL, 'l'},
        {"encrypt", 1, NULL, 'e'},
        {"debug", 0, NULL, 'd'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed
    
    //Argument parsing section
    while ((curr_param = getopt_long(argc, argv, "p:l:e:d", flags, NULL)) != -1) {
        switch (curr_param) {
            case 'p':
                port_num = atoi(optarg);
                break;
            case 'l':
                log_flag = true;
                log = creat(optarg, S_IRWXU);
                if (log < 0) {
                    fprintf(stderr, "Unable to open file descriptor for log.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                filename = optarg;
                break;
            case 'e':
                encrypt_flag = true;
                char* encrypt = grab_key(optarg);
                break;
            case 'd':
                debug = true;
                if (debug) debug_print(0);
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab1b-client --port=portnumeber [--log=filename --encrypt=keyfile]");
                exit(1);
        }
    }
    if (debug & log_flag) debug_print(2);
    if (debug & encrypt_flag) debug_print(3);

    input_setup();
    socket_setup();

    exit(0);
}