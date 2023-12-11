
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "WS2_32.lib")
using namespace std;
bool flag = true;
DWORD WINAPI clientReceive(LPVOID lpParam) { //получения клиентом
	char buffer[1024] = { 0 };
	SOCKET server = *(SOCKET*)lpParam;
	while (flag) {
		if (!flag) {break;}
		if (recv(server, buffer, sizeof(buffer), 0) == SOCKET_ERROR ) {
			cout << "Ошибка получения ответа : " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffer, "exit\n") == 0) {
			cout << "Сервер отключен" << endl;
			flag = false;
			break;
		}
		cout << "Server: " << buffer << endl;
		memset(buffer, 0, sizeof(buffer));
	}
	return 1;
}

DWORD WINAPI clientSend(LPVOID lpParam) { //отправка клиентом
	char buffer[1024] = { 0 };
	SOCKET server = *(SOCKET*)lpParam;
	while (flag) {
		fgets(buffer, 1024, stdin);
		if (send(server, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
			cout << "Ошибка отправки запроса 1: " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffer, "exit\n") == 0) {
			cout << "Выход с сервера" << endl;
			flag = false;
			break;
		}
	}
	return 1;
}
int main() {
	WSADATA WSAData;
	SOCKET client;
	SOCKADDR_IN addr;
	char buffer[1024] = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5555);
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	
	while (true) {
		if ((client = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			cout << "Ошибка создания сокета: " << WSAGetLastError() << endl;
			return -1;
		}
		cout << "Ожидание подключения к серверу , введите ip сервера:" << endl;
		fgets(buffer, 1024, stdin);
		addr.sin_addr.s_addr = inet_addr(buffer); //получения адресса
		if (connect(client, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
			cout << "Ошибка подключения: " << WSAGetLastError() << endl;
		}
		else if (strcmp(buffer, "exit\n") == 0) {
			break;
		}
		else {
			cout << "Успешное подключение к серверу" << endl;
			cout << "Теперь вы можете отправлять запросы путём ввода текста." << "Enter \"exit\" to disconnect" << endl;

			DWORD tid;
			HANDLE t1 = CreateThread(NULL, 0, clientReceive, &client, 0, &tid);
			if (t1 == NULL) cout << "Ошибка создания потока получения: " << GetLastError();
			HANDLE t2 = CreateThread(NULL, 0, clientSend, &client,  0, &tid);
			if (t2 == NULL) cout << "Ошибка создания потока отправки: " << GetLastError();
			WaitForSingleObject(t2, INFINITE);
			TerminateThread(t1, 1);
			WaitForSingleObject(t1, INFINITE);
			flag = true;
			if (closesocket(client) == SOCKET_ERROR) { //Закрытие сокета
				cout << "Ошибка закрытия сокета : " << WSAGetLastError() << endl;
				return -1;
			}
		}
	}
	WSACleanup();
}