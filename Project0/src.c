#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>

void inputRedirection(char* input) {
    //Taken from fd_juggling.html
    int ifd = open(input, O_RDONLY);
    if (ifd >= 0) {
        close(0);
        dup(ifd);
        close(ifd);
    }
    else {
        fprintf(stderr, "An error with the input has occured. \nInput file: %s cannot be opened. \nError number: %d \n Error message: %s \n", input, errno, strerror(errno));
        //TODO: Clean this up
        exit(2);
    }
}

void outputRedirection(char* output) {
    //Taken from fd_juggling.html
    int ofd = creat(output, 0666);
    if (ofd >= 0) {
	    close(1);
	    dup(ofd);
	    close(ofd);
    }
    else {
        //TODO: Error message
        exit(3);
    }
}

void segfault_handler(int hold) {
    if (hold == SIGSEGV) {
        fprintf(stderr, "Segmentation fault has been caught\n");
        exit(4);
    }
}

int main(int argc, char** argv) {
    struct option flags[] = { //Sets up the 4 viable flags
        {"input", 1, NULL, 'i'},
        {"output", 1, NULL, 'o'},
        {"segfault", 0, NULL, 's'},
        {"catch", 0, NULL, 'c'}
    };

    char* input = NULL;
    char* output = NULL;
    char* death = NULL;

    int curr_param = getopt_long(argc, argv, "i:o:sc", flags, NULL); //Holds the current parameter that is being analyzed
    while(curr_param != -1) {
        switch (curr_param) { //TODO: Add cases
            case 'i':
                input = optarg; //optarg becomes input file when the input flag is the current parameter
                break;
            case 'o':
                output = optarg; //optarg becomes output file when the output flag is the current parameter
                break;
            case 's':
                *death = '6'; //By setting a null pointer equal to something, we get a segfault
                break;
            case 'c': //TODO: Doesn't work
                signal(SIGSEGV, segfault_handler);
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab0 [--segfault --catch] --input=filename --output=filename\n");
                exit(1);
        }
        int curr_param = getopt_long(argc, argv, "i:o:sc", flags, NULL); //Moves current parameter to the next one
    }
    if (input) //Set input file
        inputRedirection(input);
    if (output) //Set output file
        outputRedirection(output);
    exit(0); //We made it; successful run through
}