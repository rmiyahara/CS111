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
#include <signal>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>

//Global Variables
int port_num = -1; //Set to port number passed into execution
bool encrypt_flag = false; //Set to true if --encrypt flag is used
char* encrypt; //Set to encrypt file
bool debug = false; //Used to help debug the program
struct sockaddr_in addy_server; //Holds internet address of server referenced here: http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
struct sockaddr_in addy_client; //Holds internet address of client
pid_t child; //Holds the id for the child
int pipe0[2]; //Holds file descriptors of the pipe that goes from the terminal to the shell
int pipe1[2]; //Holds file descriptors of the pipe that goes from the shell to the terminal
int socket_fd[2]; //Holds file descriptor of socket[0] and when it updated after binding[1]

void debug_print(int message) {
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
        default:
            fprintf(stderr, "You shouldn't get here!\r\n");
    }
    return;
}

//Socket Functions
voide set_socket() {
    socket_fd[0] = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd[0] < 0) {
        fprintf(stderr, "Unable to open socket.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(2);
}

void bind_socket() {
    memset((char*) &addy_server, 0, sizeof(addy_server)); //Help with memset here: https://www.geeksforgeeks.org/memset-c-example/
    
    addy_server.sin_port = htons(port_num); //Set portnumber specified by command line arg
    addy_server.sin_family = AF_INET; //Set in socket tutorial
    addy_server.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd[0], (struct socketaddr*) &addy_server, sizeof(addy_server)) < 0) {
        fprintf(stderr, "Unable to connect socket.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }

    if (debug) debug_print(3);
    return;
}

void socket_setup() {
    set_socket();
    bind_socket();
    
    listen(socket_fd[0], 5); //Passively listens to socket referenced: http://man7.org/linux/man-pages/man2/listen.2.html
    
    socket_fd[1] = accept(socket_fd[0], (struct sockaddr*) &addy_client, sizeof(addy_client));
    if (socket_fd[1] < 0) {
        fprintf(stderr, "Unable to accept new socket.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    if (debug) debug_print(4);
}

int main(int argc, char** argv) {
    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"port", 1, NULL, 'p'},
        {"encrypt". 1, NULL, 'e'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed

    while ((curr_param = getopt_long(argc, argv, "p:l:e:d", flags, NULL)) != -1) {
        switch(curr_param) {
            case 'p':
                port_num = atoi(optarg);
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

    exit(0); //Successful finish
}