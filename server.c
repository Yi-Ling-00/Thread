#include "clientSession.h"
#include "packet.h" 
  
#define BACKLOG 20 
bool message=false;
int numUserConnected=0;
int cancelThreadID;
pthread_mutex_t sessionList_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t userList_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t numUserConnected_mutex = PTHREAD_MUTEX_INITIALIZER;


struct message processPacket(struct message incomingPacket, User *current){
    //printf("\n\nEnterring process packet, client looks like:\n");
    //printClientStruct(current);
    bool removed;
    struct message packetToSend;
    //process the packet 
    //1. look at the packet type
    bool addSuccess=false;
    char * whyFailed = malloc(MAXBUFLEN* sizeof(char));
    bool joinSuccess=false;
    bool leaveSuccess = false;
    bool createSuccess=false;
    char sessionIDInvite[MAXBUFLEN], person[MAXBUFLEN];
    char * packetData = (char * )incomingPacket.data;
    char * token = NULL;
    int fdToSend;
    bool sessionValid;
    switch (incomingPacket.type){
        //follow the types according to the enumeration
        case 0: //login: store the clientID in the client list
            printf("Its a login!\n");
            //struct clientStruct * currentClient;
            //verify if client already logged in or already existing
            //bool addSuccess=false;
            //strcpy(current->clientID, (unsigned char *) incomingPacket.source);
            pthread_mutex_lock(&userList_mutex);
            addSuccess = addToClientList(current);
            pthread_mutex_unlock(&userList_mutex);
            //printf("^^^^^Add success : %d\n",addSuccess);

            if (addSuccess){
                printf("Login sucess!\n");
                current->quit=false;
                memcpy(current -> clientID,incomingPacket.source,MAXBUFLEN);
                //take out password from packet data
                char clientpw[MAXBUFLEN];
                char * packetData = (char * )incomingPacket.data;
                char * token;
                token = strtok(packetData, ",");
                /* walk through other tokens */
                while( token != NULL ) {
                    strcpy(clientpw, token);
                    token = strtok(NULL, ",");
                }
                //printf("packetdata: '%s'\n", packetData);
                strcpy(current -> pw,clientpw);
                //do LO_ACK
                packetToSend = makeLoAckPacket(current);
            }else{  
                printf("Login fail!\n");
                //do LO_NAK
                char reasonFailed[] = "This user is already logged in";
                packetToSend = makeLoNakPacket(current, reasonFailed);
            }
            message=false;
            break;

        case 3:
            printf("It's a quit!\n");
            int clientFDTemp= current->clientFD;
            char tempClientID[100];
            //printf("It's a logout!\n");
            
            strcpy(tempClientID,(char*)current->clientID);

            tempClientID[strlen((char*) current->clientID)]='\0';
               
            //printf("tempClientID: %s\n", tempClientID);
            pthread_mutex_lock(&userList_mutex);
            removed = removeFromClientList(current);
            pthread_mutex_unlock(&userList_mutex);
            pthread_mutex_lock(&numUserConnected_mutex);
            numUserConnected--;
            pthread_mutex_lock(&numUserConnected_mutex);
            message=false;
            packetToSend.type=-9;
            close(clientFDTemp);
            return packetToSend;
            break;
        case 18:
            printf("It's a logout!\n");
            //remove all sessions in this client
                //call leave session within
            if(current -> loggedIn == false){
                makeLogoutNakPacket(current -> clientID, "You're not even logged in");
            }
            char * whyFailed = malloc(sizeof(char));
            removeAllUsersSessions(current,whyFailed);
            //printf("Removed all sessions\n");
            removed = removeFromClientList(current);
            current -> loggedIn = false;
            if(removed){
                packetToSend = makeLogoutAckPacket(current->clientID);
            }else{
                packetToSend = makeLogoutNakPacket(current->clientID, whyFailed);
                free(whyFailed);
                whyFailed = NULL;
            }
            firstClient = true;
            message= false;
            break;
        case 4: //Joinsession
            printf("Joining session!\n");
            //joining
            // joinSuccess = joinSession((char *)incomingPacket.data, current->clientFD, joinReasonForFailure, current->loggedIn); 
            joinSuccess = joinSession((char *)incomingPacket.data, current,whyFailed);
            //printf("\n&&&&   IN SERVER: %s\n", whyFailed);
            if (joinSuccess) {
                printf("Sucessfuly joined session!\n");
                //create Jn_ack packet
                pthread_mutex_lock(&sessionList_mutex);
                packetToSend = makeJnAckPacket(current, incomingPacket.data);
                pthread_mutex_unlock(&sessionList_mutex);

            }else{
                printf("Can't join session !\n");
                 //make jn_nak packet
                packetToSend = makeJnNakPacket(current, incomingPacket.data,whyFailed);
                free(whyFailed);
                whyFailed = NULL;

            }
            message=false;
            break;
           
        case 7: //leavesession
            printf("Leaving session!\n");
            pthread_mutex_lock(&sessionList_mutex);
            leaveSuccess = leaveSession((char *)incomingPacket.data,current, whyFailed);
            pthread_mutex_unlock(&sessionList_mutex);

            if(leaveSuccess){
                //printf("Sucessfuly left session!\n");
                packetToSend = makeLeaveAckPacket(current, incomingPacket.data);
            }else{
                //printf("Can't leave session!\n");
                packetToSend = makeLeaveNakPacket(current, incomingPacket.data);
                free(whyFailed);
                whyFailed = NULL;
            }
            
            message=false;
            break;
        case 8: //create session 
            printf("creating session!\n");
            pthread_mutex_lock(&sessionList_mutex);
            createSuccess = createSession((unsigned char *)incomingPacket.data, current, whyFailed); 
            pthread_mutex_unlock(&sessionList_mutex);

            //printf("createsession: %d\n",createSuccess);
            if(createSuccess) {
                printf("Sucessfuly Created session!\n");
                //create Jn_ack packet
                packetToSend = makeNsAckPacket(current, (char *)incomingPacket.data);
            }else{
                printf("Unsucessful!\n");
                packetToSend = makeNsNakPacket(current, whyFailed);
                free(whyFailed);
                whyFailed = NULL;
            }
            
            message=false;
            break;

         case 10: //message 
            //unsigned char emptyUnsignedChar[MAXBUFLEN] = {};
            //memset(emptyUnsignedChar, 0, sizeof(emptyUnsignedChar));
            if(memcmp(current -> currentSess, "",MAXBUFLEN)!=0){
                message= true;
                packetToSend = makeMessagePacket((char *)current->clientID, (char *) incomingPacket.data);
                //find that session 
                Session *tempSess= sessionList;
                while(tempSess!=NULL){
                    struct message * ptrToPacketToSend;
                    if(strcmp((char *)current->currentSess, tempSess->sessionID)==0){
                        for(int i = 0; i < tempSess -> nextAvailIndex; i++){
                            ptrToPacketToSend = &packetToSend;
                            if(tempSess -> clientsInSess[i]!=-1){
                                int sendBytes = write(tempSess -> clientsInSess[i], ptrToPacketToSend, sizeof(struct message));
                            }
                        } 
                        break;
                    }else{
                        tempSess= tempSess->next;
                    }
                }       
            }else{
                printf("You're not in session");
            }
            
            break;
        case 11: //list
            printf("List!\n"); 
            packetToSend = makeQuAckPacket(current->clientID);
            message=false;
            break;
      case 19://Invite
            printf("Invite!\n");
            
            token = strtok(packetData, ",");
            strcpy(person, token );
            /* walk through other tokens */
            while( token != NULL ) {
                strcpy(sessionIDInvite, token);
                token = strtok(NULL, ",");
            }
            fdToSend = returnInviteFD(person,whyFailed);
            sessionValid = sessionIsValid(sessionIDInvite);
            //printf("sessionValid: %d\n", sessionValid);
            if(fdToSend == -1){
                //invalid: nak  
                packetToSend = makeInviteNakPacket(current, whyFailed);//send it to inviter
                free(whyFailed);
                whyFailed = NULL;
                //need to send this to the client
            }else if(sessionValid != 1){
                //strcpy(whyFailed, "This session doesn't exist.\n");
                packetToSend = makeInviteNakPacket(current, whyFailed);//send it to inviter
                free(whyFailed);
                whyFailed = NULL;
            }else{
                packetToSend = makeInviteAckPacket((char*)current->clientID, incomingPacket, sessionIDInvite, person);//want to sent it to invitee
                int inviteClientBytes = write(fdToSend, &packetToSend, sizeof(struct message));
            }
            message=false;
            break;
        default:  
            message=false;
            break;
    }

