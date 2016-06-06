#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

#include "ConnOb.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"

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

std::vector<std::thread> threads;

int main() {
	std::string s;
	int error = 0;
	int numberOfThreads = 10;

	// init server and get ListenSocket
	SOB listenSOB = serverInit();
	error = listenSOB.errorcode;

	for (int i = 0; i < numberOfThreads; i++) {
		threads.push_back(connectNow(listenSOB.socket));
	}
	for (int i = 0; i < numberOfThreads; i++) {
		threads[i].join();
	}

	std::cout << "DONE" << std::endl;;

	std::cin >> s;

	return error;
}