#include "server.h"

server::server()
{

}

server::~server()
{
    //dtor
}

//Windows specific 
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
	output.open("output.txt");
	shutDown=false;
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
    while(!shutDown){
        printFollowers("test");
        printMessages();
        printTweeters();
        memcpy(&workingSet, &fdSet, sizeof(fdSet));
        cout << "waiting for anything" << endl;
        rc = select(maxSocket+1, &workingSet, NULL, NULL, &timeout);
        cout<< rc << endl;
        if(rc<0){
            error("select failed");			
            break;
        }
        if(rc==0){
            cout<< "select timeout" << endl;
            continue;
        }
        descriptor_ready = rc;
        iterateThrowSockets();  
		

    }  
	output.close();
}

void server::iterateThrowSockets(){
    for(i=0; i <= maxSocket && descriptor_ready > 0 ; i++){            
        if(FD_ISSET(i, &workingSet)) {
            descriptor_ready-- ;
             if(i == listenSocket)
                listening();
            else {                  
                closeConnection=false;
                while(true){
                    rc = recv(i, receiveBuffer, sizeof(receiveBuffer),0);
                    if(rc<0){
#ifdef WIN32
                        if(WSAGetLastError() != WSAEWOULDBLOCK)
#else
                        if(errno != EWOULDBLOCK)
#endif
                            closeConnection=true;    
                        break;
                    }
                    if(rc==0){
                        closeConnection=true;
                        break;
                    }
                    sendingUser = findUserSocket(i);
                    cout << "after receive: " << receiveBuffer << endl;
                    //proof if logged in or not
                    if(sendingUser=="")							
                        login();                         
                    else{            
                        //following                            
                        if(receiveBuffer[0] ==  'f' && receiveBuffer[1] == ' ')
                            following();
                        //messages
                        else{
                            message m (sendingUser,receiveBuffer, timestamp() );
							output<< sendingUser <<"wrote:"<<m.getText() <<"\n";
                            cout << m.getName() << " wrote " << m.getText() << endl;
                            messages.push_back(m);
                            pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
                            ret = followers.equal_range(sendingUser);
                            memcpy(&sendBuffer, receiveBuffer, sizeof(receiveBuffer));
                            for(followers_it=ret.first; followers_it != ret.second;++followers_it){
                                rc = send(users[(*followers_it).second], sendBuffer, sizeof(sendBuffer), 0);
                            }
                        }
                    }                      
                }  
                if(closeConnection){
#ifdef WIN32
                    closesocket(i);
#else
                    close(i);
#endif
                    FD_CLR(i,&fdSet);
                    users[findUserSocket(i)] = -1;
                    if(i == maxSocket){
                        while(FD_ISSET(maxSocket, &fdSet)==false)
                            maxSocket--;
                    }
                }
            }                
        }           
    }
}

void server::login(){
    string text = "User logged in";
    if(!findUser(receiveBuffer)){
        //add user to map if not exist
        users.insert(pair<string, int>(receiveBuffer, i));
    }
    else{
        if(users[receiveBuffer] == -1)
            users.find(receiveBuffer)->second=i;
        else{
            text = "Already logged in";
        }
    }
    cout << receiveBuffer << text << endl;
    memcpy(&sendBuffer, text.c_str(), sizeof(text));
    rc = send(i, sendBuffer, sizeof(sendBuffer), 0);
}

void server::following(){
    cout << "rb befor cut" << receiveBuffer << "!" << endl;
    memmove(receiveBuffer, receiveBuffer+2, sizeof(receiveBuffer)-2);
    cout << "rb after cut" << receiveBuffer << "!" << endl;                                 
    if(findUser(receiveBuffer)){
        pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
        ret = followers.equal_range(receiveBuffer);                   
        for(followers_it=ret.first; followers_it != ret.second;++followers_it){
            if((*followers_it).second == sendingUser){
                memcpy(&sendBuffer, "You are already following ", sizeof("You are already following "));
                strcat(sendBuffer,receiveBuffer);
                rc = send(i, sendBuffer, sizeof(sendBuffer), 0);
                return;
            }
        }
        followers.insert(pair<string,string>(receiveBuffer,sendingUser));
        memcpy(&sendBuffer, "You now follow ", sizeof("You now follow "));
        strcat(sendBuffer,receiveBuffer);
        rc = send(i, sendBuffer, sizeof(sendBuffer), 0);
    }
    else{
        memcpy(&sendBuffer, "User does not exist", sizeof("User does not exist"));                                  
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
        //set new Socket to 1 in fdSet[Array]
        FD_SET(newSocket, &fdSet);		
        if(newSocket > maxSocket){
            maxSocket=newSocket;
        }
    }while(newSocket != -1);
}

void server::bindServer(char* address, int port){
    memset(&addr, 0, sizeof(SOCKADDR_IN));
   
    addr.sin_family=AF_INET;												
    addr.sin_port=htons(5000);												//Port 5000 in use
    //addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    addr.sin_addr.s_addr = ADDR_ANY;
    //addr.sin_addr.s_addr=gethostbyname(address);

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

void server::printFollowers(string username){
    cout << "PrintFollowser:\n-------------------------\n";
    for(followers_it = followers.begin(); followers_it != followers.end(); followers_it++){
        cout << (*followers_it).first << " followed by " << (*followers_it).second << endl;
    }
    cout << "-------------------------\n";
}
void server::printMessages(){
    cout << "PrintMessages:\n-------------------------\n";
    for(messages_it = messages.begin(); messages_it != messages.end(); messages_it++){
        cout << (*messages_it).getText() << " from " << (*messages_it).getName() << endl; 
    }
    cout << "-------------------------\n";
}
void server::printTweeters(){
    cout << "PrintTweeters:\n-------------------------\n";
    for(users_it = users.begin();users_it!=users.end();users_it++){
        cout << (*users_it).first << " Socketnumber: " << (*users_it).second << endl;
    }
    cout << "-------------------------\n";
}
