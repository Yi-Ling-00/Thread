#ifndef CLIENTLIST
#define CLIENTLIST

#include "global.h"



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


void printIntArrar(int arr[], int size){
    for(int i = 0; i < size; i++){
        printf("arr[i]: %d\n", arr[i]);
    }
    return;
}

int returnInviteFD(char *clientID, char * whyFailed){    
    //printf("\n\nThis is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

    printClientList(clientList);
    int inviteFD;
    //printf("Invite ID: %s\n", clientID);

    User * temp = clientList;
    if(temp == NULL){
        char failedReason[MAXBUFLEN]="There's no one logged in!\n";
        strcpy(whyFailed,failedReason);
        return -1;
    }
    while(temp != NULL){
        //printf("In while in checkClientID\n");
        //printf("Checking : %s\n",temp -> clientID);
        if(strcmp((char *)temp -> clientID,clientID)==0){
            return temp -> clientFD;
        }
        ///if(temp -> next != NULL)temp = temp -> next;
        temp = temp -> next;
    }
    char failedReason[MAXBUFLEN]="Can't find that invitee!\n";
    strcpy(whyFailed,failedReason);
    return -1;
    
}

void printSessionStruct(Session *currentSession){
    //printf("sessionID: %s\n", currentSession -> sessionID);
    //printf("users in this section: \n");
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
    //printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

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
    if( checkClientID(currentClient -> clientID)== true){
        return false;
    }

    currentClient -> loggedIn = true;
    //else add to list and return true
    //printf("current User: %s\n", currentClient->clientID);
    
    if(clientList==NULL){
        //printf("First client in the list!\n");
        clientList = currentClient;
        lastClient = currentClient;
        currentClient -> next = NULL;
    }else{
        //printf("Adding to end of list\n");
        lastClient -> next = currentClient;
        lastClient = currentClient;
        currentClient -> next = NULL;
    }
    return true;
}

bool removeFromClientList(User * cliToRemove){
    //printf("Removing from client list!\n");

    //can't logout if you haven't logged in
    if(cliToRemove -> loggedIn == false){
        return false;
    }
    User * temp = clientList;//save og head 
    User * prev = NULL;
    //deleting from head
    if(clientList -> clientFD == cliToRemove -> clientFD){
        //remove head
        //delete head
        clientList = clientList -> next; //temp = second node
        free(temp);
        return true;   
    }

    while (temp != NULL && cliToRemove -> clientFD != temp -> clientFD) { 
        prev = temp; 
        temp = temp->next; 
    } 
    
    // If user was not present in user list
    if (temp == NULL) return false; 
    // Unlink the node from linked list 
    prev->next = temp->next; 
    // Free memory 
    free(temp); 
    return true;
}


bool sessionIsValid(char sessID[]){
    bool validSessID=false;
    Session * currSess = sessionList;
    while(currSess!= NULL){
        //printf("Checking if session is valid\n");
        //if not found, continue to check
        if(strcmp(currSess->sessionID,sessID)!=0)currSess = currSess -> next;
        else{
            //check if client already in this session. If so, return true with the appropriate message
            validSessID = true;
            return true;
        }
    }
    return false;
    // if(currSess == NULL){
    //     return false;
    // }
    // return true;
}


bool joinSession(char sessID[], User * client, char * reasonForFailure){ 
    //printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
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
    //for(int i = 0; i < client -> nextAvailIndex; i++){
    //printf("Checking for if it's already in this session\n");
       if(strcmp((char*)client -> currentSess ,sessID)==0){
            char whyFailed[] = "Can't join session: you are already in this session!\n";
            strcpy(reasonForFailure, whyFailed);
            return false;
        }
    //}


    //currentSession now points to the session the user wants to join
    //add to session's clients
    //currSess should be at the right session node
    //if not already present in the user's session list and not in session's user list, update
//else just change currentSess
    //bool updatedCurrSess = false;
    for(int i = 0; i < client -> nextAvailIndex; i++){
        //check if this sessID is already in the user's session list
        if(strcmp((char*)client -> sessionID[i],sessID)==0 && memcmp(client -> sessionID[i], client -> currentSess, MAXBUFLEN)!=0){
            memcpy(client -> currentSess, (unsigned char *)sessID, MAXBUFLEN);
            //updatedCurrSess = true;
            return true;
            //break;
        }
    }
    //Add to the session's list of clients
    currSess -> clientsInSess[currSess -> nextAvailIndex] = client -> clientFD;
    (currSess->nextAvailIndex)++;
    (currSess->numClients)++;
    //add to the client's list of sessions
    memcpy(client -> sessionID[client -> nextAvailIndex], (unsigned char *)sessID,MAXBUFLEN);
    (client -> nextAvailIndex)++;
    // printf("currSess -> nextAvailIndex: %d\n", currSess->nextAvailIndex);
    // printf("client -> nextAvailIndex: %d\n", client -> nextAvailIndex);
    memcpy(client -> currentSess, (unsigned char *)sessID, MAXBUFLEN);
    return true;
}


