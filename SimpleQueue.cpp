#pragma comment(lib,"Ws2_32.lib")
#include "SimpleQueue.h"
#include <iostream>

SimpleQueue::SimpleQueue() : SimpleQueue("find") {}

SimpleQueue::SimpleQueue(std::string typeAsked) {
    //-----------------------------------------
    // Declare and initialize variables
    WSADATA wsaData = { 0 };
    type = typeAsked;
    // Init sock (listener) var
    socket = { 0 };
    connected_socket = { 0 };
    socket.hEvent = WSACreateEvent();

    // Initialize Winsock
    int iResult = 0;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        throw std::runtime_error("Socket WSAStartup failed: " + std::to_string(iResult));
    }

    socket.s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket.s == INVALID_SOCKET) {
        int errCode = WSAGetLastError();
        WSACleanup();
        throw std::runtime_error("socket socket function failed with error = " + std::to_string(errCode));
    }

    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
    service.sin_family = AF_INET;
    InetPton(AF_INET, _T("127.0.0.1"), &service.sin_addr.s_addr);
    service.sin_port = htons(PORT);

    if (typeAsked == "server") initSrv();
    else if (typeAsked == "client") initCli();
    else if (typeAsked == "find") {
        try { initSrv(); }
        catch (...) { initCli(); }
    }
    else throw std::runtime_error("Type of socket not in (server|client|find)");
};

void SimpleQueue::initSrv() {
    int iResult = bind(socket.s, (SOCKADDR*)&service, sizeof(service));
    if (iResult == SOCKET_ERROR) {
        int errCode = WSAGetLastError();
        close();
        throw std::runtime_error("bind socket function failed with error  " + std::to_string(errCode()));
        
   }
    //----------------------
    // Listen for incoming connection requests 
    // on the created socket
    if (listen(socket.s, 1) == SOCKET_ERROR)
        throw std::runtime_error("listen function failed with error: " + std::to_string(WSAGetLastError()));
    WSAEventSelect(socket.s, socket.hEvent, FD_ACCEPT);
    type = "server";
}

bool SimpleQueue::acceptClient(int wait) {
    DWORD dwEvent = WSAWaitForMultipleEvents(1, &socket.hEvent, FALSE, wait, FALSE);
    WSAResetEvent(socket.hEvent);
    if (dwEvent == WSA_WAIT_FAILED)
        throw std::runtime_error("WSAWaitForMultipleEvents failed: " + std::to_string(WSAGetLastError()));
    if (dwEvent == WSA_WAIT_TIMEOUT) return FALSE;
    
    connected_socket.s = accept(socket.s, NULL, NULL);
    if (connected_socket.s == INVALID_SOCKET)
        throw std::runtime_error("accept failed: " + std::to_string(WSAGetLastError()));
    else {
        connected_socket.hEvent = WSACreateEvent();
        WSAEventSelect(connected_socket.s, connected_socket.hEvent, FD_READ);
        return TRUE;
    }
}
bool SimpleQueue::waitClient() { return acceptClient(WSA_INFINITE); }

void SimpleQueue::initCli() {
    int iResult = connect(socket.s, (SOCKADDR*)&service, sizeof(service));
    if (iResult == SOCKET_ERROR)
        throw std::runtime_error("connect function failed with error: " + std::to_string(WSAGetLastError()));

    connected_socket = socket;
    WSAEventSelect(connected_socket.s, connected_socket.hEvent, FD_READ);
    type = "client";
}

void SimpleQueue::sendMsg(std::string msg) {
    int iResult = send(connected_socket.s, &msg[0], (int)strlen(&msg[0]), 0);
    if (iResult == SOCKET_ERROR) {
        int errCode = WSAGetLastError();
        close();
        throw std::runtime_error("sendMsg function failed with error: " + std::to_string(errCode));
    }
}

std::string SimpleQueue::rcvMsg(int wait) {
    DWORD dwEvent = WSAWaitForMultipleEvents(1, &connected_socket.hEvent, FALSE, wait, FALSE);
    if (dwEvent == WSA_WAIT_FAILED) {
        throw std::runtime_error("WSAWaitForMultipleEvents function failed with error: " + std::to_string(WSAGetLastError()));
    }
    if (dwEvent == WSA_WAIT_TIMEOUT) return "";
    WSAResetEvent(connected_socket.hEvent);
    CHAR recvbuff[512];
    memset(recvbuff, 0, 512);
    int iResult = recv(connected_socket.s, recvbuff, 512, 0);
    if (iResult == SOCKET_ERROR) {
        int errCode = WSAGetLastError();
        close();
        throw std::runtime_error("recv function failed with error: " + std::to_string(errCode));
    }
    return (std::string)recvbuff;
}

std::string SimpleQueue::waitMsg() { return rcvMsg(WSA_INFINITE); }


void SimpleQueue::close() {
    // Close the socket to release the resources associated
    // Normally an application calls shutdown() before closesocket 
    //   to  disables sends or receives on a socket first
    // This isn't needed in this simple sample
    shutdown(socket.s, SD_BOTH);
    int iResult = closesocket(socket.s);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"closesocket failed with error = %d\n", WSAGetLastError());
        WSACleanup();
    }
}
