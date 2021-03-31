# SocketMessenger
Клиент-серверное приложение, позволяющее обмениваться сообщениями через интернет.

Для работы с подключениями используется библиотека <winsock2.h>. Сервер принимает все входящие в него запросы по подключению и ожидает дальнейших действий. В сервере используется многопоточность. В основном (UI) потоке сервер обрабатывает запросы на подключения и создает для каждого подключения свой поток. Вторичные потоки пользователей обрабатывают запросы конкретной запущенной программы. В случае экстренных завершения первого или второго приложения отключается и возвращает код ошибки. В качестве базы данных используется обычные .txt файлы, хранящиеся на сервере. Все существующие пользователи равны в правах, функция очистки сделана в качестве требования к работе программы. Каждый пользователь обладает правами на изменения только своих данных. С базой данных идет постоянное взаимодействие, при запуске сервера осуществляется считывание данных, при внесении каких либо изменений в основные списки (пользователей или сообщений) файл перезаписывается в только что внесенными изменениями. Локер файла и остальные элементы безопасной работы с многопоточностью отсутствуют, т.к. не предусмотрена большая нагрузка на серверную часть.
