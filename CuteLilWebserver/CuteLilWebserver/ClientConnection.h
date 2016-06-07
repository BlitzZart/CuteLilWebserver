#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <iostream>
#include <string>

#include <thread>

struct SOB {
	int errorcode;
	SOCKET socket;
};

static int id = 0;

class ClientConnection {
public:
	ClientConnection(SOCKET &ClientSocket) {
		this->ClientSocket = ClientSocket;
		makeThread();
	}

	void killMe() {
		closeConnection();
	}

private:
	SOCKET ClientSocket;

	void makeThread() {
		std::thread t(theThread, this);
		t.join();
	}

	int communicate() {
#define DEFAULT_BUFLEN 512

		char recvbuf[DEFAULT_BUFLEN];
		int iResult, iSendResult;
		int recvbuflen = DEFAULT_BUFLEN;
		id++;
		std::cout << "Thread # " << id << "\n" << std::endl;

		// Receive until the peer shuts down the connection
		do {
			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				printf("Bytes received: %d\n", iResult);

				// Echo the buffer back to the sender
				iSendResult = send(ClientSocket, recvbuf, iResult, 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}
				printf("Bytes sent: %d\n", iSendResult);

				//TODO this exits the loop
				iResult = -1;
			}
			else if (iResult == 0)
				printf("Connection closing...\n");
			else {
				printf("recv failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}

		} while (iResult > 0);
	}

	int closeConnection() {
		// shutdown the send half of the connection since no more data will be sent
		int iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			printf("shutdown failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

		// cleanup
		closesocket(ClientSocket);

		return 0;
	}

	static int theThread(ClientConnection* me) {
		me->communicate();
		me->closeConnection();
		return 0;
	}
};