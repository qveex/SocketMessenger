#include "Server.h"

Server::Server()
{
	std::string dateAndTime = getCurrentTime();
	std::cout << dateAndTime + " - Запуск\n";
	outputToFile("../Dates.txt", dateAndTime);

	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Ошибка" << std::endl;
		exit(-1);
	}

	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//addr.sin_addr.s_addr = inet_addr("192.168.1.64");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;
	
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);

	initMessages();
	initUsers();
}

// работает в UI потоке
// обрабатывает входящие запросы на сервер
// при получении запроса запускает clientHandler
int Server::run()
{
	int sizeofaddr = sizeof(addr);
	User newUser;

	while (true) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
		if (newConnection == 0) {
			std::cout << "Ошибка #2\n";
			return -2;
		}
		else {
			connections.insert(std::make_pair(newConnection, newUser));
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StaticThreadStart, (this), NULL, NULL);
		}
	}
	return 0;
}

// каждый вызов clientHandler - новый поток
// вызов происходит после подключения каждого нового
// пользователя к серверу, все потоки "хранятся" в connections
// каждый поток участвует в бесконечном цикле, пока пользователь
// не заврешит работу клиентской программы
// заврешается в случае получения от клиента SOCKET_ERROR
// действие для каждого полученного кода описано в файле Client.cpp (см. другой проект)
void Server::clientHandler(SOCKET conn)
{
	int action;
	std::string msg;

	// ожидание авторизации или регистрации пользователя
	if (waitingForLoginLoop(conn) == false) return;

	// ожидание действий от клиента
	while (true) {
		if (recv(conn, (char*)&action, sizeof(int), NULL) == SOCKET_ERROR) {
			std::cout << getCurrentTime() << " - " << connections.at(conn).username << " Отключился\n";
			connections.erase(conn);
			closesocket(conn);
			return;
		}
		else {
			switch (action) {
			case(-1):
				connections.at(conn).waiting = !connections.at(conn).waiting;
				break;
			case(1):
				sendStr(readFromFile(), conn);
				break;
			case(2):
				chat(conn);
				break;
			case(3):
				removeMessage(conn);
				break;
			case(4):
				editMessage(conn);
				break;
			case(5):
				messages.sort();
				loadMessages();
				break;
			case(6):
				messages.clear();
				loadMessages();
				break;
			case(7):
				msg = "";
				for (const auto& c : connections)
					msg += "  " + c.second.username + "\n";
				sendStr(msg, conn);
				break;
			}
		}
	}
}

// load - загрузка сообщений в файл
//        из списка messages, вызывается после изменений
//        внесенных в список
// init - при запуске программы загружает старые сообщения
//        из соответствующего файла
void Server::loadMessages()
{
	std::fstream file;
	file.open("../DataBase.txt", std::fstream::out);
	if (file) {
		for (const auto& e : messages)
			file << e.date << "\n" << e.username << ": " << e.message << "\n\n";
	}
	file.close();
}
void Server::initMessages()
{
	std::smatch res;
	std::fstream file;
	std::string line;
	std::regex reg_msg("(^.{1,}): (.{1,})");
	std::regex reg_date("^(0[1-9]|[1-2][0-9]|3[0 - 1])-([0][1-9]|[1][0-2])-([0-9]{4}) (0[0-9]|1[0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])\0");
	Message msg;
	file.open("../DataBase.txt", std::fstream::in);
	if (file) {
		while (std::getline(file, line)) {
			if (std::regex_search(line, res, reg_date))
				msg.date = res[0].str();

			if (std::regex_search(line, res, reg_msg)) {
				msg.username = res[1].str();
				msg.message = res[2].str();
				messages.push_back(msg);
			}
		}
	}
	file.close();
}

// аналогично initMessages, но для списка пользователей (см. выше)
void Server::initUsers()
{
	std::smatch res;
	std::fstream file;
	std::string line;
	std::regex reg_user("(username = )(.{1,}) (hash = )(.{6})");
	User user;
	file.open("../Users.txt", std::fstream::in);
	if (file) {
		while (std::getline(file, line)) {
			if (std::regex_search(line, res, reg_user)) {
				user.username = res[2].str();
				user.passwordHash = res[4].str();
				users.insert(make_pair(user.username, user));
			}
		}
	}
	file.close();
}

