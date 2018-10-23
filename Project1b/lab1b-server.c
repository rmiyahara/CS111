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
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>

//Global Variables
int port_num = -1; //Set to port number passed into execution
bool encrypt_flag = false; //Set to true if --encrypt flag is used
char* encrypt; //Set to encrypt file
int encrypt_size; //How many bytes are in the encrypt file
bool debug = false; //Used to help debug the program
struct sockaddr_in addy_server; //Holds internet address of server referenced here: http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
struct sockaddr_in addy_client; //Holds internet address of client
pid_t child; //Holds the id for the child
int pipe0[2]; //Holds file descriptors of the pipe that goes from the terminal to the shell
int pipe1[2]; //Holds file descriptors of the pipe that goes from the shell to the terminal
int socket_fd[2]; //Holds file descriptor of socket[0] and when it updated after binding[1]
socklen_t socket_size; //Holds size of client address
char carriage_return[2] = {'\r', '\n'}; //P1A.html specifies all newlines be saved as <cr><lf>
char buffer[256]; //Holds reads
struct pollfd poll_helper[2]; //Holds events and revents for polling

void debug_print(int message) { //Used for debugging, message is used to pick which message
    switch (message) {
        case 0:
            fprintf(stderr, "Debug flag has been raised!\r\n");
            break;
        case 1:
            fprintf(stderr, "Encrypt flag has been raised with %s.\r\n", encrypt);
            break;
        case 2:
            fprintf(stderr,"Socket has been created.\r\n");
            break;
        case 3:
            fprintf(stderr, "Socket has been bound.\r\n");
            break;
        case 4:
            fprintf(stderr, "Socket setup is complete.\r\n");
            break;
        case 5:
            fprintf(stderr, "Inside parent process.\r\n");
            break;
        case 6:
            fprintf(stderr, "Inside child process.\r\n");
            break;
        case 7:
            fprintf(stderr, "Child process has changed the duped and closed pipes.\r\n");
            break;
        case 8:
            fprintf(stderr, "Shell has been executed by the child process.\r\n");
            break;
        case 9:
            fprintf(stderr, "Parent process has changed the pipes.\r\n");
            break;
        case 10:
            fprintf(stderr, "Pipes have been opened!\r\n");
            break;
        case 11:
            fprintf(stderr, "Polling has stopped.\r\n");
            break;
        case 12:
            fprintf(stderr, "^D received.\r\n");
            break;
        case 13:
            fprintf(stderr, "^C received.\r\n");
            break;
        case 14:
            fprintf(stderr, "Char received.\r\n");
            break;
        case 15:
            fprintf(stderr, "SIGINT recieved!\r\n");
            break;
        default:
            fprintf(stderr, "You shouldn't get here!\r\n");
    }
    return;
}

//Handler functions
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
    if (debug) debug_print(11);
    int i;
    for (i = 0; i < 2; i++) {
        if (close(socket_fd[i])) {
            fprintf(stderr, "Unable to close socket file descriptors.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
            exit(1);
        }
    }
    return;
}

