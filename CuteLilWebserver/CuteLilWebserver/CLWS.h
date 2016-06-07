#include <iostream>
#include <string>
#include <map> 
#include "ClientConnection.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"

std::map<ClientConnection*, ClientConnection> connections;
int error = 0;

SOB initServer() {

	WSADATA wsaData;
	SOB returnData;
	returnData.errorcode = 0;
	returnData.socket = INVALID_SOCKET;

	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		returnData.errorcode = 1;
		return returnData;
	}

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof (hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		returnData.errorcode = 1;
		return returnData;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	// Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		returnData.errorcode = 1;
		return returnData;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		returnData.errorcode = 1;
		return returnData;
	}
	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		returnData.errorcode = 1;
		return returnData;
	}

	returnData.socket = ListenSocket;
	return returnData;
}

void removeConnection(ClientConnection cC) {
	connections.erase(&cC);
}

void removeAllConnections() {
	for (std::map<ClientConnection*, ClientConnection>::iterator iter = connections.begin(); iter != connections.end(); ++iter) {
		iter->second.killMe();
	}
	connections.clear();
}

void userInOut() {
	std::string s;

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	std::cout << "----------------------------" << std::endl;
	if (error == 0)	std::cout << "------Lil WS Started--------" << std::endl;
	std::cout << "----Enter \"stop\" to quit----" << std::endl;
	std::cout << "----------------------------\n" << std::endl;
	for (;;) {
		std::cin >> s;
		if (s == "stop") {
			break;
		}
	}
	removeAllConnections();

	WSACleanup();
	exit(0);
}

int provideClientSockets(SOCKET &ListenSocket) {

	for (;;) {
		SOCKET ClientSocket = INVALID_SOCKET;
		// accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();

			return 1;
		}
		// start new client socket object
		ClientConnection cC =  ClientConnection(ClientSocket);

		// store connection
		connections.insert(std::pair<ClientConnection*, ClientConnection>(&cC, cC));
	}
	return 0;
}

int main() {

	int numberOfThreads = 10;

	std::thread inputThread(userInOut);

	// init server and get ListenSocket
	SOB listenSOB = initServer();
	error = listenSOB.errorcode;
	if (error != 0) {
		printf("server initialization faild\n");
		printf("restart program\n");

		// wait for termiation - user input "stop"
		for (;;) {}
	}
	else {
		provideClientSockets(listenSOB.socket);
	}

	return error;
}