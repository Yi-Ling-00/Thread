#include "clientSession.h"
#include "packet.h" 
bool message= false; 
bool inSession= false; 
int sockfd;
bool quit=false;
pthread_t receive_thread;


void loginMessageReceived();
void *receiveMessage();

void loginMessageReceived(){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message * serverPacket = (struct message * ) malloc (sizeof (struct message));

    read(sockfd, ( void *)serverPacket, sizeof(struct message));
    printPacket(serverPacket);

     switch(serverPacket->type){
        case 1:
            printf("Deliver.c- LO_ACK\n ");
            if(pthread_create(&receive_thread, NULL, receiveMessage, NULL)==0) {
            printf("LO_ACK: Successful login!\n");
            }
            break;
        case 2:
            printf("LO_NAK: Unsuccessful login, reason: %s\n", serverPacket->data); 
            break; 
        default:
        printf("Random message received!\n");
        break;
     }

    printf("\n");
    free(serverPacket);
    return;
}   

void *receiveMessage(){
    printf("In receive message function!\n"); 
    int numBytes=0;
    struct message * serverPacket = (struct message * ) malloc (sizeof (struct message));
    //serverPacket= NULL;
    //bool one=true;

    while(1){
        
        read(sockfd, ( void *)serverPacket, sizeof(struct message));
            

        printPacket(serverPacket);

        switch(serverPacket->type){
            
            case 5:
                printf("JN_ACK: Successfully join session\n");
                inSession=true;
                break;
            case 6: 
                printf("JN_NAK: Unsuccessful join session, reason: %s\n", serverPacket->data);
                break;
            case 9: 
                printf("NS_ACK: Successful new session\n");
                inSession=true;
                break;
            case 12: 
                printf("QU_ACK: Users and sessions: %s\n", serverPacket->data);
                break;
            case 13:
                printf("LEAVE_ACK: Successful leave session\n");
                break;
            case 14: 
                printf("NS_NAK: Unsuccessful leave session, reasons: %s\n", serverPacket -> data);
                break;
            case 15:
                printf("LEAVE_NAK: Unsuccessful leave session, reason: %s\n", serverPacket -> data);
                break;
            default:
                printf("\n\n\n----------Unknown packet received!--------\n");
                break;    
        }   

    }
    printf("\n");
    free(serverPacket);
    printf("Deliver.c-Whats wrong with you>>>>???????????????\n");
    return NULL;
}

