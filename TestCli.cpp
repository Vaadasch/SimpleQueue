#include <iostream>
#include "SimpleQueue.h"


int main()
{   
    SimpleQueue q = SimpleQueue("client");

    std::string cmd;
    std::cout << "Hello World!\n";
    
    while (cmd != "stop") {
        std::cout << "Please enter command\n";
        std::cin >> cmd;
        q.sendMsg(cmd);
        std::cout << "Wait For Reply\n";
        std::string resp = q.waitMsg();

        std::cout << "Message received : " << resp << "\n\n\n";

    }

}
