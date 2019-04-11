#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

using namespace std;

int main() 
{
    // create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        return 1;
    }

    // create a hint structure for the server we're connecting with
    int port = 5000;
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