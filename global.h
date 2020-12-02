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


typedef struct clientStruct{
    int clientFD;
    unsigned char clientID[MAXBUFLEN];
    char pw[MAXBUFLEN];
    bool loggedIn;
    //currentSess will be the last joinsession's session
    unsigned char currentSess[MAXBUFLEN];
    unsigned char sessionID[MAX_ARR_SIZE][MAXBUFLEN];
    int nextAvailIndex;
    pthread_t tid;
    bool quit;
    struct clientStruct * next;
}User;

typedef struct session{
    char sessionID[MAXBUFLEN];
    //User * clientsInSession;
    int numClients;
    int clientsInSess[MAX_ARR_SIZE];    //store the acceptFD's
    int nextAvailIndex;
    struct session *next;
}Session;


//global client and session lists
User * clientList = NULL; //head
User * lastClient = NULL;

Session *sessionList = NULL;
Session *lastSession = NULL;

/*
./server 3030
./deliver
/login emily huang 192.168.0.33 3030
/list
/createsession haha
/list
*/

#endif