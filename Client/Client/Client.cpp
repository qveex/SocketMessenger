#include "Client.h"

Client::Client()
{
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "������" << std::endl;
		exit(-1);
	}

	int sizeofaddr = sizeof(addr);
	//addr.sin_addr.s_addr = inet_addr("127.0.0.1");    // ���������
	addr.sin_addr.s_addr = inet_addr("95.26.233.218");  // ����������
	//addr.sin_addr.s_addr = inet_addr("192.168.1.64"); // ���������
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	Connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		std::cout << "������: �� ������� ������������ � �������.\n";
		exit(-2);
	}
	std::cout << "  ����������!\n";
}


// ��������������� ������ ����� ���
// �������� ����, ��� �������� ���������� ����� (��� ��� ������ �����)
int Client::input(int num)
{
	std::cin >> num;
	while (std::cin.fail() || std::cin.get() != '\n')
	{
		std::cin.clear();
		std::cin.ignore(std::cin.rdbuf()->in_avail());
		std::cout << "������ �����, ��������� �������: ";
		std::cin >> num;
	}
	return num;
}


void Client::menu()
{
	std::cout << "  /------------------------------\\\n";
	std::cout << "  | ������� |      �������       |\n";
	std::cout << "  |------------------------------|\n";
	std::cout << "  |" << std::setw(9) << "1" << "|" << std::setw(20) << "������� ���������" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "2" << "|" << std::setw(20) << "���" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "3" << "|" << std::setw(20) << "������� ���������" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "4" << "|" << std::setw(20) << "�������������" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "5" << "|" << std::setw(20) << "����������" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "6" << "|" << std::setw(20) << "�������� �������" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "7" << "|" << std::setw(20) << "������������ ������" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "8" << "|" << std::setw(20) << "����" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "0" << "|" << std::setw(20) << "�����" << "|" << std::endl;
	std::cout << "  \\------------------------------/\n";
	std::cout << std::endl << std::endl << " �������� ���� ��������: ";
}


// ��������� ����� ��� ��������� ���������
// ������ ����� - ������������ �� ���� ����������� ����
//     ��������� ������������ �� ������ �� UI ������
//     ������ ����� ������������ � ������� �� ����� ���������� ���������
// readMark - �����, ����������, ��� ������������  ����� ������ ����� ���������
//     ��� ������ �� ���� �������� "EXIT" ����� ������������ �����
//	   ������ ����� ����������� �������, �.�. ����� ������������ ������������
//     �� ����������� ��������� ���������� ���������
// id - ��������� ����� ����������� �����, ����� ����� ������������
//     �������� 2 ������, ��� ������ �� ������ chat() ������ �����
//     ��������� �����������, ������� ���������� �������� ���������
void Client::chat() 
{
	DWORD id = 0u;
	const int readMark = -1;
	int act = 2;
	std::string MSG;
	HANDLE handler = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StaticThreadStart, this, NULL, &id);
	send(Connection, (char*)&readMark, sizeof(int), NULL);
	std::cout << "\n\t��� ������ �������� �����\n\n";
	std::cout << "\n-----------------���----------------\n";
	while (true) {
		do
		{
			std::getline(std::cin, MSG);
		} while (MSG == "");
		if (MSG == "�����") {
			TerminateThread(handler, id); // ���������� ����������
			send(Connection, (char*)&readMark, sizeof(int), NULL);
			system("cls");
			std::cout << "\n\t�� ����� �� ����!\n";
			menu();
			return;
		}
		MSG = "\n" + getCurrentTime() + "\n" + user_name + ": " + MSG;
		send(Connection, (char*)&act, sizeof(int), NULL);
		sendStr(MSG);
	}
}


// ����������� � ������������ ��������� ��� ��������
// ���������� �� ������, ���� ����� ��������� � ������������ ����
// �������� ������������ � ������� ��������� �� ������
// � �������� ������, ������������, ������������
void Client::removeMessage() 
{
	int answer;
	std::string req;
	std::string msg = recvStr();
	std::cout << "---------------������� ����� ���������---------------\n\n";
	std::cout << msg;
	std::cout << "---------------������� ����� ���������---------------\n\n";
	std::cout << "������� ���� �� ����� ��������� ��� ��� ��������: ";
	std::getline(std::cin, req);
	sendStr(req);
	recv(Connection, (char*)&answer, sizeof(int), NULL);
	system("cls");
	if (answer == 1)
		std::cout << "\n\t��������� �������\n\n";
	else if (answer == -1)
		std::cout << "\n\t���, ������ ��������� ���\n\n";
	menu();
}


