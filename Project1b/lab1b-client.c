/*
NAME: Ryan Miyahara
EMAIL: rmiyahara144@gmail.com
ID: 804585999
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <getopt.h>

//Global Variable section
bool log_flag = false; //Set to true if --log flag is used
int log; //Used to write to filename when log flag is used
bool encrypt_flag = false; //Set to true if --encrypt flag is used
int port_num; //Set to port number passed into execution

int main(int argc, char** argv) {

    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"port", 1, NULL, 'p'},
        {"log", 1, NULL, 'l'},
        {"encrypt", 1, NULL, 'e'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed
    
    //Argument parsing section
    while ((curr_param = getopt_long(argc, argv, "p:l:e:", flags, NULL)) != -1) {
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
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab1b-client --port=portnumeber [--log=filename --encrypt=keyfile]");
                exit(1);
        }
    }

    exit(0);
}