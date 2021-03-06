#pragma comment(lib,"Ws2_32.lib")
#include "SimpleQueue.h"

using std::string; using std::to_string;
using std::runtime_error;

SimpleQueue::SimpleQueue() : SimpleQueue("find") {}

SimpleQueue::SimpleQueue(string typeAsked) {
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
        throw runtime_error("Socket WSAStartup failed: " + std::to_string(iResult));
    }

    socket.s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket.s == INVALID_SOCKET) {
        int errCode = WSAGetLastError();
        WSACleanup();
        throw runtime_error("socket socket function failed with error = " + to_string(errCode));
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
    else throw runtime_error("Type of socket not in (server|client|find)");
};

void SimpleQueue::initSrv() {
    int iResult = bind(socket.s, (SOCKADDR*)&service, sizeof(service));
    if (iResult == SOCKET_ERROR) {
        int errCode = WSAGetLastError();
        close();
        throw runtime_error("bind socket function failed with error  " + to_string(errCode));

    }
    //----------------------
    // Listen for incoming connection requests 
    // on the created socket
    if (listen(socket.s, 1) == SOCKET_ERROR)
        throw runtime_error("listen function failed with error: " + to_string(WSAGetLastError()));
    WSAEventSelect(socket.s, socket.hEvent, FD_ACCEPT);
    type = "server";
}

bool SimpleQueue::acceptClient(int wait) {
    DWORD dwEvent = WSAWaitForMultipleEvents(1, &socket.hEvent, FALSE, wait, FALSE);
    WSAResetEvent(socket.hEvent);
    if (dwEvent == WSA_WAIT_FAILED)
        throw runtime_error("WSAWaitForMultipleEvents failed: " + to_string(WSAGetLastError()));
    if (dwEvent == WSA_WAIT_TIMEOUT) return FALSE;

    connected_socket.s = accept(socket.s, NULL, NULL);
    if (connected_socket.s == INVALID_SOCKET)
        throw runtime_error("accept failed: " + to_string(WSAGetLastError()));
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
        throw runtime_error("connect function failed with error: " + to_string(WSAGetLastError()));

    connected_socket = socket;
    WSAEventSelect(connected_socket.s, connected_socket.hEvent, FD_READ);
    type = "client";
}

void SimpleQueue::sendMsg(string msg) {
    int msgLen = (int)strlen(&msg[0]);
    std::stringstream sstream;
    sstream << std::setfill('0') << std::setw(2) << std::hex << msgLen;
    msg = sstream.str() + msg;
    msgLen = (int)strlen(&msg[0]);
    int iResult = send(connected_socket.s, &msg[0], msgLen, 0);
    if (iResult == SOCKET_ERROR) {
        int errCode = WSAGetLastError();
        close();
        throw runtime_error("sendMsg function failed with error: " + to_string(errCode));
    }
}

string SimpleQueue::getMsg(int bufflen) {
    CHAR recvbuff[512];
    memset(recvbuff, 0, 512);
    int iResult = recv(connected_socket.s, recvbuff, bufflen, 0);
    if (iResult == SOCKET_ERROR) {
        int errCode = WSAGetLastError();
        close();
        throw runtime_error("recv function failed with error: " + to_string(errCode));
    }
    return (string)recvbuff;

}

string SimpleQueue::rcvMsg(int wait) {
    DWORD dwEvent = WSAWaitForMultipleEvents(1, &connected_socket.hEvent, FALSE, wait, FALSE);
    if (dwEvent == WSA_WAIT_FAILED) {
        throw runtime_error("WSAWaitForMultipleEvents function failed with error: " + to_string(WSAGetLastError()));
    }
    if (dwEvent == WSA_WAIT_TIMEOUT) return "";
    WSAResetEvent(connected_socket.hEvent);
    string msgLenStr = getMsg(2);
    int msgLen = std::strtoll(&msgLenStr[0], 0, 16);
    WSAResetEvent(connected_socket.hEvent);
    return getMsg(msgLen);
}

string SimpleQueue::waitMsg() { return rcvMsg(WSA_INFINITE); }


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
