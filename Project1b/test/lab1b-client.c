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
char buffer[1024]; //Holds reads


void getErrors(char* error, int error_val, int error_number){
    if(strcmp(error, "write") ==0  && error_val < 0){
        fprintf(stderr, "Write failed. Error: %d, Message: %s\n", error_number, strerror(error_number));
        exit(1);
    }
    if(strcmp(error, "read") == 0 && error_val < 0){
        fprintf(stderr, "Error reading from input. Error: %d, Message: %s\n", error_number, strerror(error_number));
        exit(1);
    }
    if(strcmp(error, "log") == 0  && error_val < 0){
        fprintf(stderr, "failed to write to log. Error: %d, Message: %s\n", error_number, strerror(error_number));
        exit(1);
    }
    if(strcmp(error, "poll")  == 0 && error_val < 0){
        fprintf(stderr, "Error in creating poll: Error: %d, Message: %s\n", error_number, strerror(error_number));
        exit(1);
    }
    if(strcmp(error, "dprintf") == 0 && error_val < 0){
        fprintf(stderr, "Error with dprintf, unable to log to logFile: Error: %d, Message: %s\n", error_number, strerror(error_number));
        exit(1);
    }
}

void read_write(){ //from keyboard to socket
    poll_helper[0].fd = STDIN_FILENO;
    poll_helper[0].events = POLLIN | POLLHUP | POLLERR;
    poll_helper[1].fd = socket_fd;
    poll_helper[1].events = POLLIN | POLLHUP | POLLERR;
    int ret, res;
    int num;
    int i;
    int logWrite;
    for(;;){
        ret = poll(poll_helper, 2, 0);
        getErrors("poll", ret, errno);
        if(poll_helper[0].revents & POLLIN){
            num = read(poll_helper[0].fd, &buffer, 1024);
            getErrors("read", num, errno);
            for(i = 0; i < num; i++){
                char curr = buffer[i];
                switch(curr){
                    case '\r':
                    case '\n':
                        res = write(STDOUT_FILENO, &carriage_return[0], sizeof(char));
                        getErrors("write", res, errno);
                        res = write(STDOUT_FILENO, &carriage_return[1], sizeof(char));
                        getErrors("write", res, errno);
                        res = write(socket_fd, &carriage_return[1], sizeof(char));
                        getErrors("write", res, errno);
                        break;
                    default:
                        res = write(STDOUT_FILENO, &curr, sizeof(char));
                        getErrors("write", res, errno);
                        res = write(socket_fd, &curr, sizeof(char));
                        getErrors("write", res, errno);
                        break;
                }
            }
            if(log_flag){
                logWrite = dprintf(log_fd, "SENT %d bytes: ", num);
                getErrors("dprintf", logWrite, errno);
                dprintf(log_fd, &carriage_return[1], sizeof(char));
                getErrors("dprintf", logWrite, errno);
            }
        }
        if(poll_helper[0].fd & (POLLHUP | POLLERR)){
            fprintf(stderr, "Server shut down!\n");
            exit(1);
        }
        if(poll_helper[1].revents & POLLIN){
            num = read(poll_helper[1].fd, &buffer, 1024);
            getErrors("num", num, errno);
            if(num == 0){
                exit(1);
            }
            if(log_flag){
                logWrite = dprintf(log_fd, "RECEIVED %d bytes: ", num);
                getErrors("dprintf", logWrite, errno);
                logWrite = write(log_fd, &buffer, num);
                getErrors("log", logWrite, errno);
                logWrite = write(log_fd, &carriage_return[1], sizeof(char));
                getErrors("log", logWrite, errno);
            }
            for(i = 0; i < num; i++){
                char curr = buffer[i];
                switch(curr){
                    case '\r':
                    case '\n':
                        res = write(STDOUT_FILENO, &carriage_return[0], sizeof(char));
                        getErrors("write", res, errno);
                        res = write(STDOUT_FILENO, &carriage_return[1], sizeof(char));
                        getErrors("write", res, errno);
                        break;
                    default:
                        res = write(STDOUT_FILENO, &curr, sizeof(char));
                        getErrors("write", res, errno);
                        break;
                }
            }
        }
        if(poll_helper[1].revents & (POLLHUP | POLLERR)){
            fprintf(stderr, "Server shut down!\n");
            exit(1);
        }
    }
}


void save_terminal_attributes() {
    int result = tcgetattr(STDIN_FILENO, &holdme);
    if(result < 0){
        fprintf(stderr, "Error in getting attributes. Error: %d, Message: %s\n", errno, strerror(errno));
        exit(1);
    }
}
//help with termios from https://support.sas.com/documentation/onlinedoc/sasc/doc/lr2/tgetatr.htm

void reset(){
    if(tcsetattr(STDIN_FILENO, TCSANOW, &holdme) < 0) {
        fprintf(stderr, "Could not set the attributes\n");
        exit(1);
    }
}
//remember to map <cr> or <lf> into <cr><lf> in read and write function
void set_input_mode(){
    save_terminal_attributes();
    atexit(reset);
    tcgetattr(STDIN_FILENO, &alteredterminal);
    alteredterminal.c_iflag = ISTRIP;
    alteredterminal.c_oflag = 0;
    alteredterminal.c_lflag = 0;
    int res = tcsetattr(STDIN_FILENO, TCSANOW, &alteredterminal);
    if(res < 0){
        fprintf(stderr, "Error with setting attributes. Error: %d, Message: %s\n", errno, strerror(errno));
        exit(1);
    }
}

void createSocket_and_connect(){
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        fprintf(stderr, "Socket creation failed\n");
        exit(1);
    }
    server = gethostbyname("localhost");
    if(server == NULL){
        fprintf(stderr, "Error! No such host!\n");
        exit(0);
    }
    memset((char*) &addy_server, 0, sizeof(addy_server)); //instead of bzero use memset
    addy_server.sin_family = AF_INET;
    addy_server.sin_port = htons(port_num);
    memcpy((char*) &addy_server.sin_addr.s_addr, (char*)server->h_addr, server->h_length);
    if(connect(socket_fd, (struct sockaddr*)&addy_server, sizeof(addy_server)) < 0){
        fprintf(stderr, "Error in establishing connection\n");
        exit(1);
    }
}//help with socket from https://www.geeksforgeeks.org/socket-programming-cc/
//and http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/client.c

int main(int argc, char** argv){
    if(isatty(STDIN_FILENO) == 0){ //check if STDIN is a terminal
        fprintf(stderr, "STDIN is not an open file descriptor referring to a terminal. Error: %d, Message: %s\n", errno, strerror(errno));
        exit(1);
    }
    int portOpt = 0;
    int option;
    
    static struct option options [] = {
        {"port", 1, 0, 'p'},
        {"log", 1, 0, 'l'},
        {0, 0, 0, 0}
    };
    while((option = getopt_long(argc, argv, "p:lc", options, NULL)) != -1){
        switch(option){
            case 'p':
                portOpt = 1;
                port_num = atoi(optarg);
                break;
            case 'l':
                log_flag = 1;
                log_fd = creat(optarg, S_IRWXU);//RWX for owner
                if(log_fd < 0){
                    fprintf(stderr, "Unable to create log file. Error: %d, Message: %s\n", errno, strerror(errno));
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, "Incorrect argument: correct usage is ./lab1b-client --port=portno [--log=filename] [--compress] \n");
                exit(1);
        }
    }
    if(!portOpt){
        fprintf(stderr, "Please specify host in options\n");
    }
    set_input_mode();
    createSocket_and_connect();
    read_write();
    exit(0);
}