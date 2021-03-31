#include "Client.h"

Client::Client()
{
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Ошибка" << std::endl;
		exit(-1);
	}

	int sizeofaddr = sizeof(addr);
	//addr.sin_addr.s_addr = inet_addr("127.0.0.1");    // локалхост
	addr.sin_addr.s_addr = inet_addr("95.26.233.218");  // глобальный
	//addr.sin_addr.s_addr = inet_addr("192.168.1.64"); // локальный
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	Connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		std::cout << "Оишбка: не удалось подключиться к серверу.\n";
		exit(-2);
	}
	std::cout << "  Подключено!\n";
}


// вспомогательный статик метод для
// проверки того, что вводится корректное число (что это вообще число)
int Client::input(int num)
{
	std::cin >> num;
	while (std::cin.fail() || std::cin.get() != '\n')
	{
		std::cin.clear();
		std::cin.ignore(std::cin.rdbuf()->in_avail());
		std::cout << "Ошибка ввода, повторите попытку: ";
		std::cin >> num;
	}
	return num;
}


void Client::menu()
{
	std::cout << "  /------------------------------\\\n";
	std::cout << "  | Клавиша |      Функция       |\n";
	std::cout << "  |------------------------------|\n";
	std::cout << "  |" << std::setw(9) << "1" << "|" << std::setw(20) << "История сообщений" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "2" << "|" << std::setw(20) << "Чат" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "3" << "|" << std::setw(20) << "Удалить сообщение" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "4" << "|" << std::setw(20) << "Редактировать" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "5" << "|" << std::setw(20) << "Сортировка" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "6" << "|" << std::setw(20) << "Очистить историю" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "7" << "|" << std::setw(20) << "Пользователи онлайн" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "8" << "|" << std::setw(20) << "Меню" << "|" << std::endl;
	std::cout << "  |" << std::setw(9) << "0" << "|" << std::setw(20) << "Выход" << "|" << std::endl;
	std::cout << "  \\------------------------------/\n";
	std::cout << std::endl << std::endl << " Выберите ваше действие: ";
}


// запускает поток для обработки сообщений
// второй поток - представляет из себя бесконечный цикл
//     сообщений отправляются на сервер из UI потока
//     второй поток обрабатывает и выводит на экран полученные сообщения
// readMark - метка, означающая, что пользователь  готов читать чужие сообщения
//     при выходе из чата командой "EXIT" метка отправляется снова
//	   вторая метка инвертирует прошлую, т.е. далее пользователь отказывается
//     от дальнейшего прочтения приходящих сообщений
// id - сохраняет номер запущенного поток, всего может существовать
//     максимум 2 потока, при выходе из метода chat() второй поток
//     экстренно завершается, отменяя дальнейшее ожидание сообщений
void Client::chat() 
{
	DWORD id = 0u;
	const int readMark = -1;
	int act = 2;
	std::string MSG;
	HANDLE handler = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StaticThreadStart, this, NULL, &id);
	send(Connection, (char*)&readMark, sizeof(int), NULL);
	std::cout << "\n\tДля выхода наберите ВЫХОД\n\n";
	std::cout << "\n-----------------ЧАТ----------------\n";
	while (true) {
		do
		{
			std::getline(std::cin, MSG);
		} while (MSG == "");
		if (MSG == "ВЫХОД") {
			TerminateThread(handler, id); // экстренное завершение
			send(Connection, (char*)&readMark, sizeof(int), NULL);
			system("cls");
			std::cout << "\n\tВы вышли из чата!\n";
			menu();
			return;
		}
		MSG = "\n" + getCurrentTime() + "\n" + user_name + ": " + MSG;
		send(Connection, (char*)&act, sizeof(int), NULL);
		sendStr(MSG);
	}
}


// запрашивает у пользователя сообщение для удаления
// отправляет на сервер, если такое сообщение у пользователя есть
// отвечает положительно и удаляет сообщение из списка
// в обратном случае, отрицательно, бездействует
void Client::removeMessage() 
{
	int answer;
	std::string req;
	std::string msg = recvStr();
	std::cout << "---------------ИСТОРИЯ ВАШИХ СООБЩЕНИЙ---------------\n\n";
	std::cout << msg;
	std::cout << "---------------ИСТОРИЯ ВАШИХ СООБЩЕНИЙ---------------\n\n";
	std::cout << "Введите одно из ваших сообщений для его удаления: ";
	std::getline(std::cin, req);
	sendStr(req);
	recv(Connection, (char*)&answer, sizeof(int), NULL);
	system("cls");
	if (answer == 1)
		std::cout << "\n\tСообщение удалено\n\n";
	else if (answer == -1)
		std::cout << "\n\tУвы, такого сообщения нет\n\n";
	menu();
}


