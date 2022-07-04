#pragma once
#ifndef SAS_SOCKET_H
#define SAS_SOCKET_H

// link with Ws2_32.lib
#pragma comment(lib,"Ws2_32.lib")
#include <tchar.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string> 
#include <iostream>
#include <sstream>
#include <iomanip>

#define PORT 21612


typedef struct _SOCK_INFO
{
	SOCKET          s;
	WSAEVENT        hEvent;
} SOCK_INFO;

class SimpleQueue {
	SOCK_INFO socket;
	SOCK_INFO connected_socket;
	std::string type;
	sockaddr_in service;

public:
	SimpleQueue(std::string typeAsked);
	SimpleQueue();
	void close();
	void sendMsg(std::string msg);
	std::string rcvMsg(int wait = 0);
	std::string waitMsg();
	bool waitClient();
	bool acceptClient(int wait = 0);

private:
	void initSrv();
	void initCli();
	std::string getMsg(int bufflen);
};


#endif
