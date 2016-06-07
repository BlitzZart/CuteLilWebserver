#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <map> 

#include "ClientConnection.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"

std::map<std::string, ClientConnection> connections;

SOB serverInit() {

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

#define DEFAULT_PORT "27015"

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

void inputListener() {
	std::string s;

	std::cout << "Server Running" << std::endl;
	std::cout << "Enter stop to quit" << std::endl;

	std::cin >> s;
	
	WSACleanup();
	exit(0);
}

int getClientSocket(SOCKET &ListenSocket) {

	for (;;) {
		SOCKET ClientSocket = INVALID_SOCKET;
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();

			return 1;
		}
		// start new client socket thread
		ClientConnection cC = ClientConnection(ClientSocket);
	}
	return 0;
}

int main() {
	std::string s;
	int error = 0;
	int numberOfThreads = 10;

	// init server and get ListenSocket
	SOB listenSOB = serverInit();
	error = listenSOB.errorcode;

	std::thread inputThread(inputListener);

	getClientSocket(listenSOB.socket);

	return error;
}