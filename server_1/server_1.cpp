﻿#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <winsock2.h> 
#pragma comment(lib, "WS2_32.lib")
#include <condition_variable>
#include <Windows.h>
#include <ctime>
using namespace std;

// Подключаемся к именованному каналу

DWORD bytesWritten;
HANDLE pipe;
OVERLAPPED overlapped;
HANDLE connectionPipe() {
	const wchar_t* pipeName = L"\\\\.\\pipe\\TaskServerPipe";
	pipe = CreateFile(
		pipeName,               // Имя канала
		GENERIC_WRITE, // Доступ на запись
		0,                      // Нет общего доступа
		NULL,                   // Атрибуты защиты по умолчанию
		OPEN_EXISTING,          // Открываем существующий канал
		FILE_FLAG_OVERLAPPED,                      // Флаги и атрибуты
		NULL);         // Дескриптор файла шаблона
	return pipe;
}


DWORD WINAPI serverContorl(LPVOID lpParam) { //Управление
	char buffer[1024] = { 0 }; //Сокет клиента
	while (true) { 	
		fgets(buffer, 1024, stdin);
		if (strcmp(buffer, "exit\n") == 0) {
			cout << "Отключение сервера" << endl;
			sprintf_s(buffer, "%s", "Отключение сервера");
			WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
			GetOverlappedResult(pipe, &overlapped, &bytesWritten, TRUE);
			CloseHandle(pipe);
			exit(1);
		}
	}
	return 1;
}
DWORD WINAPI serverSend(LPVOID lpParam) { //Отправка клиенту
	char buffercl[1024] = { 0 };
	char buffersr[1024] = { 0 };
	char buffer[100] = { 0 };
	char bufferready[1024] = {0};
	SOCKET client = *(SOCKET*)lpParam;
	while (true) {
		if (recv(client, buffercl, sizeof(buffercl), 0) == SOCKET_ERROR) {
			cout << "Ошибка получения запроса от клиента: " << WSAGetLastError() << endl;
			sprintf_s(buffer, "%s %d", "Ошибка получения запроса от клиента: ", WSAGetLastError());
			WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
			GetOverlappedResult(pipe, &overlapped, &bytesWritten, TRUE);
			CloseHandle(pipe);
			return -1;
		}
		time_t currentTime = time(nullptr);
		tm localTime;
		localtime_s(&localTime, &currentTime);
		char timeString[100];
		strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &localTime);
		if (strcmp(buffercl, "exit\n") == 0) {
			cout << "Клиент отключился" << endl;
			sprintf_s(buffer, "%s", "Клиент отключился");
			WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
			GetOverlappedResult(pipe, &overlapped, &bytesWritten, TRUE);
			CloseHandle(pipe);
			break;
		}

		else if (strcmp(buffercl, "work_time\n") == 0) {
			ULONGLONG uptime = GetTickCount64();
			unsigned int days = uptime / (24 * 60 * 60 * 1000);
			uptime %= (24 * 60 * 60 * 1000);

			unsigned int hours = uptime / (60 * 60 * 1000);
			uptime %= (60 * 60 * 1000);

			unsigned int minutes = uptime / (60 * 1000);
			uptime %= (60 * 1000);

			unsigned int seconds = uptime / 1000;
			
			sprintf_s(buffersr, "%s %d %s %d %s %d %s %d %s", "Время работы компьютера: ", days, "дней", hours, " часов, "
				, minutes, " минут, "
				, seconds, " секунд.");
			sprintf_s(buffer, "%s", "Время работы сеанса");
			WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
			GetOverlappedResult(pipe, &overlapped, &bytesWritten, TRUE);
			CloseHandle(pipe);
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
			sprintf_s(buffer, "%s", "Клинт смотрит часовой пояс");
			WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
			CloseHandle(pipe);
		}
		sprintf_s(bufferready, "%s %s %s", timeString, ":", buffersr);
		if (send(client, bufferready, strlen(bufferready), 0) == SOCKET_ERROR) {
			cout << "Ошибка отправки ответа клиенту : " << WSAGetLastError() << endl;
			sprintf_s(buffer, "%s %d", "Ошибка отправки ответа клиенту : ", WSAGetLastError());
			WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
			GetOverlappedResult(pipe, &overlapped, &bytesWritten, TRUE);
			CloseHandle(pipe);
			return -1;
		}
		memset(buffercl, 0, sizeof(buffercl));
		memset(buffersr, 0, sizeof(buffersr));
		memset(buffer, 0, sizeof(buffer));
		memset(bufferready, 0, sizeof(bufferready));
	}
	return 1;
}

