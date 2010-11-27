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
    on = 1;
    startWinSock();
    listenSocket =  socket(AF_INET, SOCK_STREAM, 0);
    
    if(listenSocket == INVALID_SOCKET){
        error("Socket U fail");
        exit(1);
    }
    rc = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
    if(rc < 0){
        error("setsocketopt");
    }
#ifndef WIN32
    ioctl(listenSocket, FIONBIO, (char*)&on);
#else
    ioctlsocket(listenSocket, FIONBIO, (u_long*)&on);
#endif
    timeout.tv_sec = 20;
    timeout.tv_usec = 0;

    bindServer("localhost", PORT);  
    listenServer(); 
    FD_ZERO(&fdSet);
    maxSocket = listenSocket;
    FD_SET(listenSocket, &fdSet);
    cout<<"Server started\n";
    while(1){
        //cout << "before memcpy " << endl;
        memcpy(&workingSet, &fdSet, sizeof(fdSet));
        cout << "waiting for anything" << endl;
        rc = select(maxSocket+1, &workingSet, NULL, NULL, &timeout);
        cout<< rc << endl;
        if(rc<0){
            error("select failed");			
        }
        if(rc==0){
            cout<< "select timeout" << endl;
            continue;
        }
        descriptor_ready = rc;
        for(i=0; i <= maxSocket && descriptor_ready > 0 ; i++){            
            if(FD_ISSET(i, &workingSet)) {
                descriptor_ready-- ;
                if(i == listenSocket){
                    listening();
                }
                else {                  
                    closeConnection=false;
                    while(true){
                        rc = recv(i, receiveBuffer, sizeof(receiveBuffer),0);
                        if(rc<0){
                            closeConnection=true;
                            break;
                        }
                        if(rc==0){
                            closeConnection=true;
                            break;
                        }
                        //login				
                        sendingUser = findUserSocket(i);
                        cout << "after receive: " << receiveBuffer << endl;
                        if(sendingUser==""){							
                            login();                         
                        }
                        //messages
                        else{                         
                            //following                            
                            if(receiveBuffer[0] ==  'f' && receiveBuffer[1] == ' '){
                                following();
                            }
                            //messages
                            else{
                                message m (sendingUser,receiveBuffer, timestamp() );
                                messages.push_back(m);
                            }
                        }
                        ////////////////////////////
                    }                  
                }                
            }           
        }       
    }        
}

void server::login(){
    //add user to map if not exist
    if(!findUser(receiveBuffer)){
        users.insert(pair<string, int>(receiveBuffer, i));
    }
    else{
        users.find(receiveBuffer)->second=i;
    }
    cout << "Connected User:" << receiveBuffer << endl;
    memcpy(&sendBuffer, "User logged in", sizeof("User logged in"));
    rc = send(i, sendBuffer, sizeof(sendBuffer), 0);
    //cout << sendBuffer << endl;
}

void server::following(){
    cout << "rb befor cut" << receiveBuffer << endl;
    //memmove(receiveBuffer, receiveBuffer+2, sizeof(receiveBuffer)-2);
    cout << "rb after cut" << receiveBuffer << endl;                                 
    if(findUser(receiveBuffer)){
        memcpy(&sendBuffer, "You follows ", sizeof("You follows "));
        strcat(sendBuffer,receiveBuffer);

        rc = send(i, sendBuffer, sizeof(sendBuffer), 0);
    }
    else{
        memcpy(&sendBuffer, "User does not exists", sizeof("User does not exists"));                                  
        rc = send(i, sendBuffer, sizeof(sendBuffer), 0);
    }
}

void server::listening(){
    do{
        newSocket = accept(listenSocket, NULL, NULL);
                        
        if(newSocket < 0){
            break;
        }
        cout<<"New client " << newSocket << endl;

        memcpy(&sendBuffer, "Connected with Server", sizeof("Connected with Server..."));

        send(newSocket, sendBuffer, sizeof(sendBuffer), 0);

        //set new Socket to 1 in fdSet
        FD_SET(newSocket, &fdSet);		
        if(newSocket > maxSocket){
            maxSocket=newSocket;
        }
    }while(newSocket != -1);
}

void server::bindServer(char* address, int port){
    memset(&addr, 0, sizeof(SOCKADDR_IN));
   /* addr.sin_family=AF_INET;
    addr.sin_port=htons(5000);
    addr.sin_addr.s_addr = ADDR_ANY;
    */
    addr.sin_family=AF_INET;												//IPv4
    addr.sin_port=htons(5000);												// Port 5000 in use
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    //todo addr.sin_addr.s_addr=gethostbyname(address);

    int rc;
    rc = bind(listenSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
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

string server::findUserSocket(int socketNr){
    for(users_it=users.begin(); users_it!=users.end(); users_it++){
        if(users_it->second == socketNr){
            return users_it->first;
        }
    }
    return "";
}

bool server::findUser(string username){
    return users.find(username)!= users.end();
}