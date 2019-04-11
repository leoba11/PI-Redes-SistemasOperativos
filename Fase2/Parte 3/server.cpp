#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#define PORT 5000

using namespace std;

int main() 
{
	//Create a socket
	int listening = socket(AF_INET, SOCK_STREAM, 0); 

	if (listening == -1) {
		cerr << "Can't create a socket";
		return -1;
	}else
	{
		cout << "Socket created successfully\n" << "Successfull link in port 5000\n" << endl;
	}
	
	//Bind a socket to IP and Port
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(PORT);
	inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
	

	if( bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1 ) 
	{
		cerr << "Can't bind to IP/Port";
		return -2;
	}
	

	// Mark the socket to listening in
	if(listen(listening, 3) == -1)
	{
		cerr << "Can't listen...";
		return -3;
	}

	// Accept a call
	sockaddr_in client;
	socklen_t clientSize = sizeof(client);
	char host[NI_MAXHOST];
	char svc[NI_MAXSERV];

	int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

	if (clientSocket == -1) {
		cerr << "Problem connecting client...";
		return -4;
	}

	// close the listening socket
	close(listening);

	memset(host, 0, NI_MAXHOST);
	memset(svc, 0, NI_MAXSERV);

	int result = getnameinfo((sockaddr*)&client, clientSize, host, NI_MAXHOST, svc, NI_MAXSERV, 0);

	if(result) 
	{
		cout << host << " connected on " << svc << endl; 
	}else
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on " << ntohs(client.sin_port) << endl;
	}
	
	// while recieving display message, echo message
	char buf[4096];
	while(true){
		// clear the buffer
		memset(buf, 0, 4096);
		// Wait for a message
		int bytesRcv = recv(clientSocket, buf, 4096, 0);
		if (bytesRcv == -1) {
			cerr << "There was a connection issue..." << endl;
			break;
		}
		if (bytesRcv == 0) {
			cout << "The client disconnected" << endl;
			break;
		}
		
		// Display message
		cout << "Recieved: " << string(buf, 0, bytesRcv) << endl;

		// Resend Message
		send(clientSocket, buf, bytesRcv + 1, 0);
	}
	
	// close the socket
	close(clientSocket);

	return 0;
}
