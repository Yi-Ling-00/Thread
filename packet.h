#ifndef MAKE_PACKET
#define MAKE_PACKET

#define MAX_NAME 50

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//  5050
// /login me 123 127.0.0.1 5050
// /login hi 123 127.0.0.1 5050
// /login helo 123 127.0.0.1 5050
// /login 1 123 127.0.0.1 5050
// /login 2 123 127.0.0.1 5050
// /login 3 123 127.0.0.1 5050
// /login 4 123 127.0.0.1 5050
char * emptyData = "";

//                      0       1        2          3         4     5       6
enum dataCtrlType {LOGIN_TYPE, LO_ACK, LO_NAK, EXIT_SERVER, JOIN, JN_ACK,JN_NAK, 
LEAVE_SESS, NEW_SESS, NS_ACK, MESSAGE, QUERY, QU_ACK, LEAVE_ACK, NS_NAK, LEAVE_NAK, EXIT_ACK, EXIT_NAK,CLIENT_LOGOUT, INVITATION, INVITE_ACK};
//    7         8       9       10      11      12      13         14       15          16      17          18          19       20

enum clientCommands {LOGIN, LOGOUT, JOINSESSION, LEAVESESSION, CREATESESSION, LIST, QUIT, INVITE};

enum clientCommands convertToEnum(char * str){
    if(strcmp(str,"login")==0) return LOGIN;
    if(strcmp(str,"logout")==0) return LOGOUT;
    if(strcmp(str,"joinsession")==0) return JOINSESSION;
    if(strcmp(str,"leavesession")==0) return LEAVESESSION;
    if(strcmp(str,"createsession")==0) return CREATESESSION;
    if(strcmp(str,"list")==0) return LIST;
    if(strcmp(str,"quit")==0) return QUIT;
    if(strcmp(str, "invite")==0) return INVITE;
    else return -1;
}

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAXBUFLEN];
};


struct message makeLoginPacket(char * clientID, char * pw){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

    //create a packet using struct message
    // char clientID[MAX_NAME], pw[MAX_DATA-MAX_NAME];
    // char serverIP[20], serverPortNum[40];
    unsigned char ucPacketData[MAXBUFLEN],ucClientID[MAX_NAME];
    // sscanf(str, "%s %s %s %s", clientID, pw, serverIP, serverPortNum);

    char packetData[MAXBUFLEN] = {'\0'};
    strcat(packetData, clientID);
    strcat(packetData, ",");
    strcat(packetData, pw);
    printf("Login packet data: %s\n",packetData);

    strcpy((char *)ucPacketData,packetData);
    strcpy((char *)ucClientID,clientID);

    struct message Msg;
    Msg.type = LOGIN_TYPE;
    Msg.size = strlen((char * )ucPacketData);
    memcpy(Msg.source, (unsigned char *) ucClientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) ucPacketData, sizeof(Msg.data));
    return Msg;
}

struct message makeQuitPacketAck(char * clientID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    printf("Quit packet\n");
    struct message Msg;
    Msg.size = 0;
    memcpy(Msg.source, (unsigned char *) clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) emptyData, sizeof(Msg.data));

    return Msg;
}


//need makeLogoutPacket....
struct message makeLogoutPacket(char * clientID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = CLIENT_LOGOUT;
    Msg.size = 0;
    memcpy(Msg.source, (unsigned char *) clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) emptyData, sizeof(Msg.data));

    return Msg;
}



struct message makeJoinSessPacket(char * clientID, char * sessionID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = JOIN;
    Msg.size = strlen((char *)sessionID);
    memcpy(Msg.source, (unsigned char *) clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) sessionID, sizeof(Msg.data));
    return Msg;
}

struct message makeLeaveSessPacket(char *clientID, char * sessionID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = LEAVE_SESS;
    Msg.size = strlen((char *)sessionID);
    memcpy(Msg.source, (unsigned char *) clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) sessionID, sizeof(Msg.data));
   // strcpy(Msg.data," ",sizeof(Msg.data) );
    return Msg;
}

