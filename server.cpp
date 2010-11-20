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

    listenSocket =  socket(AF_INTE, SOCK_STREAM, 0);
    if(listenSocket == INVALID_SOCKET){
        error("Socket U fail");
        exit(1);
    }
   
    timeout.tv_sec = 3 * 60;
    timeout.tv_usec = 0;

    bind(char* address);    
    
    listenServer(); 
    
    FD_ZERO(&fdSet);
    maxSocket = listenSocket;
    FD_SET(listenSocket, &fdSet);
    
    while(1){
		memcpy(&workingSet, &fdSet, sizeof(fdSet));
		int rc, descriptor_ready;
		rc = select(maxSocket+1, &fdSet, NULL, NULL, &timeout);

		if(rc<0){
			error("select failed");
			break;
		}
		if(rc==0){
			cout<< "select timeout";
			continue;
		}
		
		descriptor_ready = rc;

		for(i=0; i <= maxSocket && descriptor_ready > 0 ; i++){
			if(FD_ISSET(i, &workingSet)) {
				descriptor_ready-- ;
			
				if(i == listenSocket){
					do{
						newSocket = accept(listenSocket, NULL, NULL);
						if(newSocket < 0){
							error("client connection failed");
							break;
						}
						cout<<"New client "<<newSocket<<endl;
						//set new Socket to 1 in fdSet
						FD_SET(newSocket, &fdSet);		
						if(newSocket > maxSocket){
							maxSocket=newSocket;
						}
					}while(newSocket != -1)
				}
				else{
					closeConnection=false;
					while(true){
						rc = recv(i, receiveBuffer, sizeof(receiveBuffer),0);
						if(rc<0){
							if(errno == EWOULDBLOCK){
								closeConnection=true;
							}
							break;
						}
						if(rc==0){
							closeConnection=true;
							break;
						}
						//todo!!!!!!!!!!!!!!!!!!!!!
					}
				}
			}
		}
    }    
}

void server::bindServer(char* address, int port){
    memset(&addr, 0, sizeof(SOCKADDR_IN);
    addr.sin_family=AF_INET;
    addr.sin_port=htons(5000); 
    addr.sin_addr.s_addr=gethostbyname(address);

    int rc;
    rc = bind(listenSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN);
    if(rc == SOCKET_ERROR){
        error("Binding failed");
        exit(1);
    }
}

void server::listenServer(){
    int rc;
    rc = listen(listenSocket, BACKLOG);
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
