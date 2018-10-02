#include <stdio.h>
#include <errno.h>

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