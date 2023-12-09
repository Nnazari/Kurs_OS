#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <winsock2.h> 
#pragma comment(lib, "WS2_32.lib")
#include <Windows.h>
using namespace std;
bool flag = true;
DWORD pid;
DWORD WINAPI serverContorl(LPVOID lpParam) { //Управление
	char buffer[1024] = { 0 };
	SOCKET client = *(SOCKET*)lpParam; //Сокет клиента
	while (true) {
		fgets(buffer, 1024, stdin);
		if (send(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
			cout << "Ошибка отправки инфомации клиенту: " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffer, "exit\n") == 0) {
			cout << "Отключение сервера" << endl;
			break;
		}
	}
	return 1;
}
DWORD WINAPI serverSend(LPVOID lpParam) {
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
		else if (strcmp(buffercl, "time\n") == 0) {
			
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
			FILETIME createTime, exitTime, kernelTime, userTime;
			GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime);

			sprintf_s(buffersr, "%s %lld %s ","Время процесса в пользовательском режиме: ",hProcess ,"мс");
		}
		if (send(client, buffersr, sizeof(buffersr), 0) == SOCKET_ERROR) {//ЋиЁЎЄ  ®вЇа ўЄЁ
			cout << "Ошибка отправки : " << WSAGetLastError() << endl;
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
	while (true) {
		server1 = socket(AF_INET, SOCK_STREAM, 0); //создание сокета сервера
		if (server1 == INVALID_SOCKET) {
			cout << "Ошибка создания сокета:" << WSAGetLastError() << endl;
			return -1;
		}
		serverAddr.sin_addr.s_addr = inet_addr("127.0.0.2");
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
			WaitForSingleObject(t1, INFINITE);
			WaitForSingleObject(t2, INFINITE);
			if (closesocket(server1) == SOCKET_ERROR) {
				cout << "Ошибка закрыттия сокета: " << WSAGetLastError() << endl;
				return -1;
			}
		}
	}
	WSACleanup();
}