void sigint_handler(int oibruv) {
    if (oibruv == SIGINT) {
        if (kill(child, SIGINT)) {
            fprintf(stderr, "Unable to kill child process.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
            exit(1);
        }
    }
    if (debug) debug_print(15);
    return;
}

void sigpipe_handler(int oibruv) {
    if (oibruv == SIGPIPE)
        exit(1);
    return;
}

void pipe_handler(int pipe_status) { //Called to check if an error has occured when duping or closing a pipe
    if (pipe_status < 0) { //Pipe_status will be set to -1 on error
        fprintf(stderr, "Unable to close pipes.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    return;
}

//Pipe functions
void open_pipes() { //Opens pipes to and from the shell
    if ((pipe(pipe0)) < 0) {
        fprintf(stderr, "Unable to open pipe from terminal to shell.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if ((pipe(pipe1)) < 0) {
        fprintf(stderr, "Unable to open pipe from shell to terminal.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(10);
}

void parent_duppipes() { //Sets up pipes for parent process
    pipe_handler(close(pipe0[0]));
    pipe_handler(close(pipe1[1]));
    if (debug) debug_print(9);
    return;
}

void child_duppipes() { //Sets up pipes for child process
    //Help with close function: https://linux.die.net/man/2/close
    //Help with dup2 function: http://man7.org/linux/man-pages/man2/dup.2.html
    pipe_handler(close(pipe0[1]));
    pipe_handler(close(pipe1[0]));
    pipe_handler(dup2(pipe0[0], STDIN_FILENO));
    pipe_handler(dup2(pipe1[1], STDOUT_FILENO));
    pipe_handler(dup2(pipe1[1], STDERR_FILENO));
    pipe_handler(close(pipe0[0]));
    pipe_handler(close(pipe1[1]));
    if (debug) debug_print(7);
    return;
}

//Socket Functions
void set_socket() {
    socket_fd[0] = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd[0] < 0) {
        fprintf(stderr, "Unable to open socket.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
}

void bind_socket() {
    memset((char*) &addy_server, 0, sizeof(addy_server)); //Help with memset here: https://www.geeksforgeeks.org/memset-c-example/
    
    addy_server.sin_port = htons(port_num); //Set portnumber specified by command line arg
    addy_server.sin_family = AF_INET; //Set in socket tutorial
    addy_server.sin_addr.s_addr = INADDR_ANY;
    if(bind(socket_fd[0], (struct sockaddr *)&addy_server, sizeof(addy_server)) < 0){
        fprintf(stderr, "Unable to connect socket.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(3);
    return;
}

void socket_setup(){
    set_socket();
    bind_socket();
    listen(socket_fd[0], 5);
    socket_size = sizeof(addy_client);

    socket_fd[1] = accept(socket_fd[0], (struct sockaddr *) &addy_client, &socket_size);
    if(socket_fd[1] < 0){
        fprintf(stderr, "Unable to accept new socket.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(4);
}

//Encrypt Functions
char* encrypt_key(char* keyname) {
    int encrypt_fd = open(keyname, O_RDONLY); //Open encrypt file descriptor
    if (encrypt_fd < 0) {
        fprintf(stderr, "Unable to open encryption key.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }

    struct stat encrypt_buf; //Defined here: https://linux.die.net/man/2/fstat
    if (fstat(encrypt_fd, &encrypt_buf) < 0) {
        fprintf(stderr, "Unable obtain information about encryption key.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    encrypt_size = encrypt_buf.st_size;

    char* the_key = (char*)malloc((encrypt_buf.st_size * sizeof(char)) + 1);
    if (read(encrypt_fd, the_key, encrypt_buf.st_size) < 0) {
        fprintf(stderr, "Unable to read encryption key.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    return the_key;
}

//Big functions
void through_pipe0() {
    char curr_char; //Holds current char we are processing
    ssize_t n = read(STDIN_FILENO, &buffer, 256); //How many bytes are read
    if (n < 0){
        fprintf(stderr, "Error reading fron terminal. Error: %d, Message: %s\n", errno, strerror(errno));
        exit(1);
        }
    int i; //Itterator for loop
    for(i = 0; i < n; i++) {
        curr_char = buffer[i];
        if (curr_char == 0x04) {
            pipe_handler(close(pipe0[1]));
            if (debug) debug_print(12);
        }
        else if (curr_char == 0x03) {
            if ((kill(child, SIGINT)) < 0){
                fprintf(stderr, "Unable to kill child process.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                exit(1);
            }
            if (debug) debug_print(13);
        }
        else if (curr_char == '\r' || curr_char == '\n') {
            if ((write(pipe0[1], &carriage_return[1], sizeof(char))) < 0) {
                fprintf(stderr, "Unable to write through pipe0 (terminal to shell).\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                exit(1);
            }
        }
        else {
            if ((write(pipe0[1], &curr_char, 1)) < 0) {
                fprintf(stderr, "Unable to write through pipe0.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                exit(1);
            }
            if (debug) debug_print(14);
        }
    }
    return;
}

void through_socket() {
    ssize_t n = read(poll_helper[1].fd, buffer, 256); //How many bytes are read
    if(n < 0){
        fprintf(stderr, "Error reading from input. Error: %d, Message: %s\n", errno, strerror(errno));
        exit(1);
    }
    if ((write(socket_fd[1], buffer, n)) < 0) {
        fprintf(stderr, "Unable to write through socket.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    return;
}

void parent_process() {
    parent_duppipes();
    //Set up poll_helper and a main loop as reccomended by P1A.html
    poll_helper[0].fd = 0;
    poll_helper[1].fd = pipe1[0];
    poll_helper[0].events = POLLIN | POLLERR | POLLHUP;
    poll_helper[1].events = POLLIN | POLLERR | POLLHUP;

    signal(SIGINT, sigint_handler);
    signal(SIGPIPE, sigpipe_handler);

    int poll_hold = poll(poll_helper, 2, 0);
    while(true) { //Endless loop, functions inside will end process
        if (poll_hold < 0) {
            fprintf(stderr, "Unable to poll.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
            exit(1);
        }
        else if (poll_hold > 0) {
            if (poll_helper[0].revents & (POLLERR | POLLHUP)) {
                pollerror_handler();
                return;
            }
            if (poll_helper[1].revents & (POLLERR | POLLHUP)) {
                pollerror_handler();
                return;
            }
            if (poll_helper[0].revents & POLLIN)
                through_pipe0();
            if (poll_helper[1].revents & POLLIN)
                through_socket();
        }
        poll_hold = poll(poll_helper, 2, 0);
    }
}

void child_process() {
    if (debug) debug_print(6);

    child_duppipes();
    char* shell_arguments[2] = {
        "/bin/bash",
        NULL
    }; //Arguments must be terminated by a NULL byte: https://linux.die.net/man/3/execvp
    if ((execvp("/bin/bash", shell_arguments)) < 0) {
        fprintf(stderr, "Unable to execute shell.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }

    if (debug) debug_print(8);
}

int main(int argc, char** argv){
    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"port", 1, NULL, 'p'},
        {"encrypt", 1, NULL, 'e'},
        {"debug", 0, NULL, 'd'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed
    
    while ((curr_param = getopt_long(argc, argv, "p:l:e:d", flags, NULL)) != -1) {
        switch(curr_param) {
            case 'p':
                port_num = atoi(optarg);
                break;
            case 'e':
                encrypt_flag = true;
                encrypt = encrypt_key(optarg);
                break;
            case 'd':
                debug = true;
                if (debug) debug_print(0);
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab1b-server --port=portnumber [--encrypt=keyfile]");
                exit(1);
            
        }
    }
    if (port_num < 0) {
        fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab1b-server --port=portnumber [--encrypt=keyfile]");
        exit(1);
    }
    if (debug & encrypt_flag) debug_print(1);
    
    socket_setup();
    open_pipes();

    child = fork();
    if (child < 0) {
        fprintf(stderr, "Unable to fork.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }

    if (child)
        parent_process();
    else
        child_process();

    exit(0);
}