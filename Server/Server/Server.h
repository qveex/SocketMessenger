#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <list>
#include <map>

#pragma warning(disable: 4996)

struct User {
	std::string username = "unknown";
	std::string passwordHash;
	bool waiting = false;
};

struct Message {
	std::string date;
	std::string username;
	std::string message;

	bool operator > (const Message &second) {
		return this->username > second.username;
	}
	bool operator < (const Message& second) {
		return this->username < second.username;
	}
};

class Server
{
public:
	Server();
	int run();

private:
	int currentConnectionsCounter = 0;
	SOCKET newConnection;

	std::map<SOCKET, User> connections;
	SOCKADDR_IN addr;
	SOCKET sListen;

	std::map<std::string, User> users;
	std::list<Message> messages;

	void clientHandler(SOCKET conn);
	void outputToFile(std::string path, std::string str);
	std::string readFromFile();
	std::string messagesOfUser(SOCKET conn);

	void initMessages();
	void loadMessages();
	void initUsers();

	bool logIn(SOCKET conn);
	bool signUp(SOCKET conn);
	bool waitingForLoginLoop(SOCKET conn);
	void chat(SOCKET conn);
	void removeMessage(SOCKET conn);
	void editMessage(SOCKET conn);

	std::string getCurrentTime();
	static void StaticThreadStart(Server* s);

	void sendStr(std::string str, SOCKET conn);
	std::string recvStr(SOCKET conn);
};
