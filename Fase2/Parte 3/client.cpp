#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include<limits>

using namespace std;

int main() 
{
    int port;
    bool correctPort = false;
    
    while (!correctPort)
    {
        cout << "Introduce the server Port: \n";
        cin >> port;
        while(cin.fail())
        {   
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(),'\n');
            cout << "------ not an integer------" << endl;
            cin >> port;
        } 
        if(!cin.fail())
        {
            if (port < 1024)
            {
                cout << "Must be a number greater than 1024 " << endl;
            }else
            {
                correctPort = true;
            } 
        }
         
    }  

    // create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        return 1;
    }

    // create a hint structure for the server we're connecting with
    
    string ipAddress = "127.0.0.1"; 
    
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    // Connect to the server on the socket

    int connectReslt = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connectReslt == -1)
    {
        return 1;
    }

    // while loop
    char buf[4096];
    string userInput;

    do
    {
        // enter text
        cout << "> ";
        getline(cin, userInput);

        // send to server 
        int sendReslt = send(sock, userInput.c_str(), userInput.size() + 1, 0);
        if (sendReslt == -1)
        {
            cout << "Couldn't send to server\r\n";
            continue;  
        }
        

        // wait for response
        memset(buf, 0, 4096);
        int bytesRcv = recv(sock, buf, 4096, 0);

        // Display response
        
        cout << "Server> " << string(buf, bytesRcv) << "\r\n"; 

    } while (true);
    
    // Close Socket
    close(sock);

    return 0;
}