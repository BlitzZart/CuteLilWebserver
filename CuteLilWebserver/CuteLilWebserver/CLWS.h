#include <iostream>
#include <string>
#include <atomic>
#include <map> 
#include "ClientConnection.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"

std::map<ClientConnection*, ClientConnection> connections;
int error = 0;

SOCKET initServer() {
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		error = 1;
		return NULL;
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
		error = 1;
		return NULL;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	// Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		error = 1;
		return NULL;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		error = 1;
		return NULL;
	}
	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		error = 1;
		return NULL;
	}

	error = 0;
	return ListenSocket;
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
	// wait for server initialisation
	std::this_thread::sleep_for(std::chrono::milliseconds(250));

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
			error = 1;
			return error;
		}
		// start new client socket object
		ClientConnection cC =  ClientConnection(ClientSocket);

		// store connection
		connections.insert(std::pair<ClientConnection*, ClientConnection>(&cC, cC));
	}
	error = 0;
	return error;
}

void serverInitFailedMsg() {
	printf("server initialization faild\n");
	printf("restart program\n");

	// wait for termiation - user input "stop"
	for (;;) {}
}

void clientSocketFailedMsg() {
	printf("client socket faild\n");
	printf("restart program\n");

	// wait for termiation (user input "stop" or closing window)
	for (;;) {}
}

int main() {
	std::thread inputThread(userInOut);

	// init server and get ListenSocket
	SOCKET ListenSocket = initServer();

	if (error != 0) {
		serverInitFailedMsg();
	}
	else {
		// server initialized
		if (provideClientSockets(ListenSocket) != 0) {
			clientSocketFailedMsg();
		}
	}

	return error;
}