//If this is the first session made, join this session
//else, don't join the session
bool createSession(unsigned char sessID[], User *currentClient, char * reasonForFailure){
    //check if this session already exists in the session list
    Session *curr = sessionList;
    while(curr!=NULL){
        //printf("Checking for duplicate session\n");
        if(strcmp((char*)sessID, curr->sessionID)==0){
            char whyfailed[] = "This session already exists.\n";
            strcpy(reasonForFailure, whyfailed);
            //printf("About to return false;");
            return false;
        }
        curr = curr -> next;
    }
    //printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
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
        //printf("First session in the list\n");
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
    //printf("Num of clients in session: %d\n", temp->numClients);
    //update currentSession
    memcpy(currentClient -> currentSess, (unsigned char *)sessID, MAXBUFLEN);

    return true;
}

bool removeSession(Session * sessionToRemove){
   //printf("Deleting session.");
    Session * temp = sessionList;
    Session * prev = NULL;

    if(strcmp(sessionList -> sessionID,sessionToRemove->sessionID)==0){
        sessionList = sessionList -> next; 
        free(temp);
        return true;
    }

    while(temp != NULL && strcmp(sessionToRemove -> sessionID , temp -> sessionID)!=0){
        prev= temp;
        temp = temp -> next;
    }

    //this session isn't in session list
    if(temp == NULL)return false;

    // Unlink the node from linked list 
    prev->next = temp->next; 
  
    // Free memory 
    free(temp); 
    return true;

}

bool leaveSession(char sessID[],User * client , char * reasonForFailure ){
    //printf("In leave session function!\n");
    Session *curr= sessionList;
    unsigned char emptyElement[MAXBUFLEN] = {0x00};
    //find the session first
    while(curr != NULL){
        if( strcmp(curr->sessionID,sessID)==0) break;
        curr = curr -> next;
    }
    //printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

    if(curr == NULL){
        //session
        char whyFailed[] = "This session doesn't exist.\n";
        strcpy(reasonForFailure, whyFailed);
        return false;
    }

    //now curr points to the right session
    //printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);


    //remove that user from the session of sessionID
    int i = 0;
    if(curr->nextAvailIndex == 0){
        removeSession(curr);
    }else{
        for(; i < curr->nextAvailIndex; i++){
            if(curr->clientsInSess[i]==client -> clientFD){
                curr->clientsInSess[i] = -1;
                (curr->numClients)--;
                //printf("Num of clients in session: %d\n", curr->numClients);
                //if it has 0 clients, remove this session all together
                if(curr -> numClients == 0){
                    removeSession(curr);
                }else{
                    //printf("There's still : %d users\n", curr->numClients);
                }
                break;
            }
        }
    }
    
    //printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

    //remove that session from the user as well
    int j =0;
    for(; j < client -> nextAvailIndex; j++){
        if(strcmp(sessID, (char * )client -> sessionID[j]) == 0){
            //found this session under client's session array
            memcpy(client -> sessionID[j], emptyElement, MAXBUFLEN); //set it to be empty
            break;
        }
    }

    //printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    /*
    if(i == curr->nextAvailIndex|| j == client -> nextAvailIndex){
        //reached the end of the client or session array: couldn't find it
        char whyFailed[] = "You're not in this session.\n";
        strcpy(reasonForFailure, whyFailed);
        return false;
    }
    */

    //clear currSess in user if currently in that one
    //printf("sessID: '%s'     currentSess: '%s'\n", sessID, client->currentSess);
    if(memcmp((unsigned char *)sessID, client -> currentSess, strlen(sessID))==0){
        // printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
        // printf("You are currently in this session\n");
        //assign it to the last session joined that it hasn't left session
        int index = (client -> nextAvailIndex) - 1;
            //printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

        while(index >= 0){
                //printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

            if(memcmp(client -> sessionID[index],emptyElement,MAXBUFLEN)!=0){
                //printf("Reseting current session to: %s\n",client -> sessionID[index]);
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

bool removeAllUsersSessions(User * current, char * reasonForFailure){//only if they are the only user in these sessions
    unsigned char emptyElement[MAXBUFLEN] = {0x00};
    char * whyFailed = malloc(sizeof(char));
    for(int i = 0; i < current -> nextAvailIndex; i++){
        
        printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
        //printf("...checking this %s\n", current -> sessionID[i]);
        
        //find the sessions that the user is in the global linked list
        Session * currentSess = sessionList; 
        while(currentSess != NULL){
            if(memcmp((unsigned char *)currentSess -> sessionID, current -> sessionID[i], MAXBUFLEN) ==0){
                //found the session that the current user is in that wants to logout
                //if(currentSess -> numClients == 1){
                    //it's me and since I'll logout, delete this session
                    bool left =  leaveSession((char * )current -> sessionID[i], current, whyFailed);
                    if(left == false){
                        strcpy(reasonForFailure, whyFailed);
                        return false;
                    }
                //}
            }
            currentSess = currentSess -> next;
        }
        //if sessionID is not empty and only has me in it
        


        // if(memcpy(current -> sessionID[i],emptyElement, MAXBUFLEN) != 0){
        //     printf("..this session ID is ok: %s\n", current -> sessionID[i]);
        //     bool leftSession = leaveSession((char * )current -> sessionID[i], current, reasonForFailure);
        //     if(leftSession != true){
        //         printf("Can't leave this session cuz: %d\n", leftSession);
        //     }   
        // }
    }
    free(whyFailed);
    return true;
}



#endif