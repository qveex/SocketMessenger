#include <iostream>
#include "Client.h"


int main(int argc, char* argv[]) {
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	int action = 0;
	Client client;
    client.login();
    client.menu();
    while (true) {
        do
        {
            action = Client::input(action);
            if (action > 8 && action < 0)
                std::cout << "Ошибка! Повторите ввод: ";
        } while (action > 8 && action < 0);

        client.action(action);
    }

	system("pause");
	return 0;
}