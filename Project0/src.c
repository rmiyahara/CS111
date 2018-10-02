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
        //TODO: add empty case
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
        //TODO: add empty case
    }
}

void sigfault_handler(int hold) {
    if (hold == SIGEGV) {
        fprintf(stderr, "Segmentation fault (core dumped)\n");
        exit(4);
    }
}

int main(int argc, char** argv) {
    struct option flags[] {
        {"input", 1, NULL, 'i'},
        {"output", 1, NULL, 'o'},
        {"segfault", 0, NULL, 's'},
        {"catch", 0, NULL, 'c'}
    };

    int curr_param = getopt_long(argc, argv, "i:o:sc", flags, NULL); //Holds the current parameter that is being analyzed
    while(curr_param != -1) {
        switch (curr_param) { //TODO: Add cases
            case 'i':
                break;
            case 'o':
                break;
            case 's':
                break;
            case 'c':
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab0 --input=filename --output=filename [--segfault --catch]\n");
                exit(1);
        }
        int curr_param = getopt_long(argc, argv, "i:o:sc", flags, NULL); //Moves current parameter to the next one
    }
}