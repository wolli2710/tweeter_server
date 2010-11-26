#ifndef SERVER_H
#define SERVER_H

#define BACKLOG 100
#define BUFFER_SIZE 300
#define WIN32 
#define PORT 5000

#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
//#pragma comment(lib, "ptheadVC1.lib")
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#include <errno.h>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

#include "message.h"
#include "timestamp.h"

using namespace std;

class server
{
    public:
        server();
        virtual ~server();
        void bindServer(char* address, int port);
        void listenServer();
        void closeServer();
        void run();
        int createSocket();
        void startWinSock();
        void error(char* string);
		string findUserSocket(int socketNr);
		bool findUser(string username);
    protected:
    private:
        map<string, int> users;
        map<string, int>::iterator users_it;
        multimap<string, string> followers;
        multimap<string, string>::iterator followers_it;
		vector<message> messages;
		vector<message>::iterator messages_it;
        struct timeval timeout;

        SOCKADDR_IN addr;
        FD_SET fdSet, workingSet;
        int listenSocket, maxSocket,newSocket, i, on;
		bool closeConnection;
		char receiveBuffer[BUFFER_SIZE];
		char sendBuffer[BUFFER_SIZE];
        int rc, descriptor_ready;
};

#endif // SERVER_H