    if(whyFailed != NULL)free(whyFailed);
    //printf("Exiting process packet, client looks like:\n");
    printClientStruct(current); 
    return packetToSend;
}


void *handle(void *tempUser){
        // User *newUser = malloc(sizeof(User));
        // newUser = (User *) tempUser;
        User * newUser = (User *) tempUser;
        struct message * incomingPacket;
        struct message packetToSend;
        struct message * ptrToPacketToSend;
        int recvBytes;

            while(1){
                //printf("\n\n****************************\n");
                incomingPacket = (struct message * ) malloc (sizeof (struct message));
                // printf("Before read:\n");
                // printf("reading from: %d\n",newUser->clientFD);
                //printPacket(incomingPacket);

                if(newUser->clientFD!=-1){
                recvBytes = read(newUser->clientFD, incomingPacket, sizeof(struct message));
                }
                else{
                    close(newUser->clientFD);
                    continue;
                }
                // printf("After read:\n");

                // printf("bytes received from client: %d\n",recvBytes);

                // if(recvBytes < 0){
                //     perror("Error receiving!\n");
                //     exit(EXIT_FAILURE);
                // }

                printf("\nReceiving packet: \n");
                printPacket(incomingPacket);

                //before processing packet, fill in user info as much as possible
                packetToSend = processPacket(*incomingPacket,newUser);
                printf("\nServer's gonna send:\n");
                printPacket(&packetToSend);

                ptrToPacketToSend = &packetToSend;
                if(packetToSend.type==-9) {
                    //printf("DONT SEND AN ACK ITS A QUIT!\n");
                    pthread_exit(&cancelThreadID);
                }
                if(!message){
                    int sendBytes = write(newUser->clientFD, ptrToPacketToSend, sizeof(struct message));
                    //printf("sendBytes: %d\n", sendBytes);
                    //if (sendBytes==-1)  printf("------------Sent!-----------------\n");
                }
                // if(newUser->quit!=true){
                //     printf("User wants to exit server!\n");
                //     newUser->clientFD = -1;
                //     close(newUser->clientFD);
                // }
                //clean up for next round
                free(incomingPacket);
                incomingPacket = NULL;
            
                firstClient = false;
        }
        
                return NULL;

}