// logIn  - получает от пользователя его имя и хеш от его пароля
//		    если пришедшие данные совпадают с данными на сервер,
//          отвечает good, иначе - bad
// signUp - получает имя и хеш, если такое имя уже зарезервировано, отвечает bad,
//          в обратно, good и добавляет пользователя в файл.
bool Server::logIn(SOCKET conn)
{
	User user;

	int good = 1;
	int bad = -1;

	user.username = recvStr(conn);
	user.passwordHash = recvStr(conn);

	auto search = users.find(user.username);
	bool online = false;
	for (const auto& el : connections)
		if (el.second.username == user.username)
			online = true;

	if (search != users.end()) {
		if (search->second.passwordHash == user.passwordHash && online == false) {
			send(conn, (char*)&good, sizeof(int), NULL);
			connections.at(conn) = user;
			std::cout << getCurrentTime() << " - " << user.username << " Подключился\n";
			return true;
		}
		else
			send(conn, (char*)&bad, sizeof(int), NULL);
	}
	else
		send(conn, (char*)&bad, sizeof(int), NULL);
	return false;
}
bool Server::signUp(SOCKET conn)
{
	std::string str;
	User newUser;

	int good = 1;
	int bad = -1;

	newUser.username = recvStr(conn);
	newUser.passwordHash = recvStr(conn);

	auto search = users.find(newUser.username);
	if (search == users.end()) {
		send(conn, (char*)&good, sizeof(int), NULL);
		std::cout << getCurrentTime() << " - " << newUser.username << " Зарегистрировался\n";
		str = "username = " + newUser.username + " " + "hash = " + newUser.passwordHash + "\n";
		users.insert(make_pair(newUser.username, newUser));
		connections.at(conn) = newUser;
		outputToFile("../Users.txt", str);
	}
	else
		send(conn, (char*)&bad, sizeof(int), NULL);
	return search == users.end();
}

// запускается при открытии потока в clientHandler
// ожидает корректных данных о пользователе при авторизации в систему
// или при регистрации нового аккаунта, осуществляется до тех пор,
// пока logIn или signUp не вернут true и не отправят пользователю good ответ
bool Server::waitingForLoginLoop(SOCKET conn)
{
	int action;
	while (true) {
		if (recv(conn, (char*)&action, sizeof(int), NULL) == SOCKET_ERROR) {
			std::cout << getCurrentTime() << " - " << connections.at(conn).username << " Отключился\n";
			connections.erase(conn);
			return false;
		}
		else {
			if (action == 101) {
				if (logIn(conn))
					return true;
			}
			else if (action == 102) {
				if (signUp(conn))
					return true;
			}
		}
	}
}

// вызывается из clientHandler, получает от пользователя строку
// разбивает ее на необходимые части и записывает в файл
// рассылает сообщение остальным участникам чата 
// (ТОЛЬКО ЧАТА, за это отвечает параметр USER.waiting)
void Server::chat(SOCKET conn)
{
	std::smatch res;
	std::regex reg_msg("(^.{1,}): (.{1,})");
	std::regex reg_date("^(0[1-9]|[1-2][0-9]|3[0 - 1])-([0][1-9]|[1][0-2])-([0-9]{4}) (0[0-9]|1[0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])\0");
	Message MSG;
	std::string msg;

	msg = recvStr(conn);
	outputToFile("../DataBase.txt", msg);
	if (std::regex_search(msg, res, reg_date))
		MSG.date = res[0].str();
	if (std::regex_search(msg, res, reg_msg)) {
		MSG.username = res[1].str();
		MSG.message = res[2].str();
	}
	messages.push_back(MSG);

	for (const auto& c : connections) {
		if (c.first != conn && c.second.username != "unknown" && c.second.waiting == true) {
			sendStr(msg, c.first);
		}
	}
}