struct message makeInvitePacket(char *clientID, char *inviteID,  char * sessionID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = INVITATION;
    Msg.size = strlen((char *)sessionID);
    int inviteFD = returnInviteFD(inviteID);
    memcpy(Msg.source, (unsigned char *) clientID, sizeof(Msg.source));
    sprintf(Msg.data, "%d:%s", inviteFD, sessionID);
    printf("%s said: Please join sesion %s!\n",clientID,  sessionID);
    return Msg;
}

struct message makeInvitePacketAck(char *clientID, int tempFD){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = INVITE_ACK;
    memcpy(Msg.source, (unsigned char *) clientID, sizeof(Msg.source));
    if (tempFD==-1)  sprintf(Msg.data, "Invalid User!\n",clientID);
    else sprintf(Msg.data, "Packet Sent!\n",clientID);
    Msg.size = strlen((char *)Msg.data);
    return Msg;
}



struct message makeCreateSessPacket(char *clientID, char * sessionID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = NEW_SESS;
    Msg.size = strlen((char *)sessionID);
    memcpy(Msg.data, (unsigned char *) sessionID, sizeof(Msg.data));
    memcpy(Msg.source, (unsigned char *) clientID, sizeof(Msg.source));
    return Msg;
}

/*Not done 
struct message makeQuitPacket(){
}
*/

struct message makeLoAckPacket(User * client){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = LO_ACK;
    Msg.size = 0;
    printf("Acking this client: %s\n", client -> clientID);
    memcpy(Msg.source, (unsigned char *) client -> clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) emptyData, sizeof(Msg.data));
    return Msg;
}

struct message makeLoNakPacket(User * client, char * reasonFailed){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = LO_NAK;
    Msg.size = strlen((char * )reasonFailed);
    memcpy(Msg.source, (unsigned char *) client -> clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) reasonFailed, sizeof(Msg.data));
    return Msg;
}

struct message makeJnAckPacket(User * client, unsigned char * sessionID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = JN_ACK;
    Msg.size = strlen((char *)sessionID);
    memcpy(Msg.source, (unsigned char *) client -> clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) sessionID, sizeof(Msg.data));
    return Msg;
}

struct message makeJnNakPacket(User * client, unsigned char * sessionID, char * reasonFailed){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

    unsigned char ucPacketData[MAXBUFLEN];//,ucClientID[MAX_NAME];
    // sscanf(str, "%s %s %s %s", clientID, pw, serverIP, serverPortNum);

    char packetData[MAXBUFLEN] = {'\0'};
    strcat(packetData, (char *)sessionID);
    strcat(packetData, ",");
    strcat(packetData, reasonFailed);

    strcpy((char *)ucPacketData,packetData);

    // printf("~~~~~source: %s\n", (unsigned char *) ucClientID);
    // printf("~~~~~data: %s\n", (unsigned char *) ucPacketData);

    struct message Msg;
    Msg.type = JN_NAK;
    Msg.size = strlen((char * )ucPacketData);
    // strcpy(Msg.source, (unsigned char *) ucClientID);
    // strcpy(Msg.data, (unsigned char *) ucPacketData);
    memcpy(Msg.source, (unsigned char *) client -> clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) ucPacketData, sizeof(Msg.data));
    return Msg;
}


struct message makeNsAckPacket(User * client, char * sessionID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = NS_ACK;
    Msg.size = strlen(sessionID);
    memcpy(Msg.source, (unsigned char *) client -> clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) sessionID, sizeof(Msg.data));
    return Msg;
}

struct message makeNsNakPacket(User * client, char * whyCantCreateSession){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = NS_NAK;
    Msg.size = strlen(whyCantCreateSession);
    memcpy(Msg.source, (unsigned char *) client -> clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) whyCantCreateSession, sizeof(Msg.data));
    return Msg;
}

struct message makeLeaveAckPacket(User * client, unsigned char * sessionID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = LEAVE_ACK;
    Msg.size = strlen((char *)sessionID);
    memcpy(Msg.source, (unsigned char *) client -> clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) sessionID, sizeof(Msg.data));
    return Msg;
}

