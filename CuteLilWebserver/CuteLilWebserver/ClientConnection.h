#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

static const std::string respondOK = "HTTP/1.1 200 OK";
static const std::string respondNOK = "HTTP/1.1 400 Bad Request"; // TODO
static const std::string dummySite = "<h1>It's a me!</h1>\n<p>The lil webserver.</p>\n<p><em>Yeahhhh!.</em></p>";
static const std::string badRequestSite = "<h1>400!</h1>\n<p>The lil webserver.</p>\n<p><em>B‰‰‰d Request!.</em></p>";
static const std::string refReq[3] = { "GET", "/", "HTTP/1.1"};

struct SOB {
	int errorcode;
	SOCKET socket;
};
static int id = 0;
// starts a new thread for each accepted client,
// handles the communiction and colses the connection afterwards
class ClientConnection {

public:
	ClientConnection(SOCKET &ClientSocket);
	void killMe();

private:
	SOCKET ClientSocket;


	void makeThread();
	int checkRequest(char *recvbuf);
	int communicate();
	int closeConnection();

	static int theThread(ClientConnection* me) {
		me->communicate();
		me->closeConnection();
		return 0;
	}
};