// ����������� � ������������ ������ ��������� � ����� ��� ��� ������
// ���������� �� ������ � ���� ������
// ������ �������� ��/��� � ����������� �� ����
// ���� �� ����� ��������� � ������������,
// ��������� ����������������� ��������� � �����
void Client::editMessage() 
{
	int answer;
	std::string req;
	std::string msg = recvStr();
	std::cout << "---------------������� ����� ���������---------------\n\n";
	std::cout << msg;
	std::cout << "---------------������� ����� ���������---------------\n\n";
	std::cout << "������� ���� �� ����� ��������� ��� ��� ��������������: ";
	std::getline(std::cin, req);
	std::cout << "\n������� ��������� �� ������: ";
	std::getline(std::cin, msg);
	sendStr(req);
	sendStr(msg);
	recv(Connection, (char*)&answer, sizeof(int), NULL);
	system("cls");
	if (answer == 1)
		std::cout << "\n\t��������� ���������������\n\n";
	else if (answer == -1)
		std::cout << "\n\t���, ������ ��������� ���\n\n";
	menu();
}


// ���������� ����������� ������ �� ������
// ������� � �������� 100 ������� ������������ ������ ��/���
// ���� �� ������ ������������� �����, ���� ������������
// 1 - 7 ����� ����� ��� ����������� �����, ��� � ���
// 101 - ������� �� �������� ����������� �������������
// 102 - ������� �� ����������� ������ ������������
// 1   - ������ �� ����� �� ����� ������� ���������
// 2   - ������ �� ���� � ���, ����������� � UI ������ (��. run())
// 3   - ������ �� �������� ������ ��������� �� ������� �� ������� ���������
// 4   - ������ �� ������ ������ ��������� ������
// 5   - ������ �� ���������� ������ ��������� �� ��������
// 6   - ������ �� �������� ������� ���� (�������� ����)
// 7   - ������ ������ ������������� ������
bool Client::action(int cur)
{
	int answer;
	if (cur != 8 && cur != 2) send(Connection, (char*)&cur, sizeof(int), NULL);
	system("cls");
	switch (cur) {
	case(0):
		exit(0);
	case(1):
		system("cls");
		std::cout << "\n---------------�������---------------\n\n";
		std::cout << recvStr();
		std::cout << "---------------�������---------------\n\n";
		menu();
		break;
	case(2):
		chat();
		break;
	case(3):
		removeMessage();
		break;
	case(4):
		editMessage();
		break;
	case(5):
		std::cout << "\n\t������� ��������� �������������!\n\n";
		menu();
		break;
	case(6):
		std::cout << "\n\t������� ��������� �������!\n\n";
		menu();
		break;
	case(7):
		std::cout << "\n--------------������������ ������--------------\n\n";
		std::cout << recvStr() << std::endl;
		std::cout << "--------------������������ ������--------------\n\n";
		menu();
		break;
	case(8):
		menu();
		break;


	case(101): // �����������
		sendStr(user_name);         // �������� �����
		sendStr(myHash(password));  // ��� ������

		if (recv(Connection, (char*)&answer, sizeof(int), NULL) == SOCKET_ERROR) {
			std::cout << "\n������ ��������\n";
			return false;
		}
		else if (answer == -1)
			return false;
		break;


	case(102): // �����������
		sendStr(user_name);
		sendStr(myHash(password));

		recv(Connection, (char*)&answer, sizeof(int), NULL);
		if (answer == -1)
			return false;
		break;
	}
	return true;
}


// ����������� � ����� ������ �
// ������������ ��������� �� �������,
// ��������� �� ������ ������������� � ����
// ����� ������� ��������� �� �����
void Client::clientHandler()
{
	int msg_size = 0;
	while (true) {
		std::cout << recvStr() << std::endl << std::endl;
	}
}


// ����� ������� �����
// �������� � ��������� �������
// ���������� � ���� ������
std::string Client::getCurrentTime()
{
	char buffer[80];
	const char* format = "%d-%m-%Y %H:%M:%S";
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);
	strftime(buffer, 80, format, timeinfo);
	return buffer;
}


