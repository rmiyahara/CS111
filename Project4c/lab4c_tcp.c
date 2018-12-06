/*
NAME: Ryan Miyahara
EMAIL: rmiyahara144@gmail.com
ID: 804585999
*/
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <poll.h>
#include "fcntl.h"
#include <math.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

//Global Variables
int period = 1; //Determines how often reports are made
char degrees = 'F'; //Determines which units are used for temperature
bool debug = false; //If set to true, debug mode is enabled
char* log_filename = NULL; //Holds the filename of the logfile
int log_fd; //Hold the file descriptor of the logfile
bool reports = true; //Does not make reports when set to 
int id = -1; //Set to ID to help find reports
int socket_fd = -1; //File descriptor for the socket which this program pulls from
struct sockaddr_in server_addy; //Initialized using socket connection
int port = -1; //Holds the given port number for TCP connection
char* hostname = NULL; //Holds the host of the connected server
struct hostent *host; //Initialized using hostname
mraa_aio_context temp; //Refers to the temperature sensor

void debug_print(int mes) {
    switch(mes) {
        case 0: //Initialize
            printf("Debug mode activated!\nPeriod set to: %d\nDegrees will be printed in: %c\n", period, degrees);
            break;
        case 1: //Log mode
            printf("Log mode activated!\nLog will be printed in: %s\n", log_filename);
            break;
        case 2: //When start_sensors() is called
            printf("Initializing the sensors.\n");
            break;
        case 3: //When rip_sensors() is called
            printf("Goodbye sensors.\n");
            break;
        case 4: //Help with segfault
            printf("Working on command.\n");
            break;
        case 5: //Check id, hostname, and port
            printf("ID: %d, Hostname: %s, Port: %d\n", id, hostname, port);
            break;
        case 6: //Socket setup
            printf("Socket has been created.\n");
            break;
        case 7: //Server setup
            printf("Server has been setup.\n");
            break;
        case 8: //Port set and connected
            printf("Port set and connected.\n");
            break;
        default:
            printf("You shouldn't get here!\n");
    }
    return;
}

void start_socket() { //Creates and connects socket
    //Set up socket_fd
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        fprintf(stderr, "Unable to open socket.\n");
        exit(2);
    }
    if (debug) debug_print(6);
    //Set up server
    host = gethostbyname(hostname);
    if (!host) {
        fprintf(stderr, "Unable to setup host.\n");
        exit(2);
    }
    memset((char*)&server_addy, 0, sizeof(server_addy));
    server_addy.sin_family = AF_INET;
    bcopy((char*)host->h_addr, (char*)&server_addy.sin_addr, host->h_length);
    if (debug) debug_print(7);
    //Set port and connected
    server_addy.sin_port = htons(port);
    if (connect(socket_fd, (struct sockaddr*)&server_addy, sizeof(server_addy)) < 0) {
        fprintf(stderr, "Unable to connect socket.\n");
        exit(2);
    }
    if (debug) debug_print(8);
    return;
}

void record_id() { //Marks ID to records for grading
    dprintf(log_fd, "ID=%d\n", id); //Write to log
    dprintf(socket_fd, "ID=%d\n", id); //Write to socket
    return;
}

float format_temperature(int reading) { //Formats raw temperature from sensor to F or C
    //Taken from Temperature Sensor Documentation
    const int B = 4275;               // B value of the thermistor
    const int R0 = 100000;            // R0 = 100k
    float R = 1023.0/reading-1.0;
    R = R0 * R;
    float celcius = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
    
    if (degrees == 'F')
        return (9.0 * celcius / 5.0) + 32;
    else
        return celcius; 
}