// находит необходимое сообщение в списке всех сообщений
// удаляет из списка, отвечает good, если такого сообщения нет - bad
void Server::removeMessage(SOCKET conn)
{
	int good = 1;
	int bad = -1;

	sendStr(messagesOfUser(conn), conn);
	std:: string msg = recvStr(conn);
	if (msg == "") return;
	for (std::list<Message>::iterator m = messages.begin(); m != messages.end(); ++m) {
		if (m->username == connections.at(conn).username && m->message == msg) {
			messages.erase(m);
			send(conn, (char*)&good, sizeof(int), NULL);
			loadMessages();
			return;
		}
	}
	send(conn, (char*)&bad, sizeof(int), NULL);
}

// аналогично removeMessage, но в случае положительного результат,
// создает и вставляет новое сообщение на место старого (с измененным текстом)
void Server::editMessage(SOCKET conn)
{
	int good = 1;
	int bad = -1;

	sendStr(messagesOfUser(conn), conn);
	std::string msg = recvStr(conn);
	if (msg == "") return;
	std::string newMsg = recvStr(conn);
	for (std::list<Message>::iterator m = messages.begin(); m != messages.end(); ++m) {
		if (m->username == connections.at(conn).username && m->message == msg) {
			Message MSG = *m;
			MSG.message = newMsg;
			auto ptr = messages.erase(m);
			messages.insert(ptr, MSG);
			send(conn, (char*)&good, sizeof(int), NULL);
			loadMessages();
			return;
		}
	}
	send(conn, (char*)&bad, sizeof(int), NULL);
}

// первичная единица для большинства записей в файл
// path - название файла в корне проект
// str  - строка, котороую необходимо внести в файл path
// все новые записи не удаляют старые, запись происходит в конец файла
void Server::outputToFile(std::string path, std::string str)
{
	std::fstream file;
	file.open(path, std::fstream::in | std::fstream::out | std::fstream::ate);
	if (file) {
		file << str << "\n";
	}
	file.close();
}

// считывает сообщения и базы данных истории сообщений
// формирует одну большую строку, возвращает ее
std::string Server::readFromFile()
{
	std::fstream file;
	std::string line;
	std::string result;
	file.open("../DataBase.txt", std::fstream::in);
	if (file) {
		while (std::getline(file, line)) {
			result += line + "\n";
		}
	}
	file.close();
	return result;
}

// возвращает одну большую строку,
// включающую в себя только сообщения от конкретного пользователя
std::string Server::messagesOfUser(SOCKET conn) 
{
	std::string msg;
	for (const auto& m : messages)
		if (m.username == connections.at(conn).username)
			msg += m.date + "\n" + m.username + ": " + m.message + "\n\n";
	return msg;
}

// метод для запуска нового потока для нашего сервера
void Server::StaticThreadStart(Server* s)
{
	s->clientHandler(s->newConnection);
}

// берет текущее время, приводит к заданному формату, возвращает в виде строки
std::string Server::getCurrentTime()
{
	char buffer[80];
	const char* format = "%d-%m-%Y %H:%M:%S";
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);
	strftime(buffer, 80, format, timeinfo);
	return buffer;
}

// первичные единицы для общения client - server
// sendStr - отправляет строку msg текущему соединению conn
// recvStr - принимает сообщения от соединения conn, возвращает в виде строки
//			 в случае полученя от пользователя ошибки
//			 (пользователь нажал крестик по ходу работы программы)
//			 закрывает соединение с ним и вдальнейшем удаляет из списка онлайн пользователей 
void Server::sendStr(std::string msg, SOCKET conn) {
	int msg_size = msg.size() + 1;
	send(conn, (char*)&msg_size, sizeof(int), NULL);
	send(conn, msg.c_str(), msg_size, NULL);
}
std::string Server::recvStr(SOCKET conn) {
	int msg_size;
	if (recv(conn, (char*)&msg_size, sizeof(int), NULL) == SOCKET_ERROR) {
		//std::cout << getCurrentTime() << " - " << connections.at(conn).username << " Îòêëþ÷èëñÿ\n";
		//connections.erase(conn);
		closesocket(conn);
		return "";
	}
	char* msg = new char[msg_size];
	msg[msg_size - 1] = '\0';

	recv(conn, msg, msg_size, NULL);
	return msg;
}
