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
DWORD WINAPI serverContorl(LPVOID lpParam) { //Контроль
	char buffer[1024] = { 0 }; //Буфер для данных
	SOCKET client = *(SOCKET*)lpParam; //Сокет для клиента
	while (true) { //Цикл работы сервера 
		fgets(buffer, 1024, stdin);
		if (send(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {//Ошибка отправки
			cout << "Ошибка отправки : " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffer, "exit\n") == 0) {//Ошибка приёма
			cout << "Отключение сервера " << endl;
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
			//Если не удалось получить данные буфера, сообщить об ошибке и выйти
			cout << "Не удалось получить данные: " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffercl, "exit\n") == 0) {
			cout << "Клиент отключился" << endl;
			break;
		}
		else if (strcmp(buffercl, "session_time\n") == 0) {
			sprintf_s(buffersr, "%lld %s", (GetTickCount64()- startTime)/(1000)," секунд");
		}
		else if (strcmp(buffercl, "time_zone\n") == 0) {//часовой пояс
			TIME_ZONE_INFORMATION tzi;
			DWORD result = GetTimeZoneInformation(&tzi);
			int bias = -(tzi.Bias);
			int hoursOffset = bias / 60;
			int minutesOffset = bias % 60;
			if (result == TIME_ZONE_ID_STANDARD) {
				sprintf_s(buffersr, " %s %d %s %d", "Смещение относительно UTC: -", hoursOffset, ":", minutesOffset);
			}
			else {
				sprintf_s(buffersr, " %s %d %s %d", "Смещение относительно UTC: +", hoursOffset, ":", minutesOffset);
			}
		}
		if (send(client, buffersr, sizeof(buffersr), 0) == SOCKET_ERROR) {//Ошибка отправки
			cout << "Ошибка отправки : " << WSAGetLastError() << endl;
			return -1;
		}
		memset(buffercl, 0, sizeof(buffercl));
		memset(buffersr, 0, sizeof(buffersr));
	}
	return 1;
}

int main() {
	WSADATA WSAData; //Данные 
	SOCKET server1, client; //Сокеты сервера и клиента
	SOCKADDR_IN serverAddr, clientAddr; //Адреса сокетов
	while(true){
		WSAStartup(MAKEWORD(2, 0), &WSAData);
		server1 = socket(AF_INET, SOCK_STREAM, 0); //Создали сервер
		if (server1 == INVALID_SOCKET) {
			cout << "Ошибка создание сервера :" << WSAGetLastError() << endl;
			return -1;
		}
		serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(5555);
		if (bind(server1, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
			cout << "Ошибка связывания: " << WSAGetLastError() << endl;
			return -1;
		}

		if (listen(server1, 0) == SOCKET_ERROR) { //Если не удалось получить запрос
			cout << "Ошибка получения запроса :" << WSAGetLastError() << endl;
			return -1;
		}
		cout << "Сервер ожидает подключения...." << endl;

		char buffer[1024];  //Создать буфер для данных
		int clientAddrSize = sizeof(clientAddr);
		if ((client = accept(server1, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
			//Если соединение установлено
			startTime = GetTickCount64();
			cout << "Клиент подключен." << endl;
			cout << "Теперь вы можете пользоваться командами." << "Enter \"exit\" to disconnect" << endl;

			DWORD tid; //Идентификатор

			HANDLE t2 = CreateThread(NULL, 0, serverSend, &client, 0, &tid); //Создание потока для отправки данных
			if (t2 == NULL) {
				cout << "Ошибка созжания потока : " << WSAGetLastError() << endl;
			}
			HANDLE t1 = CreateThread(NULL, 0, serverContorl, &client, 0, &tid); //Создание потока для отправки данных
			if (t2 == NULL) {
				cout << "Ошибка созжания потока : " << WSAGetLastError() << endl;
			}
			WaitForSingleObject(t1, INFINITE);
			WaitForSingleObject(t2, INFINITE);
			if (closesocket(server1) == SOCKET_ERROR) { //Ошибка закрытия сокета
				cout << "Ошибка закрытия сокета: " << WSAGetLastError() << endl;
				return -1;
			}
		}
	}
	WSACleanup();
}