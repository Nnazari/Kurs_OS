﻿#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <winsock2.h> 
#pragma comment(lib, "WS2_32.lib")
#include <Windows.h>
#include <format>
using namespace std;
DWORD pid;
ULONGLONG startTime;

DWORD WINAPI serverContorl(LPVOID lpParam) { //Управление
	char buffer[1024] = { 0 }; //Сокет клиента
	while (true) {
		fgets(buffer, 1024, stdin);
		if (strcmp(buffer, "exit\n") == 0) {
			cout << "Отключение сервера" << endl;
			exit(1);

		}
	}
}


DWORD WINAPI serverSend(LPVOID lpParam) {
	char buffercl[1024] = { 0 };
	char buffersr[1024] = { 0 };
	char bufferready[1024] = { 0 };
	SOCKET client = *(SOCKET*)lpParam;
	while (true) {
		
		if (recv(client, buffercl, sizeof(buffercl), 0) == SOCKET_ERROR) {
			cout << "Ошибка получения запроса от клиента: " << WSAGetLastError() << endl;
			return -1;
		}
		time_t currentTime = time(nullptr);
		tm localTime;
		localtime_s(&localTime, &currentTime);
		char timeString[100];
		strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &localTime);
		if (strcmp(buffercl, "exit\n") == 0) {
			cout << "Клиент отключился" << endl;
			break;
		}
		else if (strcmp(buffercl, "Memory\n") == 0) {
			MEMORYSTATUSEX memStatus;
			memStatus.dwLength = sizeof(memStatus);

			if (GlobalMemoryStatusEx(&memStatus)) {
				float freeMemoryPercent = (memStatus.ullAvailPhys * 100.0f) / memStatus.ullTotalPhys;
				sprintf_s(buffersr, "%s %f %c", "Свободная память: ", freeMemoryPercent , '%');
			}else{
				sprintf_s(buffersr, "%s", "Ошибка доступа к памяти");
			}
		}
		else if (strcmp(buffercl, "session_time\n") == 0) {
			sprintf_s(buffersr, "%lld %s", (GetTickCount64() - startTime) / (1000), " сек.");
		}
		sprintf_s(bufferready, "%s %s %s", timeString,":", buffersr);
		if (send(client, bufferready, strlen(bufferready), 0) == SOCKET_ERROR) {
			cout << "Ошибка отправки : " << WSAGetLastError() << endl;
			return -1;
		}
		memset(buffercl, 0, sizeof(buffercl));
		memset(buffersr, 0, sizeof(buffersr));
		memset(bufferready, 0, sizeof(bufferready));
	}
	return 1;
}

DWORD WINAPI clientControl(LPVOID client) { //Поток клиента 
	cout << "Клиент подключен" << endl;
	cout << "Enter \"exit\" to disconnect" << endl;

	DWORD tid;

	HANDLE sendThread = CreateThread(NULL, 0, serverSend, (SOCKET*)client, 0, &tid);
	if (sendThread == NULL) {
		cout << "ошибка создания потока отправки: " << WSAGetLastError() << endl;
	}
	

	WaitForSingleObject(sendThread, INFINITE);
	
	return 1;
}





int main() {
	HANDLE mutexHandle = CreateMutexW(NULL, TRUE, (LPCWSTR)"server_2");//проверка
	WSADATA WSAData;
	SOCKET server2, client;
	SOCKADDR_IN serverAddr, clientAddr;
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	startTime = GetTickCount64();//добавить разветвление 
	HANDLE serverControlThred = CreateThread(NULL, 0, serverContorl, 0, 0, 0);
	if (serverControlThred == NULL) {
		cout << "Ошибка создания потока контроля: " << WSAGetLastError() << endl;
	}
	while (true) {
		server2 = socket(AF_INET, SOCK_STREAM, 0); //создание сокета сервера
		if (server2 == INVALID_SOCKET) {
			cout << "Ошибка создания сокета:" << WSAGetLastError() << endl;
			CloseHandle(mutexHandle);
			return -1;
		}
		serverAddr.sin_addr.s_addr = inet_addr("192.168.0.106");
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(5517);
		if (bind(server2, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
			cout << "Ошибка привязки : " << WSAGetLastError() << endl;
			CloseHandle(mutexHandle);
			return -1;
		}

		if (listen(server2, 0) == SOCKET_ERROR) {
			cout << "Ошибка поиска:" << WSAGetLastError() << endl;
			CloseHandle(mutexHandle);
			return -1;
		}
		cout << "Ожидание подключения клиента...." << endl;

		int clientAddrSize = sizeof(clientAddr);
		if ((client = accept(server2, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
			DWORD tid;
			HANDLE cl = CreateThread(NULL, 0, clientControl, &client, 0, &tid);
			if (cl == NULL) {
				cout << "Ошибка создания потока клиента: " << WSAGetLastError() << endl;
			}
			WaitForSingleObject(cl, INFINITE);
		}
		if (closesocket(server2) == SOCKET_ERROR) {
			cout << "Ошибка закрыттия сокета: " << WSAGetLastError() << endl;
			CloseHandle(mutexHandle);
			return -1;
		}
	}
	WSACleanup();
}