DWORD WINAPI clientControl(LPVOID client) { //Поток клиента
	
	char buffer[100] = { 0 };
	cout << "Клиент подключен." << endl;
	cout << "Enter \"exit\" to disconnect" << endl;
	sprintf_s(buffer ,"%s", "Клиент подключен.");
	WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
	GetOverlappedResult(pipe, &overlapped, &bytesWritten, TRUE);
	CloseHandle(pipe);
	DWORD tid;
	HANDLE t2 = CreateThread(NULL, 0, serverSend, (SOCKET*)client, 0, &tid);
	if (t2 == NULL) {
				cout << "ошибка создания потока отправки: " << WSAGetLastError() << endl;
				sprintf_s(buffer, "%s %d", "ошибка создания потока отправки: ", WSAGetLastError());
				WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
				GetOverlappedResult(pipe, &overlapped, &bytesWritten, TRUE);
				CloseHandle(pipe);

			}
	WaitForSingleObject(t2, INFINITE);

	return 1;
}


int main() {
	HANDLE mutexHandle = CreateMutexW(NULL, TRUE, (LPCWSTR)"server_1");//проверка
	WSADATA WSAData;
	SOCKET server1, client;
	SOCKADDR_IN serverAddr, clientAddr;
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	char buffer[100] = { 0 };
	// Имя именованного канала
	HANDLE t1 = CreateThread(NULL, 0, serverContorl, 0, 0, 0);
	if (t1 == NULL) {
		cout << "Ошибка создания потока контроля: " << WSAGetLastError() << endl;
		sprintf_s(buffer, "%s %d", "Ошибка создания потока контроля : ", WSAGetLastError());
		WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
		GetOverlappedResult(pipe, &overlapped, &bytesWritten, TRUE);
		CloseHandle(pipe);
	}
	while (true) {

			server1 = socket(AF_INET, SOCK_STREAM, 0); //создание сокета сервера
			if (server1 == INVALID_SOCKET) {
				cout << "Ошибка создания сокета:" << WSAGetLastError() << endl;
				sprintf_s(buffer, "%s %d", "Ошибка создания сокета:", WSAGetLastError());
				WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
				GetOverlappedResult(pipe, &overlapped, &bytesWritten,TRUE);
				CloseHandle(pipe);
				CloseHandle(mutexHandle);
				return -1;
			}
			serverAddr.sin_addr.s_addr = inet_addr("192.168.0.106");
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(5555);
			if (bind(server1, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
				cout << "Ошибка привязки : " << WSAGetLastError() << endl;
				sprintf_s(buffer, "%s %d", "Ошибка привязки : ", WSAGetLastError());
				WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
				GetOverlappedResult(pipe, &overlapped, &bytesWritten, TRUE);
				CloseHandle(pipe);
				CloseHandle(mutexHandle);
				return -1;
			}

			if (listen(server1, 0) == SOCKET_ERROR) {
				cout << "Ошибка поиска:" << WSAGetLastError() << endl;
				sprintf_s(buffer, "%s %d", "Ошибка поиска:", WSAGetLastError());
				WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
				GetOverlappedResult(pipe, &overlapped, &bytesWritten, TRUE);
				CloseHandle(pipe);
				CloseHandle(mutexHandle);
				return -1;
			}
			
			cout << "Ожидание подключения клиента...." << endl;
			sprintf_s(buffer, "%s", "Сервер ожидает подключения клиента....");
			WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
			CloseHandle(pipe);
			int clientAddrSize = sizeof(clientAddr);
			if ((client = accept(server1, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
				DWORD tid;
				HANDLE cl = CreateThread(NULL, 0, clientControl, &client, 0, &tid);
				if (cl == NULL) {
					cout << "Ошибка создания потока клиента: " << WSAGetLastError() << endl;
					sprintf_s(buffer, "%s %d", "Ошибка создания потока клиента: ", WSAGetLastError());
					WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
					GetOverlappedResult(pipe, &overlapped, &bytesWritten, TRUE);
					CloseHandle(pipe);
				}
				if (cl == 0) { WaitForSingleObject(cl, INFINITE); }
			}
			if (closesocket(server1) == SOCKET_ERROR) {
				cout << "Ошибка закрыттия сокета: " << WSAGetLastError() << endl;
				sprintf_s(buffer, "%s %d", "Ошибка закрыттия сокета: ", WSAGetLastError());
				WriteFile(connectionPipe(), buffer, sizeof(buffer), &bytesWritten, &overlapped);
				GetOverlappedResult(pipe, &overlapped, &bytesWritten, TRUE);
				CloseHandle(pipe);
				CloseHandle(mutexHandle);
				return -1;
			}	
	}
	WSACleanup();
}