// запрашивает у пользователя старое сообщение и новое для его замены
// отправляет на сервер и ждет ответа
// сервер отвечает да/нет в зависимости от того
// есть ли такое сообщение у пользователя,
// добавляет отредактированное сообщение в конец
void Client::editMessage() 
{
	int answer;
	std::string req;
	std::string msg = recvStr();
	std::cout << "---------------ИСТОРИЯ ВАШИХ СООБЩЕНИЙ---------------\n\n";
	std::cout << msg;
	std::cout << "---------------ИСТОРИЯ ВАШИХ СООБЩЕНИЙ---------------\n\n";
	std::cout << "Введите одно из ваших сообщений для его редактирование: ";
	std::getline(std::cin, req);
	std::cout << "\nВведите сообщение на замену: ";
	std::getline(std::cin, msg);
	sendStr(req);
	sendStr(msg);
	recv(Connection, (char*)&answer, sizeof(int), NULL);
	system("cls");
	if (answer == 1)
		std::cout << "\n\tСообщение отредактировано\n\n";
	else if (answer == -1)
		std::cout << "\n\tУвы, такого сообщения нет\n\n";
	menu();
}


// отправляет необходимый запрос на сервер
// запросы с индексом 100 ожидают однозначного ответа да/нет
// пока не придет положительный ответ, цикл продолжается
// 1 - 7 могут иметь как однозначный ответ, так и нет
// 101 - запросы на проверку авторизации поользователя
// 102 - запросы на регистрацию нового пользователя
// 1   - запрос на вывод на экран истории сообщений
// 2   - запрос на вход в чат, открывается в UI потоке (см. run())
// 3   - запрос на удаление одного сообщения из истории по полному совпдению
// 4   - запрос на замену одного сообщения другим
// 5   - запрос на сортировку списка сообщений по алфавиту
// 6   - запрос на очищение истории чата (доступен всем)
// 7   - запрос список пользователей онлайн
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
		std::cout << "\n---------------ИСТОРИЯ---------------\n\n";
		std::cout << recvStr();
		std::cout << "---------------ИСТОРИЯ---------------\n\n";
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
		std::cout << "\n\tИстория сообщений отсортирована!\n\n";
		menu();
		break;
	case(6):
		std::cout << "\n\tИстория сообщений очищена!\n\n";
		menu();
		break;
	case(7):
		std::cout << "\n--------------ПОЛЬЗОВАТЕЛИ ОНЛАЙН--------------\n\n";
		std::cout << recvStr() << std::endl;
		std::cout << "--------------ПОЛЬЗОВАТЕЛИ ОНЛАЙН--------------\n\n";
		menu();
		break;
	case(8):
		menu();
		break;


	case(101): // авторизация
		sendStr(user_name);         // отправка имени
		sendStr(myHash(password));  // хеш пароля

		if (recv(Connection, (char*)&answer, sizeof(int), NULL) == SOCKET_ERROR) {
			std::cout << "\nСервер отключен\n";
			return false;
		}
		else if (answer == -1)
			return false;
		break;


	case(102): // регистрация
		sendStr(user_name);
		sendStr(myHash(password));

		recv(Connection, (char*)&answer, sizeof(int), NULL);
		if (answer == -1)
			return false;
		break;
	}
	return true;
}


// Открывается в новом потоке и
// обрабатывает сообщения от сервера,
// пришедшие от других пользователей в чате
// далее выводит сообщение на экран
void Client::clientHandler()
{
	int msg_size = 0;
	while (true) {
		std::cout << recvStr() << std::endl << std::endl;
	}
}


// Берет текущее время
// приводит к заданному формату
// возвращает в виде строки
std::string Client::getCurrentTime()
{
	char buffer[80];
	const char* format = "%d-%m-%Y %H:%M:%S";
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);
	strftime(buffer, 80, format, timeinfo);
	return buffer;
}


