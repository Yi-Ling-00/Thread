#ifndef CLIENTLIST
#define CLIENTLIST

#include "global.h"



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

Session *sessionList=NULL;
Session *lastSession=NULL;

void printIntArrar(int arr[], int size){
    for(int i = 0; i < size; i++){
        printf("arr[i]: %d\n", arr[i]);
    }
    return;
}

void printClientStruct(User *currentClient){
    printf("clientFD: %d\n", currentClient -> clientFD);
    printf("clientID: %s\n", currentClient -> clientID);
    printf("pw: %s\n", currentClient -> pw);   
    printf("logged in: %d\n", currentClient -> loggedIn);
    printf("next avail: %d\n", currentClient->nextAvailIndex);
    printf("current session: %s\n",currentClient->currentSess);
    for(int i = 0 ; i < currentClient->nextAvailIndex; i++){
        printf("session: %s\n", currentClient->sessionID[i]);
    }
    printf("\n");
    return;
}   
void printClientList(User * n){
    if(n == NULL){
        printf("No user(s) to list. Nothing is connected.\n");
        return;
    }
    while(n != NULL){
        printClientStruct(n);
        if(n -> next != NULL)n = n -> next;
        else break;
    }
    return;
}

void printSessionStruct(Session *currentSession){
    printf("sessionID: %s\n", currentSession -> sessionID);
    printf("users in this section: \n");
    printIntArrar(currentSession -> clientsInSess, currentSession -> nextAvailIndex);
    return;
}   

void printSessionList(Session * n){
    if (n==NULL){
        printf("No active session!\n");
        return;
    }
    while(n!=NULL){
        printSessionStruct(n);
        if(n -> next != NULL)n = n -> next;
        else break;
    }
}


void listCommand(){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

    //print everything in the client linked list
    printf("These are the clients connected: \n");
    printClientList(clientList);
    
    printf("\nThese are the sessions available: \n");
    printSessionList(sessionList);
    return;
}


//check if client list has someone with the same userID
bool checkClientID(unsigned char * clientID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

    User * temp = clientList;
    while(temp != NULL){
        printf("In while in checkClientID\n");
        if(memcmp(temp -> clientID,clientID, MAXBUFLEN)==0)return true;
        ///if(temp -> next != NULL)temp = temp -> next;
        temp = temp -> next;
    }
    return false;
}


bool addToClientList(User *currentClient){
    //if user has already logged in: return false
    if(currentClient -> loggedIn == true || checkClientID(currentClient -> clientID)== true){
        return false;
    }

    currentClient -> loggedIn = true;
    //else add to list and return true
    printf("current User: %s\n", currentClient->clientID);
    
    if(clientList==NULL){
        printf("First client in the list!\n");
        clientList = currentClient;
        lastClient = currentClient;
        currentClient -> next = NULL;
    }else{
        lastClient -> next = currentClient;
        lastClient = currentClient;
        currentClient -> next = NULL;
    }
    return true;
}

void removeFromClientList(unsigned char clientID[], int acceptFD){
    printf("Removing from client list!\n");
    struct clientStruct * current = clientList;
    struct clientStruct * prev = NULL;

    if (current != NULL && memcpy(current -> clientID,clientID,MAXBUFLEN)==0){
        clientList = current -> next;
        free(current);
        return;
    }

    while(current != NULL && memcpy(current -> clientID,clientID, MAXBUFLEN)!=0){
        prev = current;
        current = current -> next;
    }

    if(current == NULL) return;//never found
    prev -> next = current -> next;
    free(current);
    return;
}






bool joinSession(char sessID[], User * client, char * reasonForFailure){ 
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    //1.check if session exists in the complete session list
    Session * currSess = sessionList;
    bool validSessID=false;
    while(currSess!= NULL){
        //if not found, continue to check
        if(strcmp(currSess->sessionID,sessID)!=0)currSess = currSess -> next;
        else{
            //check if client already in this session. If so, return true with the appropriate message
            validSessID = true;
            break;
        }
    }
    
    //session doesn't exist || never found session
    if (validSessID==false || currSess == NULL){
        char whyFailed[] = "Can't join session: session doesn't exist!\n";
        strcpy(reasonForFailure, whyFailed);
        return false;
    }

    //not logged in
    if(client -> loggedIn==false){
        char whyFailed[] = "Can't join session: you are not logged in!\n";
        strcpy(reasonForFailure, whyFailed);
        return false;
    }
   
    //already in this session
    for(int i = 0; i < client -> nextAvailIndex; i++){
        printf("Checking for if it's already in this session\n");
        if(strcmp((char*)client -> sessionID[i] ,sessID)==0){
            char whyFailed[] = "Can't join session: you are already in this session!\n";
            strcpy(reasonForFailure, whyFailed);
            return false;
        }
    }


    //currentSession now points to the session the user wants to join
    //add to session's clients
    //currSess should be at the right session node

    //Add to the session's list of clients
    currSess -> clientsInSess[currSess -> nextAvailIndex] = client -> clientFD;
    (currSess->nextAvailIndex)++;
    (currSess->numClients)++;
    //add to the client's list of sessions
    memcpy(client -> sessionID[client -> nextAvailIndex], (unsigned char *)sessID,MAXBUFLEN);
    (client -> nextAvailIndex)++;
    printf("currSess -> nextAvailIndex: %d\n", currSess->nextAvailIndex);
    printf("client -> nextAvailIndex: %d\n", client -> nextAvailIndex);
    memcpy(client -> currentSess, (unsigned char *)sessID, MAXBUFLEN);
    return true;
}