struct message makeLeaveNakPacket(User * client, unsigned char * sessionID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = LEAVE_NAK;
    Msg.size = strlen((char *)sessionID);
    memcpy(Msg.source, (unsigned char *) client -> clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) sessionID, sizeof(Msg.data));
    return Msg;
}


struct message makeMessagePacket(char *clientID, char * messageData){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = MESSAGE;
    Msg.size = strlen((char *)messageData);
    memcpy(Msg.source, (unsigned char *) clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *) messageData, sizeof(Msg.data));
    return Msg;
}

struct message makeQueryPacket(char *clientID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = QUERY;
    Msg.size = 0;
    memcpy(Msg.source, (unsigned char *) clientID, sizeof(Msg.source));
    memcpy(Msg.data, (unsigned char *)emptyData, sizeof(Msg.data));
    return Msg;
}

//!!!not sure what is expected
//To: add list of usrs and sessions
struct message makeQuAckPacket(unsigned char *clientID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    struct message Msg;
    Msg.type = QU_ACK;
    char sessionListArr[1000];
    strcpy(sessionListArr, "Session: ");
    char userListArr[3000];
    strcpy(userListArr, "User: ");
    memcpy(Msg.source, (unsigned char *) clientID, sizeof(Msg.source));

    Session *tempSess= sessionList;
    User *tempClient= clientList;
    if (tempSess==NULL){
       strcat(sessionListArr, "Empty!");
    }

    while(tempSess!=NULL){
        printf("In Packet.h line 267\n");
        if (tempSess->numClients!=0){ 
        printf("Session not empty!\n");
        strcat(sessionListArr, tempSess->sessionID);
        strcat(sessionListArr, "\t");
        }
        if(tempSess -> next != NULL)tempSess = tempSess -> next;
        else break;

    }

    printf("Session list: %s\n", sessionListArr);
    if (tempClient==NULL){
       strcat(userListArr, "Empty!");
    }
    while(tempClient!=NULL){
        strcat(userListArr, (char *)tempClient->clientID);
        strcat(userListArr, "\t");
        if(tempClient -> next != NULL)tempClient = tempClient -> next;
        else break;
    }
    printf("User list: %s\n", userListArr);
    
    strcat(userListArr, "\n");
    strcat(userListArr, sessionListArr);

    memcpy(Msg.data, (unsigned char *)userListArr, sizeof(Msg.data));
    printf("Msg.data: %s\n", Msg.data);

    memcpy(Msg.data, (char *)userListArr, sizeof(Msg.data));
    Msg.size = strlen((char *)userListArr);
    return Msg;
}

struct message makeLogoutAckPacket(unsigned char *clientID){
   struct message Msg;
   Msg.type = EXIT_ACK;
   Msg.size = 0;
   memcpy(Msg.data, emptyData, sizeof(Msg.data));
    memcpy(Msg.source, clientID, sizeof(Msg.source));
   return Msg;
}
struct message makeLogoutNakPacket(unsigned char *clientID, char * whyfailed){
    struct message Msg;
    Msg.type = EXIT_NAK;
    Msg.size = strlen("Logout");
    memcpy(Msg.data, (unsigned char *)whyfailed, sizeof(Msg.data));
    memcpy(Msg.source, clientID, sizeof(Msg.source));
    return Msg;
}

struct message makeQuitPacket(char *clientID){
   struct message Msg;
   Msg.type= EXIT_SERVER;
   Msg.size = strlen("Quit");
   memcpy(Msg.data, "Quit", sizeof(Msg.data));
    memcpy(Msg.source, (unsigned char *) clientID, sizeof(Msg.source));
   return Msg;
}


void printPacket(struct message * packet){
    printf("\nPrint packet: \n");
    printf("type : %d\n", packet -> type);
    printf("size : %d\n", packet -> size);
    printf("source : %s\n", packet -> source);
    printf("data : %s\n", packet -> data);
}
    

#endif