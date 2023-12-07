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
		else if (strcmp(buffercl, "Memory\n") == 0) {
			MEMORYSTATUSEX memStatus;
			memStatus.dwLength = sizeof(memStatus);

			if (GlobalMemoryStatusEx(&memStatus)) {
				float freeMemoryPercent = (memStatus.ullAvailPhys * 100.0f) / memStatus.ullTotalPhys;
				sprintf_s(buffersr, "%s %f %c", "Свободная память: ", freeMemoryPercent , '%');
			}else{
				sprintf_s(buffersr, "%s", "Ошибка при получения данных о памяти");
			}
		}
		else if (strcmp(buffercl, "time\n") == 0) {//часовой пояс
			// Получаем дескриптор текущего процесса
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
			FILETIME createTime, exitTime, kernelTime, userTime;
			GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime);

			sprintf_s(buffersr, "%s %lld %s ","Время работы процесса: ",hProcess ,"мс");
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
	SOCKET server2, client; //Сокеты сервера и клиента
	SOCKADDR_IN serverAddr, clientAddr; //Адреса сокетов
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	pid = GetCurrentProcessId();
	server2 = socket(AF_INET, SOCK_STREAM, 0); //Создали сервер
	if (server2 == INVALID_SOCKET) {
		cout << "Ошибка создание сервера :" << WSAGetLastError() << endl;
		return -1;
	}
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.2");
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5555);
	if (bind(server2, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		cout << "Ошибка связывания: " << WSAGetLastError() << endl;
		return -1;
	}

	if (listen(server2, 0) == SOCKET_ERROR) { //Если не удалось получить запрос
		cout << "Ошибка получения запроса :" << WSAGetLastError() << endl;
		return -1;
	}
	cout << "Сервер ожидает подключения...." << endl;
	int clientAddrSize = sizeof(clientAddr); 
	if ((client = accept(server2, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
		//Если соединение установлено
		cout << "Клиент подключен." << endl;
		cout << "Теперь вы можете пользоваться командами." << "Enter \"exit\" to disconnect" << endl;

		DWORD tid; //Идентификатор
		HANDLE t1 = CreateThread(NULL, 0, serverContorl, &client, 0, &tid); //Создание потока для получения данных
		if (t1 == NULL) { //Ошибка создания потока 
			cout << "Ошибка созжания потока : " << WSAGetLastError() << endl;
		}
		HANDLE t2 = CreateThread(NULL, 0, serverSend, &client, 0, &tid); //Создание потока для отправки данных
		if (t2 == NULL) {
			cout << "Ошибка созжания потока : " << WSAGetLastError() << endl;
		}

		WaitForSingleObject(t1, INFINITE);
		WaitForSingleObject(t2, INFINITE);
		if (closesocket(server2) == SOCKET_ERROR) { //Ошибка закрытия сокета
			cout << "Ошибка закрытия сокета: " << WSAGetLastError() << endl;
			return -1;
		}
		WSACleanup();
		flag = true;
	}
}