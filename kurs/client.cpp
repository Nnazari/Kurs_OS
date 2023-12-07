
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <winsock2.h> 
#pragma comment(lib, "WS2_32.lib")
using namespace std;
bool flag = true;
DWORD WINAPI clientReceive(LPVOID lpParam) { //����祭�� ������ �� �ࢥ�
	char buffer[1024] = { 0 };
	SOCKET server = *(SOCKET*)lpParam;
	while (flag) {
		if (recv(server, buffer, sizeof(buffer), 0) == SOCKET_ERROR ) {
			cout << "����� ᮥ������� : " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffer, "exit\n") == 0) {
			cout << "��ࢥ� �⪫�祭" << endl;
			flag = false;
			break;
		}
		cout << "Server: " << buffer << endl;
		memset(buffer, 0, sizeof(buffer));
	}
	return 1;
}

DWORD WINAPI clientSend(LPVOID lpParam) { //��ࠢ�� ������ �� �ࢥ�
	char buffer[1024] = { 0 };
	SOCKET server = *(SOCKET*)lpParam;
	while (flag) {
		fgets(buffer, 1024, stdin);
		if (send(server, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
			cout << "�訡�� ��ࠢ�� ������: " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffer, "exit\n") == 0) {
			cout << "��室 � �ࢥ�" << endl;
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
	while (true) {
		WSAStartup(MAKEWORD(2, 0), &WSAData);
		if ((client = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			cout << "�訡�� ᮧ����� ᮪��: " << WSAGetLastError() << endl;
			return -1;
		}
		cout << "�������� ������祭�� � �ࢥ� , ������ ip:" << endl;
		fgets(buffer, 1024, stdin);
		addr.sin_addr.s_addr = inet_addr(buffer); //��⠭���� �����
		if (connect(client, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
			cout << "�訡�� ������祭�� � �ࢥ��: " << WSAGetLastError() << endl;
		}
		else if (strcmp(buffer, "exit\n") == 0) {
			break;
		}
		else {
			cout << "�ᯥ譮� ������祭�� � �ࢥ��" << endl;
			cout << "������ �� ����� ���짮������ ���������." << "Enter \"exit\" to disconnect" << endl;

			DWORD tid;
			HANDLE t1 = CreateThread(NULL, 0, clientReceive, &client, 0, &tid);
			if (t1 == NULL) cout << "�訡�� ᮧ����� ��⮪��: " << GetLastError();
			HANDLE t2 = CreateThread(NULL, 0, clientSend, &client, 0, &tid);
			if (t2 == NULL) cout << "�訡�� ᮧ����� ��⮪��: " << GetLastError();
			WaitForSingleObject(t1, 5);
			if (t2 != 0) { WaitForSingleObject(t2, INFINITE); }
			flag = true;
			if (closesocket(client) == SOCKET_ERROR) { //�����⨥ ᮪��
				cout << "�訡�� ᮧ����� ᮪��" << WSAGetLastError() << endl;
				return -1;
			}
		}
	}
	WSACleanup();
}