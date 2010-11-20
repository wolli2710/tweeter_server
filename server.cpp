#include "server.h"

server::server()
{

}

server::~server()
{
    //dtor
}

void server::startWinSock(){
#ifdef WIN32
	WORD wVersionRequested = MAKEWORD(2,2);
	WSADATA wsaData;
	if(WSAStartup(wVersionRequested, &wsaData)!=0){
        printf("WinSocket not available");
        exit(1);
	}
#endif
}

void server::run(){

    startWinSock();

    serverSocket =  socket(AF_INTE, SOCK_STREAM, 0);
    if(serverSocket == INVALID_SOCKET){
        error("Socket U fail");
        exit(1);
    }
   
    timeout.tv_sec = 3 * 60;
    timeout.tv_usec = 0;

    bind(char* address);    
    
    listenServer(); 
    
    FD_ZERO(&fdSet);
    maxSocket = serverSocket;
    FD_SET(serverSocket, &fdSet);
    
    while(1){
        
    }    
}

void server::bindServer(char* address, int port){
    memset(&addr, 0, sizeof(SOCKADDR_IN);
    addr.sin_family=AF_INET;
    addr.sin_port=htons(5000); 
    addr.sin_addr.s_addr=gethostbyname(address);

    int rc;
    rc = bind(serverSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN);
    if(rc == SOCKET_ERROR){
        error("Binding failed");
        exit(1);
    }
}

void server::listenServer(){
    int rc;
    rc = listen(serverSocket, BACKLOG);
    if(rc == SOCKET_ERROR){
        error("listen failed ");
        exit(1);
    }
}

void server::error(char* string){
	#ifndef WIN32
		cout << endl << perror(string) << endl;
	#else 
		cout << endl << string << " " << WSAGetLastError() << endl;
	#endif
}
