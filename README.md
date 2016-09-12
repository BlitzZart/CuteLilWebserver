# README #

It's a very simple concurrent web server implementation with multi threading.

It fetches for client sockets.
	> creates ClientConnections if a ClientSocket is valid
	> each ClientConnection starts a new thread and handles the communication and responds.