// первый метод для входа пользователя в систему
// получется ответы ДА/НЕТ от сервера
void Client::login() 
{
	int act = 1;
	while (true) {
		std::cout << "  1 ---> Вход\n";
		std::cout << "  2 ---> Регистрация\n";
		std::cout << "  0 ---> Выход\n\n";
		do
		{
			act = input(act);
			if (act == 0) exit(0);
			if (act != 1 && act != 2)
				std::cout << "Ошибка ввода, повторите попытку: ";
		} while (act != 1 && act != 2);

		std::cout << "\nЛогин: ";
		std::getline(std::cin, user_name);
		std::cout << "Пароль: ";
		std::getline(std::cin, password);
		system("cls");
		if (act == 1) {
			if (action(101))
				break;
			else
				std::cout << "\nНеправильный логин или пароль!!!\n\n";
		}
		else if (act == 2) {
			if (password.size() < 6 && password.find(" "))
				std::cout << "Слишком короткий пароль!\n\n";
			else if (action(102))
				break;
			else
				std::cout << "\nКажется... Такой пользователь уже существует...\n\n";
		}
	}
	system("cls");
	std::cout << "\tДобро пожаловать, " + user_name + "!!!\n\n";
}


// необходим, т.к. для запуска нового потока
// требуется статический метод
// принимает this, запускает для него поток
// в котором обрабатываются, поступающие с сервера
void Client::StaticThreadStart(Client* client)
{
	client->clientHandler();
}


// Создает хеш входной строки
// используется для хеширования пароля
// хеш отправляется на сервер для сравнения
std::string Client::myHash(std::string str)
{
	std::string hash = "";
	int size = str.size() - 1;
	int center = str.size() / 2;
	unsigned int temp;

	temp = 0;
	for (int i = 0; i < str.size(); i++)
		temp += str.at(i);
	hash += ((temp % 26) + 65);												// Б

	temp = 1;
	for (int i = str.size() / 2; i < str.size(); i++)
		temp += (pow(str.at(i), 3)) + (str.at(0));
	for (int i = 0; i < str.size() / 2; i++)
		temp += (str.at(i)) - str.at((str.size() - 1) / 4);
	temp -= str.at(str.size() / 9) + str.at(str.size() / 4) - pow(str.at(str.size() / 3), 2) + pow(str.at(str.size() / 5), 3);
	hash += ((temp % 26) + 65);												// Б

	temp = 0;
	temp += str.at(0) * str.at(str.size() - 1) / str.at(str.size() / 2) + str.at(str.size() / 3) + str.at(str.size() / 4) * str.at(str.size() / 5);
	hash += ((temp % 10) + 48);												// ц

	temp = 0;
	temp += str.at(str.size() / 2) + str.at((str.size() - 1) / 3);
	temp += pow(str.at(str.size() - 1), 3) / 2 + str.at(0) + str.at(str.size() / 4);
	temp *= str.at((str.size() * 3) / 4) + str.at((str.size() * 2) / 5);
	temp -= str.at(str.size() - 1) * str.at(str.size() / 5);
	hash += ((temp % 10) + 48);												// ц

	temp = 1;
	for (int i = 0; i < str.size(); i++)
		temp *= str.at(i);
	temp -= pow(str.at(0), 3) + (str.at(str.size() - 1)) / (str.at(str.size() / 2) << 2);
	hash += ((temp % 26) + 65);												// Б

	temp = 1;
	for (int i = str.size() / 3; i < str.size(); i++)
		temp += (pow(str.at(i), 3));
	temp *= str.at(str.size() / 8) + str.at(str.size() / 4);
	temp -= (pow(str.at(str.size() - 1), 3)) + (str.at(0)) + (pow(str.at((str.size() - 1) / 3), 3));
	temp += pow(str.at(str.size() / 2), 2) + pow(str.at(str.size() / 3), 3);
	hash += ((temp % 26) + 65);												// Б

	return hash;															// MISH257
}


// основные единицы работы с сервером:
// sendStr отправить строку,
// recvStr получить строку от сервера
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
		std::cout << "\nСервер отключен\n";
		exit(-1);
	}
	char *msg = new char[msg_size];
	msg[msg_size - 1] = '\0';
	recv(Connection, msg, msg_size, NULL);

	std::string MSG = msg;
	delete[] msg;

	return MSG;
}