void print_output(float* addon) { //Uses localtime to print formatted time
    time_t timer;
    time(&timer);
    struct tm* time = localtime(&timer);
    if (addon) {
        if (reports)
            dprintf(socket_fd, "%.2d:%.2d:%.2d %.1f\n", time->tm_hour, time->tm_min, time->tm_sec, *addon);
        if (log_filename)
            dprintf(log_fd, "%.2d:%.2d:%.2d %.1f\n", time->tm_hour, time->tm_min, time->tm_sec, *addon);
    }
    else {
        if (reports)
            dprintf(socket_fd, "%.2d:%.2d:%.2d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);
        if (log_filename)
            dprintf(log_fd, "%.2d:%.2d:%.2d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);
        exit(0); //OFF command was given
    }
    return;
}

//Command Functions
void scale_command(char change) {
    switch (change) {
        case 'c':
        case 'C':
            degrees = change;
            break;
        case 'f':
        case 'F':
            degrees = change;
            break;
        default:
            return;
    }
    if (log_filename)
        dprintf(log_fd, "SCALE=%c\n", change);
    return;
}

void period_command(int change) {
    if (change > 0)
        period = change;
    if (log_filename)
        dprintf(log_fd, "PERIOD=%d\n", change);
    return;
}

void stop_command() {
    reports = false;
    if (log_filename)
        dprintf(log_fd, "STOP\n");
    return;
}

void start_command() {
    reports = true;
    if (log_filename)
        dprintf(log_fd, "START\n");
    return;
}

void log_command(char* log_me) {
    if (log_filename)
        dprintf(log_fd, "%s\n", log_me);
    return;
}

void off_command() {
    if (log_filename)
        dprintf(log_fd, "OFF\n");
    print_output(NULL);
    return;
}

void command_scheduler(char* command) {
    if (!(strcmp(command, "SCALE=F")))
        scale_command('F');
    else if (!(strcmp(command, "SCALE=C")))
        scale_command('C');
    else if (!(strncmp(command, "PERIOD=", 7)))
        period_command(atoi(command + 7));
    else if (!(strcmp(command, "STOP")))
        stop_command();
    else if (!(strcmp(command, "START")))
        start_command();
    else if (!(strncmp(command, "LOG", 3)))
        log_command(command);
    else if (!(strcmp(command, "OFF")))
        off_command();
    else {
        if (log_filename)
            dprintf(log_fd, "%s\n", command);
    }
    return;
}

//Sensor Functions
void rip_sensors() { //Closes log_fd and sensors
    if (debug) debug_print(3);
    if (log_filename)
        close(log_fd);
    mraa_aio_close(temp);
    return;
}

void start_sensors() { //Initializes temperature sensor
    //Open temperature sensor
    temp = mraa_aio_init(1);
    if (!temp) {
        fprintf(stderr, "Unable to open temperature sensor.\nError message: %s\nError number: %d\n", strerror(errno), errno);
        exit(1);
    }
    atexit(rip_sensors);
    if (debug) debug_print(2);
    return;
}

void poll_sensors() { //Polls sensors for data and STD_IN for commands
    struct pollfd poll_hold;
    poll_hold.fd = socket_fd;
    poll_hold.events = POLLIN | POLLERR | POLLHUP;
    time_t start;
    time(&start);
    time_t end;
    while(true) {
        time(&end);
        if (difftime(end, start) >= period) {
            float fixed = format_temperature(mraa_aio_read(temp));
            if (reports)
                print_output(&fixed);
            time(&start);
        }
        else {
            if (poll(&poll_hold, 1, 0) < 0) {
                fprintf(stderr, "Unable to poll.\nError message: %s\nError number: %d\n", strerror(errno), errno);
                exit(1);
            }
            if (poll_hold.revents && POLLIN) { //Perform commands
                char* buffer = (char*)malloc(sizeof(char) * 256);
                memset(buffer, 0, 256);
                char* hold = (char*)malloc(sizeof(char) * 256);
                memset(buffer, 0, 256);
                int n = read(socket_fd, buffer, 256);
                int i;
                int set = 0;
                if (n < 0 || n > 256) {
                    fprintf(stderr, "Unable to read.\nError message: %s\nError number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                for (i = 0; i < n; i++) {
                    if (buffer[i] == '\n') { //Process the command in hold and prepare for the next one
                        hold[set] = '\0';
                        set = 0;
                        if (debug) debug_print(4);
                        command_scheduler(hold);
                        memset(hold, 0, 256);
                    }
                    else {
                        hold[set] = buffer[i];
                        set++;
                    }
                }
                free(buffer);
                free(hold);
            }
        }
    }
    return;
}

int main(int argc, char** argv) {
    //Variable setup section
    struct option flags [] = { //Sets up the 2 optional flags
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
        {"id", required_argument, NULL, 'i'},
        {"host", required_argument, NULL, 'h'},
        //TODO: port number
        {"debug", no_argument, NULL, 'd'}
    }; //Option data structure referenced here: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    int curr_param; //Contains the parameter that is currently being analyzed

    //Set params
    while ((curr_param = getopt_long(argc, argv, "p:s:l:d", flags, NULL)) != -1) {
        switch (curr_param) {
            case 'p':
                period = atoi(optarg);
                if (period <= 0) {
                    fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab4b [--period=seconds --scale=[CF] --log=filename --debug]\n");
                    exit(1);
                }
                break;
            case 's':
                degrees = optarg[0];
                if (degrees == 'c')
                    degrees = 'C';
                else if (degrees == 'f')
                    degrees = 'F';
                if (degrees != 'C' && degrees != 'F') {
                    fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab4b [--period=seconds --scale=[CF] --log=filename --debug]\n");
                    exit(1);
                }
                break;
            case 'l':
                log_filename = optarg;
                log_fd = creat(optarg, S_IRUSR | S_IWUSR);
                if (log_fd < 0) {
                    fprintf(stderr, "Unable to open log_fd.\nError message: %s\nError number: %d\n", strerror(errno), errno);
                    exit(1);
                }
                break;
            case 'i':
                id = atoi(optarg);
                break;
            case 'h':
                hostname = optarg;
                break;
            case 'd':
                debug = true;
                break;
            default:
                fprintf(stderr, "Incorrect usage, please use this program in the following format: ./lab4b [--period=# --scale=[CF] --log=filename --debug]\n");
                exit(1);
        }
    }
    if (!log_filename) {
        fprintf(stderr, "Incorrect usage, please use the mandatory --log=FILENAME flag.\n");
        exit(1);
    }
    if (id < 0) {
        fprintf(stderr, "Incorrect usage, please use the mandatory --id=IDNUMBER flag.\n");
        exit(1);
    }
    if (!hostname) {
        fprintf(stderr, "Incorrect usage, please use the mandatory --host=HOSTNAME flag.\n");
        exit(1);
    }
    if (argv[optind])
        port = atoi(argv[optind]);
    if (port < 0) {
        fprintf(stderr, "Incorrect usage, please specify a port (doesn't use --port).\n");
        exit(1);
    }
    if (debug) debug_print(0);
    if (debug && log_filename) debug_print(1);
    if (debug) debug_print(5);

    start_sensors();
    start_socket();
    record_id();
    float fixed = format_temperature(mraa_aio_read(temp));
    print_output(&fixed);
    poll_sensors();
    
    exit(0); //Successful exit
}