#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <iomanip>
#include <string>
#pragma warning(disable: 4996)

class Client
{
public:
	Client();

	std::string user_name;
	std::string password;

	bool action(int cur);
	void login();
	void menu();

	static int input(int num);

private:
	SOCKET Connection;
	SOCKADDR_IN addr;

	void clientHandler();
	std::string getCurrentTime();
	void chat();
	void removeMessage();
	void editMessage();

	static void StaticThreadStart(Client* client);
	std::string myHash(std::string password);

	void sendStr(std::string str);
	std::string recvStr();
};
