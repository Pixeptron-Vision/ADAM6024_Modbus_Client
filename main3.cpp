#include <winsock.h>
#include <stdio.h>
#include <conio.h>
#include <unistd.h>

#define DO_0 16            //-Used for DO-0 ID
#define DO_1 17            //-Used for DO-1 ID
#define DI_ALL 0           //-Used for DI ID
#define DO_ALL 16          //-Used for DO ID
#define DO_READ 1          //-Fn. code/command for reading DO
#define DO_WRITE 5         //-Fn. code/command for writing DO
#define DI_READ 2          //-Fn. code/command for reading DI
#define NO_OF_REGISTERS 2  //-Used for Reading DI Status
#define SET 0xff00         //-Used for writing DO Status
#define RESET 0x0000       //-Used for writing DO Status

SOCKET s;
char adamIPAddress[14]="192.168.48.77";
bool socketInitStatus;
int adamPortID;
bool DI0,DI1,DO0,DO1;

bool ADAM_init()
{
    // Initialize the socket variables
    adamPortID= 502;
    // initialize WinSock
    static WSADATA wd;
    if (WSAStartup(0x0101, &wd))
    {
        printf("cannot initialize WinSock\n");
        return false;
    }
    // set up socket
    int commandResponse;
    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(adamPortID); // ASA standard port
    server.sin_addr.s_addr = inet_addr(adamIPAddress);
    commandResponse = connect(s, (sockaddr *)&server, sizeof(sockaddr_in));
    if (commandResponse<0) // #define SOCKET_ERROR            (-1)
    {
        //-Print Error - Unable to connect
        printf("connect - error %d\n",WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return false;
    }
    fd_set fds;//typedef struct fd_set {
               // u_int   fd_count;               /* how many are SET? */
               //SOCKET  fd_array[FD_SETSIZE];   /* an array of SOCKETs */
               //} fd_set;
    FD_ZERO(&fds);//#define FD_ZERO(set) (((fd_set FAR *)(set))->fd_count=0)
                  //Initialize the file descriptor
    timeval tv;	//struct timeval {
                //		long    tv_sec;         /* seconds */
                //		long    tv_usec;        /* and microseconds */
                //};
    tv.tv_sec = 5; // Timeout parameter for the function "select"
    tv.tv_usec = 0;
    // wait for permission to send
    FD_SET(s, &fds);//#define FD_SET(fd, set) do { \
                    //	if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) \
                    //		((fd_set FAR *)(set))->fd_array[((fd_set FAR *)(set))->fd_count++]=(fd);\
                    //} while(0)

    commandResponse = select(32, NULL, &fds, NULL, &tv); // write
    if (commandResponse<=0)
    {
        //-Socket Selection Error
        printf("select - error %d\n",WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return false;
    }
    //-All executions fine. Socket Initialized
    return true;
}
bool COMMAND(int reg_no, int command, int status)
{
    // Check if the Socket is initialized
    if (!socketInitStatus)
    {
        //- Print Status Socket Not initialized
        //- Initialize socket
        socketInitStatus = ADAM_init();
        if(!socketInitStatus)
        {
            //-Status Display => Unable to connect to socket/device.
        }
    }
    // Set the MODBUS Message parameters
    // Register No. or ID =16 for DO_0 , address=0x00017  ; ID =17 for DO_1, address=0x00018
    // build request of form 0 0 0 0 0 6 ui 3 rr rr nn nn
    unsigned char obuf[261]; //Command Format
    unsigned char ibuf[261]; //Response Format
    obuf[0] = 0;
    obuf[1] = 0;
    obuf[2] = 0;
    obuf[3] = 0;
    obuf[4] = 0;//Length byte-1
    obuf[5] = 6;//Length byte-2
    obuf[6] = 0;//Unit Identifier(slave address)
    obuf[7] = command;//Function code
    obuf[8] = reg_no >> 8;//The upper eight bits of the starting address of the read register
    obuf[9] = reg_no & 0xff;//Read the lower eight bits of the starting address of the register
    obuf[10] = status >> 8;//The upper eight bits of the number of output value or the no. of registers
    obuf[11] = status & 0xff;//The lower eight bits of the number of output value or the no. of registers
    // Send the command to server
    int commandResponse = send(s, (const char *)obuf, 12, 0);//int PASCAL FAR send (
                                         //IN SOCKET s,
                                         //IN const char FAR * buf,
                                         //IN int len,
                                         //IN int flags);
    if (commandResponse<12)
    {
        printf("failed to send all 12 chars\n");
    }
//    //wait for response
//    FD_SET(s, &fds);
//    i = select(32, &fds, NULL, NULL, &tv); //read
//    if (i<=0)
//    {
//        printf("no TCP response received\n");
//        closesocket(s);
//        WSACleanup();
//        return 1;
//    }
    // read response
    commandResponse = recv(s, ( char*)ibuf, 261, 0);
    // Check for errors in response
    if (commandResponse<9)
    {
        if (commandResponse==0)
        {
            printf("unexpected close of connection at remote end\n");
            //-Connection closed error message
        }
        else
        {
            printf("response was too short - %d chars\n", commandResponse);
            //-Response too short error message
        }
    }
    else if (ibuf[7] & 0x80)
    {
        printf("Modbus exception response - type %d\n", ibuf[8]);
        //-Server Modbus exception status update
    }
    switch(command) // Read Function code of DI or DO
    {
    case 1: //- Read DO Status
        {
            switch(ibuf[9])
            {
            case 0:
                DO0 = false;
                DO1= false;
                break;
            case 1:
                DO0 = true;
                DO1= false;
                break;
            case 2:
                DO0 = false;
                DO1= true;
                break;
            case 3:
                DO0 = true;
                DO1 = true;
                break;
            }
            break;
        }
    case 2: //-Read DI Status
        {
            switch(ibuf[9])
            {
            case 0:
                DI0 = false;
                DI1= false;
                break;
            case 1:
                DI0 = true;
                DI1= false;
                break;
            case 2:
                DI0 = false;
                DI1= true;
                break;
            case 3:
                DI0 = true;
                DI1 = true;
                break;
            }
            break;
        }
    }
    //-Return server acknowledgment
    return true;

}

int main()
{
    bool commandResponse;
    socketInitStatus = ADAM_init();
    commandResponse = COMMAND(DO_0,DO_WRITE,SET);
    commandResponse = COMMAND(DO_1,DO_WRITE,SET);
    usleep(100000); // 100 ms
    commandResponse = COMMAND(DO_0,DO_WRITE,RESET);
    commandResponse = COMMAND(DO_1,DO_WRITE,RESET);
    // close down
    usleep(100000); // 100 ms
    commandResponse = COMMAND(DO_0,DO_WRITE,SET);
    commandResponse = COMMAND(DO_1,DO_WRITE,SET);
    usleep(100000); // 100 ms
    commandResponse = COMMAND(DO_0,DO_WRITE,RESET);
    commandResponse = COMMAND(DO_1,DO_WRITE,RESET);
    // close down
    usleep(100000); // 100 ms
//    commandResponse = COMMAND(DO_0,DO_WRITE,SET);
//    commandResponse = COMMAND(DO_1,DO_WRITE,SET);
//    usleep(100000); // 100 ms
//    commandResponse = COMMAND(DO_0,DO_WRITE,RESET);
//    commandResponse = COMMAND(DO_1,DO_WRITE,RESET);
    printf("close down\n");
    closesocket(s);
    WSACleanup();
return 0;
}


/*
    commandResponse = COMMAND(DI_ALL,DI_READ,NO_OF_REGISTERS);
    printf(" DI0:%d ,DI1:%d ; DO0:%d ,DO1:%d \n",DI0,DI1,DO0,DO1);
    commandResponse = COMMAND(DO_0,DO_WRITE,SET);
    commandResponse = COMMAND(DO_ALL,DO_READ,NO_OF_REGISTERS);
    printf(" DI0:%d ,DI1:%d ; DO0:%d ,DO1:%d \n",DI0,DI1,DO0,DO1);
    commandResponse = COMMAND(DO_0,DO_WRITE,RESET);
    commandResponse = COMMAND(DO_ALL,DO_READ,NO_OF_REGISTERS);
    printf(" DI0:%d ,DI1:%d ; DO0:%d ,DO1:%d \n",DI0,DI1,DO0,DO1);
    commandResponse = COMMAND(DO_1,DO_WRITE,SET);
    commandResponse = COMMAND(DO_ALL,DO_READ,NO_OF_REGISTERS);
    printf(" DI0:%d ,DI1:%d ; DO0:%d ,DO1:%d \n",DI0,DI1,DO0,DO1);
    commandResponse = COMMAND(DO_1,DO_WRITE,RESET);
    commandResponse = COMMAND(DO_ALL,DO_READ,NO_OF_REGISTERS);
    printf(" DI0:%d ,DI1:%d ; DO0:%d ,DO1:%d \n",DI0,DI1,DO0,DO1);
*/
