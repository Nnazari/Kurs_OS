#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <winsock2.h> 
#pragma comment(lib, "WS2_32.lib")
#include <condition_variable>
#include <Windows.h>
#include <ctime>
using namespace std;
ULONGLONG startTime;
DWORD WINAPI serverContorl(LPVOID lpParam) { //Управление
	char buffer[1024] = { 0 };
	SOCKET client = *(SOCKET*)lpParam; //Сокет клиента
	while (true) { 
		fgets(buffer, 1024, stdin);
		if (strcmp(buffer, "exit\n") == 0) {
			if (send(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
				cout << "Ошибка отправки инфомации клиенту: " << WSAGetLastError() << endl;
				return -1;
			}
			cout << "Отключение сервера" << endl;
			break;
		}
	}
	return 1;
}
DWORD WINAPI serverSend(LPVOID lpParam) { //Отправка клиенту
	char buffercl[1024] = { 0 };
	char buffersr[1024] = { 0 };
	SOCKET client = *(SOCKET*)lpParam;
	while (true) {
		if (recv(client, buffercl, sizeof(buffercl), 0) == SOCKET_ERROR) {
			cout << "Ошибка получения запроса от клиента: " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffercl, "exit\n") == 0) {
			cout << "Клиент отключился" << endl;
			break;
		}
		else if (strcmp(buffercl, "session_time\n") == 0) {
			sprintf_s(buffersr, "%lld %s", (GetTickCount64()- startTime)/(1000)," сек.");
		}
		else if (strcmp(buffercl, "time_zone\n") == 0) {
			TIME_ZONE_INFORMATION tzi;
			DWORD result = GetTimeZoneInformation(&tzi);
			int bias = -(tzi.Bias);
			int hoursOffset = bias / 60;
			int minutesOffset = bias % 60;
			if (result == TIME_ZONE_ID_STANDARD) {
				sprintf_s(buffersr, " %s %d %s %d", "Время относительно UTC: -", hoursOffset, ":", minutesOffset);
			}
			else {
				sprintf_s(buffersr, " %s %d %s %d", "Время относительно UTC: +", hoursOffset, ":", minutesOffset);
			}
		}
		if (send(client, buffersr, sizeof(buffersr), 0) == SOCKET_ERROR) {
			cout << "Ошибка отправки ответа клиенту : " << WSAGetLastError() << endl;
			return -1;
		}
		memset(buffercl, 0, sizeof(buffercl));
		memset(buffersr, 0, sizeof(buffersr));
	}
	return 1;
}

int main() {
	WSADATA WSAData;
	SOCKET server1, client;
	SOCKADDR_IN serverAddr, clientAddr;
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	while(true){
		server1 = socket(AF_INET, SOCK_STREAM, 0); //создание сокета сервера
		if (server1 == INVALID_SOCKET) {
			cout << "Ошибка создания сокета:" << WSAGetLastError() << endl;
			return -1;
		}
		serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(5555);
		if (bind(server1, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
			cout << "Ошибка привязки : " << WSAGetLastError() << endl;
			return -1;
		}

		if (listen(server1, 0) == SOCKET_ERROR) {
			cout << "Ошибка поиска:" << WSAGetLastError() << endl;
			return -1;
		}
		cout << "Ожидание подключения клиента...." << endl;

		int clientAddrSize = sizeof(clientAddr);
		if ((client = accept(server1, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
			startTime = GetTickCount64();
			cout << "Клиент подключен­." << endl;
			cout << "Enter \"exit\" to disconnect" << endl;

			DWORD tid; 

			HANDLE t2 = CreateThread(NULL, 0, serverSend, &client, 0, &tid); 
			if (t2 == NULL) {
				cout << "ошибка создания потока отправки: " << WSAGetLastError() << endl;
			}
			HANDLE t1 = CreateThread(NULL, 0, serverContorl, &client, 0, &tid); 
			if (t1 == NULL) {
				cout << "Ошибка создания потока контроля: " << WSAGetLastError() << endl;
			}
			
			WaitForSingleObject(t2, INFINITE);
			TerminateThread(t1, 1);
			WaitForSingleObject(t1, INFINITE);
			TerminateThread(t2, 1);
			if (closesocket(server1) == SOCKET_ERROR) { 
				cout << "Ошибка закрыттия сокета: " << WSAGetLastError() << endl;
				return -1;
			}
		}
	}
	WSACleanup();
}