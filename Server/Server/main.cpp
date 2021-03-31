#include <iostream>
#include "Server.h"


int main(int argc, char* argv[]) {

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	Server server;
	server.run();

	system("pause");
	return 0;
}