// ������ ����� ��� ����� ������������ � �������
// ��������� ������ ��/��� �� �������
void Client::login() 
{
	int act = 1;
	while (true) {
		std::cout << "  1 ---> ����\n";
		std::cout << "  2 ---> �����������\n";
		std::cout << "  0 ---> �����\n\n";
		do
		{
			act = input(act);
			if (act == 0) exit(0);
			if (act != 1 && act != 2)
				std::cout << "������ �����, ��������� �������: ";
		} while (act != 1 && act != 2);

		std::cout << "\n�����: ";
		std::getline(std::cin, user_name);
		std::cout << "������: ";
		std::getline(std::cin, password);
		system("cls");
		if (act == 1) {
			if (action(101))
				break;
			else
				std::cout << "\n������������ ����� ��� ������!!!\n\n";
		}
		else if (act == 2) {
			if (password.size() < 6 && password.find(" "))
				std::cout << "������� �������� ������!\n\n";
			else if (action(102))
				break;
			else
				std::cout << "\n�������... ����� ������������ ��� ����������...\n\n";
		}
	}
	system("cls");
	std::cout << "\t����� ����������, " + user_name + "!!!\n\n";
}


// ���������, �.�. ��� ������� ������ ������
// ��������� ����������� �����
// ��������� this, ��������� ��� ���� �����
// � ������� ��������������, ����������� � �������
void Client::StaticThreadStart(Client* client)
{
	client->clientHandler();
}


// ������� ��� ������� ������
// ������������ ��� ����������� ������
// ��� ������������ �� ������ ��� ���������
std::string Client::myHash(std::string str)
{
	std::string hash = "";
	int size = str.size() - 1;
	int center = str.size() / 2;
	unsigned int temp;

	temp = 0;
	for (int i = 0; i < str.size(); i++)
		temp += str.at(i);
	hash += ((temp % 26) + 65);												// �

	temp = 1;
	for (int i = str.size() / 2; i < str.size(); i++)
		temp += (pow(str.at(i), 3)) + (str.at(0));
	for (int i = 0; i < str.size() / 2; i++)
		temp += (str.at(i)) - str.at((str.size() - 1) / 4);
	temp -= str.at(str.size() / 9) + str.at(str.size() / 4) - pow(str.at(str.size() / 3), 2) + pow(str.at(str.size() / 5), 3);
	hash += ((temp % 26) + 65);												// �

	temp = 0;
	temp += str.at(0) * str.at(str.size() - 1) / str.at(str.size() / 2) + str.at(str.size() / 3) + str.at(str.size() / 4) * str.at(str.size() / 5);
	hash += ((temp % 10) + 48);												// �

	temp = 0;
	temp += str.at(str.size() / 2) + str.at((str.size() - 1) / 3);
	temp += pow(str.at(str.size() - 1), 3) / 2 + str.at(0) + str.at(str.size() / 4);
	temp *= str.at((str.size() * 3) / 4) + str.at((str.size() * 2) / 5);
	temp -= str.at(str.size() - 1) * str.at(str.size() / 5);
	hash += ((temp % 10) + 48);												// �

	temp = 1;
	for (int i = 0; i < str.size(); i++)
		temp *= str.at(i);
	temp -= pow(str.at(0), 3) + (str.at(str.size() - 1)) / (str.at(str.size() / 2) << 2);
	hash += ((temp % 26) + 65);												// �

	temp = 1;
	for (int i = str.size() / 3; i < str.size(); i++)
		temp += (pow(str.at(i), 3));
	temp *= str.at(str.size() / 8) + str.at(str.size() / 4);
	temp -= (pow(str.at(str.size() - 1), 3)) + (str.at(0)) + (pow(str.at((str.size() - 1) / 3), 3));
	temp += pow(str.at(str.size() / 2), 2) + pow(str.at(str.size() / 3), 3);
	hash += ((temp % 26) + 65);												// �

	return hash;															// MISH257
}


// �������� ������� ������ � ��������:
// sendStr ��������� ������,
// recvStr �������� ������ �� �������
void Client::sendStr(std::string msg) 
{
	int msg_size = msg.size() + 1;

	send(Connection, (char*)&msg_size, sizeof(int), NULL);
	send(Connection, msg.c_str(), msg_size, NULL);
}
std::string Client::recvStr() 
{
	int msg_size;
	if (recv(Connection, (char*)&msg_size, sizeof(int), NULL) == SOCKET_ERROR) {
		std::cout << "\n������ ��������\n";
		exit(-1);
	}
	char *msg = new char[msg_size];
	msg[msg_size - 1] = '\0';
	recv(Connection, msg, msg_size, NULL);

	std::string MSG = msg;
	delete[] msg;

	return MSG;
}