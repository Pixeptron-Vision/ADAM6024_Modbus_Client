#include <winsock.h>
#include <stdio.h>
#include <conio.h>

//int _tmain(int argc, char **argv)
  // test1.cpp 5/23/97
// example Win32 C++ program to read registers from PLC via gateway
// compile with BC45 or BC50
// default settings for Win32 console app
// empty DEF file
int main()
{
	//printf("usage: test1 ip_adrs unit reg_no num_regs\n"
	// "eg test1 198.202.138.72 5 0 10\n");
char ip_adrs[] = "192.168.48.77";
unsigned short unit = 0;//*(short*)(argv[2]);
unsigned short reg_no = 1;
unsigned short num_regs =2;
unsigned short fn_code = 2;
printf("ip_adrs = %s unit = %d reg_no = %d num_regs = %d\n",ip_adrs, unit, reg_no, num_regs);
// initialize WinSock
static WSADATA wd;
if (WSAStartup(0x0101, &wd))
{
	printf("cannot initialize WinSock\n");
	return 1;
}
// set up socket
SOCKET s;
s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
struct sockaddr_in server;
server.sin_family = AF_INET;
server.sin_port = htons(502); // ASA standard port
server.sin_addr.s_addr = inet_addr(ip_adrs);
int i;
i = connect(s, (sockaddr *)&server, sizeof(sockaddr_in));
if (i<0) // #define SOCKET_ERROR            (-1)
{
	printf("connect - error %d\n",WSAGetLastError());
	closesocket(s);
	WSACleanup();
	return 1;
}
fd_set fds;//typedef struct fd_set {
           // u_int   fd_count;               /* how many are SET? */
           //SOCKET  fd_array[FD_SETSIZE];   /* an array of SOCKETs */
           //} fd_set;
FD_ZERO(&fds);//#define FD_ZERO(set) (((fd_set FAR *)(set))->fd_count=0)
timeval tv;	//struct timeval {
			//		long    tv_sec;         /* seconds */
			//		long    tv_usec;        /* and microseconds */
			//};
tv.tv_sec = 5;
tv.tv_usec = 0;
// wait for permission to send
FD_SET(s, &fds);//#define FD_SET(fd, set) do { \
				//	if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) \
				//		((fd_set FAR *)(set))->fd_array[((fd_set FAR *)(set))->fd_count++]=(fd);\
				//} while(0)
				//此处将s赋给fds，使fds的成员fd_count＝1，fd_array[0]=s;
i = select(32, NULL, &fds, NULL, &tv); // write
			//多路复用——select()//功能：用来检测一个或多个套接字状态。
			//格式：int PASCAL FAR select(int nfds,fd_set FAR * readfds,fd_set FAR * writefds,
			//fd_set FAR * exceptfds(//指向要检测是否出错的指针),const struct timeval FAR * timeout);
if (i<=0)
{
	printf("select - error %d\n",WSAGetLastError());
	closesocket(s);
	WSACleanup();
	return 1;
}
// build request of form 0 0 0 0 0 6 ui 3 rr rr nn nn
unsigned char obuf[261];
unsigned char ibuf[261];
obuf[0] = 0;
obuf[1] = 0;
obuf[2] = 0;
obuf[3] = 0;
obuf[4] = 0;//Length byte-1
obuf[5] = 6;//Length byte-2
obuf[6] = unit;//Unit Identifier(slave address)
obuf[7] = fn_code;//Function code6
obuf[8] = reg_no >> 8;//The upper eight bits of the starting address of the read register
obuf[9] = reg_no & 0xff;//Read the lower eight bits of the starting address of the register
obuf[10] = num_regs >> 8;//The upper eight bits of the number of read registers
obuf[11] = num_regs & 0xff;//The lower eight bits of the number of read registers
// send request
for(int count=0;count<1;count++){
    i = send(s, (const char *)obuf, 12, 0);//int PASCAL FAR send (
                                             //IN SOCKET s,
                                             //IN const char FAR * buf,
                                             //IN int len,
                                             //IN int flags);
    if (i<12)
    {
        printf("failed to send all 12 chars\n");
    }
    //wait for response
    FD_SET(s, &fds);
    i = select(32, &fds, NULL, NULL, &tv); //read
    if (i<=0)
    {
        printf("no TCP response received\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }
    // read response
    i = recv(s, ( char*)ibuf, 261, 0);
    printf("Response Size : %d\n",i);
for (size_t j=0;j<12;j++) printf("%d ",obuf[j]);
printf("\n");
for (size_t j=0;j<i;j++) printf("%d ",ibuf[j]);
printf("\n");
}

if (i<9)
{
	if (i==0)
	{
		printf("unexpected close of connection at remote end\n");
	}
	else
	{
		printf("response was too short - %d chars\n", i);
	}
}
else if (ibuf[7] & 0x80)
{
	printf("Modbus exception response - type %d\n", ibuf[8]);
}
//else if (i != (9+2*num_regs))
//{
//	printf("incorrect response size is %d expected %d\n",i,(9+2*num_regs));
//}
else
{
	for (i=0;i<ibuf[5];i++)
	{
		unsigned short w = (ibuf[9+i+i]<<8) + ibuf[10+i+i];
		printf("word %d = %d\n", i, w);
	}
}
// close down
printf("close down\n");
closesocket(s);
WSACleanup();
return 0;
}







///*========================================================
//
//test2.c 7/23/97 - UNIX C program to read registers via gateway
//
//compile using
//
//cc -o test2 test2.c
//
//run using
//
//test2 198.202.138.73 1 2 3
//
//==========================================================*/
//
//
//
//#include <stdio.h>
//
//#include <sys/types.h>
//
//#include <sys/socket.h>
//
//#include <sys/ioctl.h>
//
//#include <sys/time.h>
//
//#include <netinet/in.h>
//
//
//
///* global data */
//
//
//
//int main(int argc, char **argv)
//
//{
//
//char *ip_adrs;
//
//unsigned short unit;
//
//unsigned short reg_no;
//
//unsigned short num_regs;
//
//int s;
//
//int i;
//
//struct sockaddr_in server;
//
//fd_set fds;
//
//struct timeval tv;
//
//unsigned char obuf[261];
//
//unsigned char ibuf[261];
//
//
//
//if (argc<5)
//
//{
//
//printf("usage: test2 ip_adrs unit reg_no num_regs\n"
//
//"eg test2 198.202.138.72 5 0 10\n");
//
//return 1;
//
//}
//
//
//
///* confirm arguments */
//
//ip_adrs = argv[1];
//
//unit = atoi(argv[2]);
//
//reg_no = atoi(argv[3]);
//
//num_regs = atoi(argv[4]);
//
//printf("ip_adrs = %s unit = %d reg_no = %d num_regs = %d\n",
//
//ip_adrs, unit, reg_no, num_regs);
//
//
//
///* establish connection to gateway on ASA standard port 502 */
//
//s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
//
//server.sin_family = AF_INET;
//
//server.sin_port = htons(502); /* ASA standard port */
//
//server.sin_addr.s_addr = inet_addr(ip_adrs);
//
//
//
//i = connect(s, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
//
//if (i<0)
//
//{
//
//printf("connect - error %d\n",i);
//
//close(s);
//
//return 1;
//
//}
//
//
//
//FD_ZERO(&fds);
//
//tv.tv_sec = 5;
//
//tv.tv_usec = 0;
//
//
//
///* check ready to send */
//
//FD_SET(s, &fds);
//
//i = select(32, NULL, &fds, NULL, &tv);
//
//if(0)if (i<=0)
//
//{
//
//printf("select - error %d\n",i);
//
//close(s);
//
//return 1;
//
//}
//
//
//
///* build Modbus request */
//
//for (i=0;i<5;i++) obuf[i] = 0;
//
//obuf[5] = 6;
//
//obuf[6] = unit;
//
//obuf[7] = 3;
//
//obuf[8] = reg_no >> 8;
//
//obuf[9] = reg_no & 0xff;
//
//obuf[10] = num_regs >> 8;
//
//obuf[11] = num_regs & 0xff;
//
//
//
///* send request */
//
//i = send(s, obuf, 12, 0);
//
//if (i<12)
//
//{
//
//printf("failed to send all 12 chars\n");
//
//}
//
//
//
///* wait for response */
//
//FD_SET(s, &fds);
//
//i = select(32, &fds, NULL, NULL, &tv);
//
//if (i<=0)
//
//{
//
//printf("no TCP response received\n");
//
//close(s);
//
//return 1;
//
//}
//
//
//
///* receive and analyze response */
//
//i = recv(s, ibuf, 261, 0);
//
//if (i<9)
//
//{
//
//if (i==0)
//
//{
//
//printf("unexpected close of connection at remote end\n");
//
//}
//
//else
//
//{
//
//printf("response was too short - %d chars\n", i);
//
//}
//
//}
//
//else if (ibuf[7] & 0x80)
//
//{
//
//printf("Modbus exception response - type %d\n", ibuf[8]);
//
//}
//
//else if (i != (9+2*num_regs))
//
//{
//
//printf("incorrect response size is %d expected %d\n",i,(9+2*num_regs));
//
//}
//
//else
//
//{
//
//for (i=0;i<num_regs;i++)
//
//{
//
//unsigned short w = (ibuf[9+i+i]<<8) + ibuf[10+i+i];
//
//printf("word %d = %d\n", i, w);
//
//}
//
//}
//
//
//
///* close down connection */
//
//close(s);
//
//
//
//}
//
//
//
//============================================================
//
//test3.java - JAVA program to read registers
//
//============================================================
//
//
//
//
//
//// test3.java 7/23/97 - JAVA program to read registers via gateway
//
//// compile as
//
//// javac test3.java
//
//// run as
//
//// java test3 aswales1.modicon.com 1 2 3
//
//
//
//
//
//import java.io.* ;
//
//import java.net.* ;
//
//import java.util.*;
//
//
//
//class test3 {
//
//public static void main(String argv[]) {
//
//if (argv.length < 4) {
//
//System.out.println("usage: java test3 dns_name unit reg_no num_regs");
//
//System.out.println("eg java test3 aswales8.modicon.com 5 0 10");
//
//return;
//
//}
//
//try {
//
//String ip_adrs = argv[0];
//
//int unit = Integer.parseInt(argv[1]);
//
//int reg_no = Integer.parseInt(argv[2]);
//
//int num_regs = Integer.parseInt(argv[3]);
//
//System.out.println("ip_adrs = "+ip_adrs+" unit = "+unit+" reg_no = "+
//
//reg_no+" num_regs = "+num_regs);
//
//
//
//// set up socket
//
//Socket es = new Socket(ip_adrs,502);
//
//OutputStream os= es.getOutputStream();
//
//FilterInputStream is = new BufferedInputStream(es.getInputStream());
//
//byte obuf[] = new byte[261];
//
//byte ibuf[] = new byte[261];
//
//int c = 0;
//
//int i;
//
//
//
//// build request of form 0 0 0 0 0 6 ui 3 rr rr nn nn
//
//for (i=0;i<5;i++) obuf[i] = 0;
//
//obuf[5] = 6;
//
//obuf[6] = (byte)unit;
//
//obuf[7] = 3;
//
//obuf[8] = (byte)(reg_no >> 8);
//
//obuf[9] = (byte)(reg_no & 0xff);
//
//obuf[10] = (byte)(num_regs >> 8);
//
//obuf[11] = (byte)(num_regs & 0xff);
//
//
//
//// send request
//
//os.write(obuf, 0, 12);
//
//
//
//// read response
//
//i = is.read(ibuf, 0, 261);
//
//if (i<9) {
//
//if (i==0) {
//
//System.out.println("unexpected close of connection at remote end");
//
//} else {
//
//System.out.println("response was too short - "+i+" chars");
//
//}
//
//} else if (0 != (ibuf[7] & 0x80)) {
//
//System.out.println("Modbus exception response - type "+ibuf[8]);
//
//} else if (i != (9+2*num_regs)) {
//
//System.out.println("incorrect response size is "+i+" expected "+(9+2*num_regs));
//
//} else {
//
//for (i=0;i<num_regs;i++) {
//
//int w = (ibuf[9+i+i]<<8) + ibuf[10+i+i];
//
//System.out.println("word "+i+" = "+w);
//
//}
//
//}
//
//
//
//// close down
//
//es.close();
//
//} catch (Exception e) {
//
//System.out.println("exception :"+e);
//
//}
//
//}
//
//}
//
//	return 0;
//}

