/*
Name: Ryan Miyahara
Email: rmiyahara144@gmail.com
ID: 804585999
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
        fprintf(stderr, "An error with the input has occured. \nInput file: %s \nError number: %d \nError message: %s \n", input, errno, strerror(errno));
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
        fprintf(stderr, "An error with the output has occured. \nOutput file: %s \nError number: %d \nError message: %s \n", output, errno, strerror(errno));
        exit(3);
    }
}

void segfault_handler(int seg_flag) { //If a segfault happens after this has been called, user will recieve the error message
    if (seg_flag == SIGSEGV) {
        fprintf(stderr, "Segmentation fault has been caught\n");
        exit(4);
    }
}

int main(int argc, char** argv) {
    //Variable setup section
    struct option flags[] = { //Sets up the 4 viable flags
        {"input", 1, NULL, 'i'},
        {"output", 1, NULL, 'o'},
        {"segfault", 0, NULL, 's'},
        {"catch", 0, NULL, 'c'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    char* input = NULL; //Will point to input file
    char* output = NULL; //Will point to outputfile
    bool register_sig_handler = false; //Set to true if --catch is used
    bool blowup = false; //Set to true if --segfault is used
    int curr_param; //Holds the current parameter that is being analyzed
    //Explained here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html

    //Argument parsing section
    while((curr_param = getopt_long(argc, argv, "i:o:sc", flags, NULL)) != -1) { //Moved variable assignment into while loop to avoid a gcc warning
        switch (curr_param) {
            case 'i':
                input = optarg; //optarg becomes input file when the input flag is the current parameter
                break;
            case 'o':
                output = optarg; //optarg becomes output file when the output flag is the current parameter
                break;
            case 'c':
                register_sig_handler = true;
                break;
            case 's':
                blowup = true;
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab0 [--catch --segfault] --input inputfilename --output outputfilename\n");
                exit(1);
        }
    }

    //File redirection section (1)
    if (input)
        inputRedirection(input); //Input now redirected to user specified input
    if (output)
        outputRedirection(output); //Output now redirected to user specified output

    //Register the signal handler section (2)
    if (register_sig_handler)
        signal(SIGSEGV, segfault_handler);

    //Cause the segfault section (3)
    if(blowup) { //Initiates if the --segfault flag is added
        char* death = NULL;
        *death = '6'; //By setting a null pointer equal to something, we get a segfault
    }
    
    //Copt stdin to stdout section (4)
    char hold;
    ssize_t bytes_left;
    while((bytes_left = read(0, &hold, 1)) > 0) //While there are still bytes left to read in
        write(1, &hold, 1);

    exit(0); //We made it; successful run through
}