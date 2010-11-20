#ifndef SERVER_H
#define SERVER_H

#define BACKLOG

#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#pragma commten(lib, "ptheadVC1.lib");
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#endif
#include <vector>
#include <map>
#include <fstream>

#include "message.h"
#include "timestamp.h"

using namespace std;

class server
{
    public:
        server();
        virtual ~server();
        void bindServer(char* address);
        void listenServer();
        void closeServer();
        void run();
        int createSocket();
        void startWinSock();
        void error(char* string)
    protected:
    private:
        map<string, int> users;
        map<string, int>::iterator users_it;
        multimap<string, string> followers;
        multimap<string, string>::iterator followers_it;
        struct timeval timeout;

        SOCKADDR_IN addr;
        FD_SET fdSet;
        int serverSocket, maxSocket;

};

#endif // SERVER_H
