#ifndef GLOBAL
#define GLOBAL

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
// /login Yi_Ling password1 127.0.0.1 5050
// /login Emily password2 127.0.0.1 5050
// /login Ian password3 127.0.0.1 5050
// /login Angus password4 123 127.0.0.1 5050
// /login Anne password5 123 127.0.0.1 5050
// /createsession JRE420
// /createsession ECE334 
// /createsession ECE361 
// /joinsession JRE420
// /joinsession ECE334 
// /joinsession ECE361 
// /leavesession ECE361 
// /leavesession ECE334 
// /leavesession ECE361 
// /invite Emily ECE361
// /list
// /quit
#define DATA_SIZE 1000
#define MAX_FILEDATA_SIZE 400
#define MAX_PACKET_SIZE 4096
#define MAXBUFLEN 1000
#define MAX_NUM_SESSIONS 100
#define MAX_DATA 200
#define MAX_ARR_SIZE 1000

bool firstClient = true;

/*
./server 3030
./deliver
/login emily huang 192.168.0.33 3030
/list
/createsession haha
/list
*/

#endif