int main(int argc, char *argv[]){
    char *portNum;
    struct addrinfo servAddr;
    struct addrinfo* servAddrPtr;
    struct sockaddr_storage cliAddr;  
    int sockfd;

    //expecting server <port num>
    if(argc != 2){
        printf("Error: the number of inputs is incorrect!\n");
        exit(EXIT_FAILURE);
    }

//===============Setup===============
    //make the socket
    portNum = argv[1];
    printf("%s\n", portNum);

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.ai_family = AF_INET;       //IPv4
    servAddr.ai_socktype = SOCK_STREAM;
    servAddr.ai_flags = AI_PASSIVE;

    //allocate linked list(fills the structs needed later) using getaddrinfo
    int structureSetupSuccess = getaddrinfo(NULL, portNum, &servAddr, &servAddrPtr);
 
    if(structureSetupSuccess < 0 ) { 
        printf("Structure setup failed!\n"); 
        exit(EXIT_FAILURE);
    }

    //create unbound socket in domain
    //servAddrPtr->ai_family: which is IPv4
    //servAddrPtr->ai_socktype is datagram for UDP
    //protocol is 0: default protocol
    //sockfd: used to access the socket later
    sockfd = socket(servAddrPtr -> ai_family, servAddrPtr->ai_socktype, servAddrPtr->ai_protocol);
    printf("MY SOCKETFD: %d\n", sockfd);
    if(sockfd < 0){
        printf("Socket failed!\n");
        exit(EXIT_FAILURE);
    } 

    
    //bind socket made
    //assign address to unbound socket just made, unbound socket CAN'T receive anything
    //assigns the address in the 2nd parameter(servAddrPtr -> ai_addr) to the socket in the 1st parameter 
    //which is the socket's file descriptor
    //3rd parameter specifies the size, in bytes, of the address structure pointed to by addr
    int bindRet = bind(sockfd, (struct sockaddr * )servAddrPtr -> ai_addr,servAddrPtr -> ai_addrlen);
    printf("bind: %d\n", bindRet);
    if(bindRet < 0){
        printf("Bind failed!\n");
        exit(EXIT_FAILURE); 
    } 

    //listen for connections
    if(listen(sockfd, BACKLOG) == -1){
        perror("Listen\n");
        exit(1);
    }  

//=============Clients coming=============
    firstClient = true;
    bool logout = false;
    int acceptFD;
    socklen_t addrLen = sizeof(cliAddr); 

    do{
        while(1){
                User *newUser = calloc(sizeof(User), 1);
                newUser->clientFD = accept(sockfd, (struct sockaddr *)&(cliAddr), &addrLen);
                if(newUser->clientFD < 0){
                    perror("newUser->clientFD\n");  
                    exit(1); 
                }
                newUser -> next = NULL;
                newUser -> nextAvailIndex = 0;

                pthread_mutex_lock(&numUserConnected_mutex);
                numUserConnected++;
                pthread_mutex_unlock(&numUserConnected_mutex);
        
                pthread_create(&(newUser->tid), NULL, handle, (void *) newUser);
                
        }
    }while(numUserConnected>0);
    
    
    close(sockfd);
    return (EXIT_SUCCESS);
}