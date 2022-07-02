#include <iostream>
#include "SimpleQueue.h"
#include <chrono>
#include <thread>


int main()
{
    SimpleQueue q = SimpleQueue("server");
    
    wprintf(L"Hello World!\n");
    std::cout << "waiting for client";
    q.waitClient();
    std::cout << "\nClient connected, waiting for receive\n";
    std::string msg;
    while (msg != "stop") {
        msg = q.waitMsg();
        std::cout << "Received : " + msg;

        std::string newStr = "OkSrv:" + msg;
        std::cout << "\nSending msg\n";
        q.sendMsg(newStr);
        std::cout << "\nMsg sent\n";
    }

    std::cout << "Closing in 3 sec...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}

