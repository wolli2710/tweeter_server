//Multimediale Netzwerke und IT Sicherheit
//Uebungsprojekt
//Andreas Stallinger / Wolfgang Vogl

#include "server.h"

server::server()
{

}

server::~server()
{
    
}

/*********************************************************
* Function to start the Windows specific Sockets         *
**********************************************************/
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

/*********************************************************
* run method contains the main functionality of			 *
* the tweeter server								     *
**********************************************************/ 
void server::run(){
    output.open("output.txt");
    shutDown=false;
    on = 1;
    startWinSock();
	
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    
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
	//duration of the timeout in select
    timeout.tv_sec = 20;
    timeout.tv_usec = 0;

	
    bindServer("127.0.0.1", PORT);
	
    listenServer();

	//initialize the fdSet
    FD_ZERO(&fdSet);
    maxSocket = listenSocket;
    FD_SET(listenSocket, &fdSet);
    cout<<"Server started\n";
	//Main loop of the server to handle the clients
    while(!shutDown){
		//Prints out all messages written by the users (testing method on the server)
        printMessages();
		//Prints out all users by name (testing method on the server)
        printTweeters();
		//copy fdSet in workingSet
        memcpy(&workingSet, &fdSet, sizeof(fdSet));
        cout << "waiting for anything" << endl;
		//Call select and wait 20 seconds
        rc = select(maxSocket+1, &workingSet, NULL, NULL, &timeout);
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


/*********************************************************
* Function iterateThrowSockets   *
**********************************************************/
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
                        //following conditions
                        if(receiveBuffer[0] == 'f' && receiveBuffer[1] == ' ')
                            following();
                        
                        else{
							//call the class timestamp and write username, message and time into a message instance
							//write the message into logfile and pushback into the messages vector
                            timestamp t;
                            message m (sendingUser,receiveBuffer, t);
                            output<< m.convertToString() <<"\n";
                            messages.push_back(m);
							
							//send the message to all online followers
                            pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
                            ret = followers.equal_range(sendingUser);
                            string mes = m.convertToString();
                            memcpy(&sendBuffer, mes.c_str(), mes.length());
                            for(followers_it=ret.first; followers_it != ret.second;++followers_it){
                                rc = send(users[(*followers_it).second], sendBuffer, sizeof(sendBuffer), 0);
                            }
                        }
                    }
                }
				//if the user close the connection his socket would be closed
                if(closeConnection){
#ifdef WIN32
                    closesocket(i);
#else
                    close(i);
#endif
					//socketnumber is set to 0 in fdSet
                    FD_CLR(i,&fdSet);
					//the users socket is set to -1 as a inactive user in the users map
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

/*********************************************************
* Login function handles known and unknown users which   *
* currently have no Socket								 *
**********************************************************/
void server::login(){
    string text = "User logged in";
    if(!findUser(receiveBuffer)){
        //add username and socketnumber to the users map if 
		//not exists (for new users)
        users.insert(pair<string, int>(receiveBuffer, i));
    }
    else{
		//handle users which don`t login for the first time
		if(users[receiveBuffer] == -1){
			//give a user a socketnumber in the map users
            users.find(receiveBuffer)->second=i;
			////////////////////////////////
			//send all messages of the tweeters which the user, who login is following
			memset(sendBuffer,0,BUFFER_SIZE); 

			for(messages_it = messages.begin(); messages_it != messages.end(); messages_it++){
				if(follows.find(receiveBuffer)->second == messages_it->getName()){
					string oldMessage = messages_it->getText();
					memcpy(&sendBuffer, oldMessage.c_str(), oldMessage.length());
					cout<< "****************************************" << sendBuffer << endl;
					send(users[receiveBuffer], sendBuffer, sizeof(sendBuffer), 0);
					cout<< "******/////////////////////*************" << sendBuffer << endl;
					sendBuffer[0] = '\0';
				}	
			}
			//////////////////////////////////
		}
        else{
            text = "Already logged in";
        }
    }
	//Print out "user logged in" and send confirmation
    cout << receiveBuffer << text << endl;
    memcpy(&sendBuffer, text.c_str(), sizeof(text));
    rc = send(i, sendBuffer, sizeof(sendBuffer), 0);
}

/*********************************************************
* Method to follow other users                           *
* Method verifies user existance, and singularity of     *
* the follower											 *
**********************************************************/
void server::following(){
    cout << "rb befor cut" << receiveBuffer << "!" << endl;
	//cut the identifier for follow "f " from receiveBuffer
    memmove(receiveBuffer, receiveBuffer+2, sizeof(receiveBuffer)-2);
    cout << "rb after cut" << receiveBuffer << "!" << endl;
	//handle users which actually exist
    if(findUser(receiveBuffer)){
        pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
        ret = followers.equal_range(receiveBuffer);
        for(followers_it=ret.first; followers_it != ret.second;++followers_it){
			//handle the case that the user is already following this tweeter
            if((*followers_it).second == sendingUser){
                memcpy(&sendBuffer, "You are already following ", sizeof("You are already following "));
                strcat(sendBuffer,receiveBuffer);
                rc = send(i, sendBuffer, sizeof(sendBuffer), 0);
                return;
            }
        }
		//fill in the user as a follower to the followers multimap and send a confirmation
        followers.insert(pair<string,string>(receiveBuffer,sendingUser));
		follows.insert(pair<string,string>(sendingUser, receiveBuffer));
        memcpy(&sendBuffer, "You now follow ", sizeof("You now follow "));
        strcat(sendBuffer,receiveBuffer);
        rc = send(i, sendBuffer, sizeof(sendBuffer), 0);
    }
    else{
		//send a information that the user not exists
        memcpy(&sendBuffer, "User does not exist", sizeof("User does not exist"));
        rc = send(i, sendBuffer, sizeof(sendBuffer), 0);
       
    }
}

/*********************************************************
* Method listening can handle the socket state of a user *
**********************************************************/
void server::listening(){
    do{
        newSocket = accept(listenSocket, NULL, NULL);
        if(newSocket < 0){
            break;
        }
        cout<<"New client " << newSocket << endl;
        memcpy(&sendBuffer, "Connected with Server...", sizeof("Connected with Server..."));
        send(newSocket, sendBuffer, sizeof(sendBuffer), 0);
        //set new Socket to 1 in fdSet
        FD_SET(newSocket, &fdSet);
        if(newSocket > maxSocket){
            maxSocket=newSocket;
        }
    }while(newSocket != -1);
}


/*********************************************************
* bindServer handles the binding between socket and port *
**********************************************************/
void server::bindServer(char* address, int port){
    memset(&addr, 0, sizeof(SOCKADDR_IN));
   
    addr.sin_family=AF_INET;	//IPv4
    addr.sin_port=htons(5000);	// Port 5000 in use
    addr.sin_addr.s_addr = ADDR_ANY;
   
    int rc;
    rc = bind(listenSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
    if(rc == SOCKET_ERROR){
        error("Binding failed");
        exit(1);
    }
}


/*********************************************************
* listenServer sets the maximum of users/Sockets         *
* that the Server can handle							 *
**********************************************************/
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


/*********************************************************
* finds a User Name out of the users map                *
* Must Iterate over the users map to find the			*
* Socketnumber and return the username					*
**********************************************************/
string server::findUserSocket(int socketNr){
    for(users_it=users.begin(); users_it!=users.end(); users_it++){
        if(users_it->second == socketNr){
            return users_it->first;
        }
    }
    return "";
}

/*********************************************************
* finds a User out of the users map by searching for name*
**********************************************************/
bool server::findUser(string username){
    return users.find(username)!= users.end();
}


/*********************************************************
* Methods to print out Followers, Messages, and Users    *
* Iterating over the whole structure				     *
**********************************************************/
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