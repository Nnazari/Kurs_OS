
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <winsock2.h> 
#pragma comment(lib, "WS2_32.lib")
using namespace std;
HANDLE t1, t2;
bool isTerminatedt1 = false;
bool isTerminatedt2 = false;
DWORD WINAPI clientReceive(LPVOID lpParam) { //получения клиентом
	char buffer[1024] = { 0 };
	SOCKET server = *(SOCKET*)lpParam;
	while (true) {
		HANDLE mutexHandle = CreateMutexW(NULL, TRUE, (LPCWSTR)"recv_send");
		if (isTerminatedt1) { 
			CloseHandle(mutexHandle);
			break;
		}
		if (recv(server, buffer, sizeof(buffer), 0) == SOCKET_ERROR ) {
			if (WSAGetLastError() == 10054) {
				cout << "Сервер отключен."<< endl;
				isTerminatedt2 = true;
				return 1;
			}
			else
			cout << "Ошибка получения ответа : " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffer, "") != 0) {
			cout << "Server "<< buffer << endl;
			
		}
		memset(buffer, 0, sizeof(buffer));
		CloseHandle(mutexHandle);
	}
	isTerminatedt1 = false;
	return 1;
}

DWORD WINAPI clientSend(LPVOID lpParam) { //отправка клиентом
	char buffer[1024] = { 0 };
	SOCKET server = *(SOCKET*)lpParam;
	while (true) {
		HANDLE mutexHandle = CreateMutexW(NULL, TRUE, (LPCWSTR)"recv_send");
		fgets(buffer, 1024, stdin);
		if (isTerminatedt2) { 
			CloseHandle(mutexHandle);
			break; 
		}
		if (send(server, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
			cout << "Ошибка отправки запроса : " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffer, "exit\n") == 0) {
			cout << "Выход с сервера" << endl;
			return 1;
		}
		memset(buffer, 0, sizeof(buffer));
		CloseHandle(mutexHandle);
	}
	isTerminatedt2 = false;
	return 1;
}

int main() {
	WSADATA WSAData;
	SOCKET client;
	SOCKADDR_IN addr;
	char buffer[1024] = { 0 };
	char buffer2[1024] = { 0 };
	addr.sin_family = AF_INET;
	
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	
	while (true) {
		isTerminatedt1 = false;
		isTerminatedt2 = false;
		if ((client = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			cout << "Ошибка создания сокета: " << WSAGetLastError() << endl;
			WSACleanup();
			return -1;
		}
		cout << "Ожидание подключения к серверу , введите ip сервера:" << endl;
		fgets(buffer, 1024, stdin);
		addr.sin_addr.s_addr = inet_addr(buffer); //получения адресса
		if (strcmp(buffer, "exit\n") == 0) {
			WSACleanup();
			exit(1);
		}
		cout << "Введите порт:" << endl;
		fgets(buffer2, 1024, stdin);
		if (strcmp(buffer2, "exit\n") == 0) {
			WSACleanup();
			exit(1);
		}
		addr.sin_port = htons(atoi(buffer2));
		if (connect(client, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
			cout << "Ошибка подключения: " << WSAGetLastError() << endl;
		}
		else {
			cout << "Успешное подключение к серверу" << endl;
			cout << "Теперь вы можете отправлять запросы путём ввода текста." << "Enter \"exit\" to disconnect" << endl;

			DWORD tid;
			 t1 = CreateThread(NULL, 0, clientReceive, &client, 0, &tid);
			if (t1 == NULL) cout << "Ошибка создания потока получения: " << GetLastError();
			 t2 = CreateThread(NULL, 0, clientSend, &client,  0, &tid);
			if (t2 == NULL) cout << "Ошибка создания потока отправки: " << GetLastError();
			WaitForSingleObject(t2, INFINITE);
			TerminateThread(t1, 1);
			WaitForSingleObject(t1, INFINITE);
			TerminateThread(t2, 1);
			 
			if (closesocket(client) == SOCKET_ERROR) { //Закрытие сокета
				cout << "Ошибка закрытия сокета : " << WSAGetLastError() << endl;
				WSACleanup();
				return -1;
			}
		}
	}
	WSACleanup();
}