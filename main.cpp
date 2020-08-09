#include <iostream>
#include <winsock2.h>
#include <chrono>
#include <thread>
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds
using namespace std;

int main()
{
    // Introduce the socket variables
    WSADATA wsaData;
    SOCKET serverSock,clientSock;
    SOCKADDR_IN serverAddr, clientAddr;
    // Initialize the sockets
    WSAStartup(MAKEWORD(2,0),&wsaData);
    serverSock = socket(AF_INET,SOCK_STREAM,0);
    // Define the socket addresses
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr =inet_addr("192.168.51.235");
    serverAddr.sin_port = htons(502);
    //connect to server socket
    connect(serverSock,(SOCKADDR*)&serverAddr,sizeof(serverAddr));
    cout<<"Connected to server"<<endl;
    // Initialize parameters for data
    char buffer[15]="1000006020101";
    //cin>>buffer;

    //send the data to server
    send(serverSock,buffer,sizeof(buffer),0);
    //send(serverSock,char(13),sizeof("\r"),0);
    sleep_for(nanoseconds(1000000000));
    cout<<"Message sent to ADAM"<<endl;
    // Bind the socket to address and port
    bind(serverSock, (SOCKADDR*)&serverAddr,sizeof(serverAddr));

    for (int a=0;a<1;a++){
        // Keep listening to the socket
        listen(serverSock,0);
        cout<<"Listening for incoming connections"<<endl;
        // Initialize parameters for data
        char buffer1[1024]{};
        int clientAddrSize = sizeof(clientAddr);
        // Check for condition
        clientSock = accept(serverSock,(SOCKADDR*)&clientAddr,&clientAddrSize);
        if (clientSock!=1)
        {
            cout<<"ADAM Connected"<<endl;
            int ret = recv(clientSock,buffer1,sizeof(buffer1),0);
            //sleep_for(nanoseconds(10000000));
            cout<<"ADAM Says: "<<ret<<" ; "<<buffer1<<endl;
            memset(buffer1,0,sizeof(buffer1));
            closesocket(clientSock);
            cout<<"ADAM Disconnected"<<endl;
        }

    }

    // close the socket
    closesocket(serverSock);
    WSACleanup();
    cout<<"Client Socket closed"<<endl;

    return 0;
}

