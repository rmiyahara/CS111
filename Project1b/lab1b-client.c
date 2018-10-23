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
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>

//Global Variable section
bool log_flag = false; //Set to true if --log flag is used
int log_fd; //Used to write to filename when log flag is used
char* log_filename; //Holds filename of log
bool encrypt_flag = false; //Set to true if --encrypt flag is used
char* encrypt; //Set to encrypt file
int encrypt_size; //How many bytes are in the encrypt file
int port_num; //Set to port number passed into execution
bool debug = false; //Used to help debug the program
struct termios holdme; //Will hold the terminal's mode
struct termios alteredterminal; //Holds terminal's mode after changes have been made
struct sockaddr_in addy_server; //Holds internet address of server referenced here: http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
struct hostent* server; //Holds multiple server attributes referenced here: http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
int socket_fd; //Holds file descriptor of socket
char carriage_return[2] = {'\r', '\n'}; //P1A.html specifies all newlines be saved as <cr><lf>
struct pollfd poll_helper[2]; //Holds events and revents while polling
char buffer[256]; //Holds reads

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
        case 9:
            fprintf(stderr, "Socket has been connected.\r\n");
            break;
        case 10:
            fprintf(stderr, "Char received.\r\n");
            break;
        case 11:
            fprintf(stderr, "Port number: %d\r\n", port_num);
            break;
        default:
            fprintf(stderr, "You shouldn't get here!\r\n");
    }
    return;
}

//Handler functions
void server_handler() {
    fprintf(stderr, "The server has been disconnected! Try turning it off then back on again.\r\n");
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

void connect_socket() {
    memset((char*) &addy_server, 0, sizeof(addy_server)); //Help with memset here: https://www.geeksforgeeks.org/memset-c-example/
    
    addy_server.sin_port = htons(port_num); //Set portnumber specified by command line arg
    addy_server.sin_family = AF_INET; //Set in socket tutorial

    memcpy((char*) &addy_server.sin_addr, (char*)server->h_addr, server->h_length); //Move address of server into addy_server
    if (connect(socket_fd, (struct sockaddr*) &addy_server, sizeof(addy_server)) < 0) {
        fprintf(stderr, "Unable to connect socket.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }

    if (debug) debug_print(9);
    return;
}

void socket_setup() {
    set_socket();
    set_server();

    connect_socket();
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

//Encrypt functions
char* encrypt_key(char* keyname) {
    int encrypt_fd = open(keyname, O_RDONLY);
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
    int n = read(STDIN_FILENO, &buffer, 256); //How many bytes are read
    if (n < 0){
        fprintf(stderr, "Error reading fron terminal. Error: %d, Message: %s\n", errno, strerror(errno));
        exit(1);
        }
    int i; //Itterator for loop
    for(i = 0; i < n; i++) {
        if (buffer[i] == '\r' || buffer[i] == '\n') {
            if ((write(STDOUT_FILENO, &carriage_return, 2 * sizeof(char))) < 0) {
                fprintf(stderr, "Unable to write through pipe0 (terminal to shell).\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                exit(1);
            }
            if (write(socket_fd, &carriage_return[1], sizeof(char)) < 0) {
                fprintf(stderr, "Unable to write through pipe0 (terminal to shell).\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                exit(1);
            }
        }
        else {
            if ((write(STDOUT_FILENO, &buffer[i], sizeof(char))) < 0) {
                fprintf(stderr, "Unable to write through pipe0 (terminal to shell).\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                exit(1);
            }
            if (write(socket_fd, &buffer[i], sizeof(char)) < 0) {
                fprintf(stderr, "Unable to write through pipe0 (terminal to shell).\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                exit(1);
            }
        }
    }
    if (log_flag) {
        if (dprintf(log_fd, "%d byte read\r\n", n) < 0) {
            fprintf(stderr, "Unable to write to log.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
            exit(1);
        }
    }
    return;
}

void through_socket() {
    int n = read(poll_helper[1].fd, &buffer, 256);
    if (n < 0) {
        fprintf(stderr, "Unable to read from socket.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
        exit(1);
    }
    for (int i = 0; i < n; i++) {
            if (buffer[i] == '\r' || buffer[i] == '\n') {
                if (write(STDOUT_FILENO, &carriage_return, 2 * sizeof(char))) {
                    fprintf(stderr, "Unable to write through socket.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
            }
            else {
                if (write(STDOUT_FILENO, &buffer[i], sizeof(char))) {
                    fprintf(stderr, "Unable to write through socket.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
            }
    }
    if (log_flag) {
        if (dprintf(log_fd, "%d byte read\r\n", n) < 0) {
            fprintf(stderr, "Unable to write to log.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
            exit(1);
        }
        if (write(log_fd, &buffer, n) < 0) {
            fprintf(stderr, "Unable to write to log.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
            exit(1);
    }
    }
    return;
}

void shell_process() {
    //Set up poll_helper and a main loop as reccomended by P1A.html
        poll_helper[0].fd = 0;
        poll_helper[1].fd = socket_fd;
        poll_helper[0].events = POLLIN | POLLERR | POLLHUP;
        poll_helper[1].events = POLLIN | POLLERR | POLLHUP;

    int poll_hold = poll(poll_helper, 2, 0);
    while(true) { //Endless loop, functions inside will end process
        if (poll_hold < 0) {
            fprintf(stderr, "Unable to poll.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
            exit(1);
        }
        else if (poll_hold > 0) {
            if (poll_helper[0].revents & (POLLERR | POLLHUP)) {
                server_handler();
                return;
            }
            if (poll_helper[1].revents & (POLLERR | POLLHUP)) {
                server_handler();
                return;
            }
            if (poll_helper[0].revents & POLLIN)
                through_pipe0();
            if (poll_helper[1].revents & POLLIN)
                through_socket();
        }
        poll_hold = poll(poll_helper, 2, 0);
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
                log_fd = creat(optarg, S_IRWXU);
                if (log_fd < 0) {
                    fprintf(stderr, "Unable to open file descriptor for log.\nError message: %s\n Error number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                log_filename = optarg;
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
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab1b-client --port=portnumber [--log=filename --encrypt=keyfile]");
                exit(1);
        }
    }
    if (debug) debug_print(11);
    if (debug & log_flag) debug_print(2);
    if (debug & encrypt_flag) debug_print(3);

    input_setup();
    socket_setup();

    shell_process();

    exit(0);
}