//If this is the first session made, join this session
//else, don't join the session
bool createSession(unsigned char sessID[], User *currentClient, char * reasonForFailure){
    //check if this session already exists in the session list
    Session *curr = sessionList;
    while(curr!=NULL){
        printf("Checking for duplicate session\n");
        if(memcmp(sessID, curr->sessionID, MAXBUFLEN)==0){
            char whyfailed[] = "This session already exists.\n";
            strcpy(reasonForFailure, whyfailed);
            printf("About to return false;");
            return false;
        }
        curr = curr -> next;
    }
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    if(currentClient->loggedIn==false){
        char whyfailed[] = "You are not logged in. You can only create a session when you're logged in.\n";
        strcpy(reasonForFailure, whyfailed);
        return false; //User not logged in
    } 
    // || currentClient->inSess==true 
    bool validSessID =false;
    Session *temp= calloc(sizeof(Session), 1);
    memcpy((unsigned char * )temp -> sessionID,sessID, MAXBUFLEN);
    temp -> next = NULL;
    temp -> nextAvailIndex = 0;

    if (sessionList==NULL){
        printf("First session in the list\n");
        sessionList = temp;
        lastSession = temp;
    }else{
        lastSession -> next = temp;
        lastSession = temp;      
    }
    
    //also join this session 
    //update client's sessionID
    memcpy(currentClient -> sessionID[currentClient -> nextAvailIndex],sessID,MAXBUFLEN);
    (currentClient -> nextAvailIndex)++;

    //update session's client list
    temp -> clientsInSess[temp -> nextAvailIndex]=currentClient -> clientFD;
    (temp -> nextAvailIndex)++;
    temp -> numClients =1;
     printf("Num of clients in session: %d\n", temp->numClients);
    //update currentSession
    memcpy(currentClient -> currentSess, (unsigned char *)sessID, MAXBUFLEN);

    return true;
}



bool leaveSession(char sessID[],User * client , char * reasonForFailure ){
    printf("In leave session function!\n");
    Session *curr= sessionList;
    unsigned char emptyElement[MAXBUFLEN] = {0x00};
    //find the session first
    while(curr != NULL){
        if( strcmp(curr->sessionID,sessID)==0) break;
        curr = curr -> next;
    }
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

    if(curr == NULL){
        //session
        char whyFailed[] = "This session doesn't exist.\n";
        strcpy(reasonForFailure, whyFailed);
        return false;
    }

    //now curr points to the right session
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);


    //remove that user from the session of sessionID
    int i = 0;
    for(; i < curr->nextAvailIndex; i++){
        if(curr->clientsInSess[i]==client -> clientFD){
            curr->clientsInSess[i] = -1;
            (curr->numClients)--;
            printf("Num of clients in session: %d\n", curr->numClients);
            break;
        }
    }
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

    //remove that session from the user as well
    int j =0;
    for(; j < client -> nextAvailIndex; j++){
        if(strcmp(sessID, (char * )client -> sessionID[j]) == 0){
            //found this session under client's session array
            memcpy(client -> sessionID[j], emptyElement, MAXBUFLEN); //set it to be empty
            break;
        }
    }

    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    if(i == curr->nextAvailIndex|| j ==client -> nextAvailIndex){
        //reached the end of the client or session array: couldn't find it
        char whyFailed[] = "You're not in this session.\n";
        strcpy(reasonForFailure, whyFailed);
        return false;
    }

    //clear currSess in user if currently in that one
    printf("sessID: '%s'     currentSess: '%s'\n", sessID, client->currentSess);
    if(memcmp((unsigned char *)sessID, client -> currentSess, strlen(sessID))==0){
        printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
        printf("You are currently in this session\n");
        //assign it to the last session joined that it hasn't left session
        int index = (client -> nextAvailIndex) - 1;
            printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

        while(index >= 0){
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

            if(memcmp(client -> sessionID[index],emptyElement,MAXBUFLEN)!=0){
                printf("Reseting current session to: %s\n",client -> sessionID[index]);
                memcpy(client -> currentSess,client -> sessionID[index],MAXBUFLEN);
                break;
            }
            index--;
        }
        if(index < 0){
            memcpy(client -> currentSess,emptyElement,MAXBUFLEN);
        }
    }
    //remove session if in client's session list
    return true;
}


#endif