#include "ClientConnection.h"
#include <iostream>
#include <string>

ClientConnection::ClientConnection(SOCKET &ClientSocket) {
		this->ClientSocket = ClientSocket;
		makeThread();
	}
	void ClientConnection::killMe() {
		closeConnection();
	}

	void ClientConnection::makeThread() {
		std::thread t(theThread, this);
		t.join();
	}

	int ClientConnection::checkRequest(char *recvbuf) {
		const int headerLength = 3;
		char *token = NULL;
		char *next_token = NULL;
		char *firstLine = strtok_s(recvbuf, "\n", &next_token);
		std::string chckReq[headerLength];

		token = strtok_s(firstLine, " ", &next_token);
		for (int i = 0; i < headerLength; i++) {
			if (token != NULL) {
				chckReq[i] = token;
				token = strtok_s(NULL, " ", &next_token);
			}
			else {
				chckReq[i] = "";
			}
		}

		for (int i = 0; i < headerLength; i++) {
			int error = strncmp(refReq[i].c_str(), chckReq[i].c_str(), refReq->length());
			if (error != 0)
				return 1; // return with error
		}

		return 0;
	}

	int ClientConnection::communicate() {
#define DEFAULT_BUFLEN 512

		char recvbuf[DEFAULT_BUFLEN];
		int iResult, iSendResult;
		int recvbuflen = DEFAULT_BUFLEN;
		id++;

		// receive until the client shuts down the connection
		do {
			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				// validate and respond to request
				if (checkRequest(recvbuf) == 0) {
					std::string toSend = respondOK + dummySite;
					iSendResult = send(ClientSocket, toSend.c_str(), toSend.length(), 0);
					std::cout << "Responded: " << respondOK;
				}
				else {
					std::string toSend = respondOK;
					iSendResult = send(ClientSocket, toSend.c_str(), toSend.length(), 0);
					toSend = respondNOK;
					iSendResult = send(ClientSocket, toSend.c_str(), toSend.length(), 0);
				}

				if (iSendResult == SOCKET_ERROR) {
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}

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

	int ClientConnection::closeConnection() {
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