#include <iostream>
#include "SimpleQueue.h"

using std::cout; using std::cin;
using std::string;

int main()
{   
    SimpleQueue q = SimpleQueue("client");

    string cmd;
    cout << "Hello World!\n";
    
    while (cmd != "stop") {
        cout << "Please enter command\n";
        //std::cin >> cmd;
        getline(cin, cmd);
        q.sendMsg(cmd);
        cout << "Wait For Reply\n";
        string resp = q.waitMsg();

        cout << "Message received : " << resp << "\n\n\n";

    }

}