int main(int argc, char *argv[]){
    
    //int sockfd;
    struct addrinfo servAddr;
    struct addrinfo *servAddrPtr;
    bool promptForLogin = true;
    struct message packetToSend;
    struct message * ptrToPacketToSend;
    bool send = true;
    bool iveLoggedIn = false;
    
    //newly added vars
    char inputStr[1000]={'\0'}; 

 
    //expecting ./deliver <server addr> to run the program
    if (argc != 1){
        printf("Error: the number of inputs is incorrect!\n Expecting ./client\n");
        exit(EXIT_FAILURE);
    }

    while(!quit){
        printf("\n\n****************************\n");
        printf("Enter your prompts below: \n");
        char input[1000],singleWordCommands[1000];
        char fwOfCommand[15], excess[1000];
        enum clientCommands fwEnum; 
        fgets (input, 1000, stdin);
        printf("input: %s\n", input);

        //send the message directly to server.
        //take out the first word of input to decide what to do with it
         
        sscanf(input," /%s %s", fwOfCommand, excess);
        printf("Command: %s\n", fwOfCommand);

        printf("Excess: %s\n", excess);
        fwEnum = convertToEnum(fwOfCommand);
        /*
        if(strcmp(excess, "")==0){
            //no second work, just take input with / removed
            ///memmove(fwOfCommand, fwOfCommand+1, strlen(fwOfCommand));
            fwEnum = convertToEnum(fwOfCommand);
        }else{
            fwEnum = convertToEnum(fwOfCommand); 
        }
        */
         
        printf("enum: %d\n",fwEnum);
 
        
        printf("!\n");
        
        switch(fwEnum){
            //login
            case 0:
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
                printf("login!\n");  

                char loginStr[7],clientIDStr[1000],pwStr[1000],serverIP[1000], serverPortStr[1000];
                sscanf(input, "%s %s %s %s %s", loginStr, clientIDStr, pwStr, serverIP, serverPortStr);
                //printf("You enterred: %s %s %s %s %s\n", loginStr, clientIDStr, pwStr, serverIP, serverPortStr);

                packetToSend = makeLoginPacket(clientIDStr, pwStr);
                ptrToPacketToSend = &packetToSend;

                if(firstClient){
                    memset(&servAddr, 0, sizeof(servAddr));
                    servAddr.ai_family = AF_INET;
                    servAddr.ai_flags = AI_PASSIVE;
                    servAddr.ai_socktype = SOCK_STREAM;

                    int structureSetupSuccess = getaddrinfo(serverIP, serverPortStr, &servAddr, &servAddrPtr);
                    if (structureSetupSuccess != 0){
                        printf("Structure setup failed!\n");
                        printf("structure setup success: %d\n",structureSetupSuccess);
                        exit(EXIT_FAILURE);
                    }

                    printf("creating socket\n");
                    sockfd = socket(servAddrPtr->ai_family, servAddrPtr->ai_socktype, servAddrPtr->ai_protocol);

                    printf("sockfd: %d\n", sockfd); 
                    if (sockfd < 1){
                        printf("Bad socket. Exit");
                        exit(EXIT_FAILURE);
                    }

                
                    if (connect(sockfd, servAddrPtr->ai_addr, servAddrPtr->ai_addrlen) == -1) {
                        close(sockfd);
                        perror("client: connect");
                        exit(EXIT_FAILURE);
                    }

                    
                    //firstClient = false; //Im taking this outside of the case

                    iveLoggedIn = true;

                   
                }
                break;
            //logout
            case 1: 
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
                printf("logout!\n");
                
                packetToSend = makeLogoutPacket(clientIDStr);
                ptrToPacketToSend = &packetToSend;
                //close socket(not actually opened at this point since no outer while loop)
                break;

            //join session
            case 2: 
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
                printf("join session!\n");
                char joinSessionStr[13], joinSessionID[1000];
                sscanf(input, "%s %s", joinSessionStr, joinSessionID);
                packetToSend = makeJoinSessPacket(clientIDStr, joinSessionID);
                ptrToPacketToSend = &packetToSend;

                break;


            //leave session
            case 3:
                //need to specify which session you want to leave
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
                printf("leave session!\n");
                char leaveSession[14],leaveSessionID[1000];
                sscanf(input, "%s %s", leaveSession, leaveSessionID);
                packetToSend = makeLeaveSessPacket(clientIDStr, leaveSessionID);
                ptrToPacketToSend = &packetToSend;

                break;


            //create session 
            case 4:
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
                printf("create session!\n");
                char createSessionStr[13],newSessID[1000];
                sscanf(input, "%s %s", createSessionStr, newSessID);
                printf("new session id: %s\n", newSessID);

                packetToSend = makeCreateSessPacket(clientIDStr,newSessID);
                ptrToPacketToSend = &packetToSend;
                break; 

            //list
            case 5:
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
                printf("list!\n");
                packetToSend = makeQueryPacket(clientIDStr);
                ptrToPacketToSend = &packetToSend;
                
                
                break; 

          //quit
            case 6:
            quit = true;
            printf("------Quitting--------\n");
            packetToSend= makeQuitPacket(clientIDStr);
            //close(sockfd);
            break;

            default:
                printf("Invalid command\n");
                printf("In default case!\n");  
                //if loggedIn == true && currSess not empty: it's a message  
                if(inSession){
                    //printf("In session !\n");
                        
                    printf("Deliver.c- Input: %s\n",input );
                    packetToSend=  makeMessagePacket(clientIDStr,input);
                    ptrToPacketToSend = &packetToSend;
                    message = true; 
                    
                    printf("\nWill send packet: \n");
                    printPacket(ptrToPacketToSend);
                    int numBytes = write(sockfd, ( void *)ptrToPacketToSend, sizeof(packetToSend));
                    //int numBytes = sendto(sockfd, ( void *)ptrToPacketToSend, sizeof(packetToSend), 0, (struct sockaddr *)(servAddrPtr->ai_addr), servAddrPtr->ai_addrlen);
                    
                    if (numBytes < 0){
                    perror("Client failed sending stringToSend!\n");
                    exit(EXIT_FAILURE);
                    }else{
                        printf("Client sent successfully!\n");
                    }

                }else{
                    printf("Not in session/logged in, can't send message!\n");
                    send = false;
                    //wait for next round of correct inputs
                }
                break;
        } 
       
        if(!message && send && iveLoggedIn){
            printf("\nWill send packet: \n");
            printPacket(ptrToPacketToSend);
            int numBytes = write(sockfd, ( void *)ptrToPacketToSend, sizeof(packetToSend));
            //int numBytes = sendto(sockfd, ( void *)ptrToPacketToSend, sizeof(packetToSend), 0, (struct sockaddr *)(servAddrPtr->ai_addr), servAddrPtr->ai_addrlen);
            if (numBytes < 0){
                perror("Client failed sending stringToSend!\n");
                exit(EXIT_FAILURE);
            }else{
                printf("Client sent successfully!\n");
            } 
        }

        if(send && iveLoggedIn){
            printf("\nReceiving from server: \n");
            //struct message * serverPacket = (struct message * ) malloc (sizeof (struct message));
            if(firstClient==true){
                 loginMessageReceived();
                 firstClient= false; 
            }
            else{
            //receiveMessage();
            printf("Did I reach here yet?????\n");
            }
            printf("\n");
        }
        //if you didn't send at all, just reprompt for a valid input
           printf("---------------1-----------------\n");

        //clear for next round 
        memset(input, 0, sizeof input);
        //clear for next round
        memset(fwOfCommand, 0, sizeof fwOfCommand);
        memset(excess, 0, sizeof excess);
        message = false;
        send = true;
        printf("---------------2-----------------\n");

    }
   


    //close(sockfd);
    return 0;      
}
