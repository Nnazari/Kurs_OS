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
DWORD WINAPI serverContorl(LPVOID lpParam) { //����஫�
	char buffer[1024] = { 0 }; //���� ��� ������
	SOCKET client = *(SOCKET*)lpParam; //����� ��� ������
	while (true) { //���� ࠡ��� �ࢥ��
		fgets(buffer, 1024, stdin);
		if (send(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {//�訡�� ��ࠢ��
			cout << "�訡�� ��ࠢ�� : " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffer, "exit\n") == 0) {//�訡�� ���
			cout << "�⪫�祭�� �ࢥ��" << endl;
			break;
		}
	}
	return 1;
}
DWORD WINAPI serverSend(LPVOID lpParam) { //��ࠢ�� �������
	char buffercl[1024] = { 0 };
	char buffersr[1024] = { 0 };
	SOCKET client = *(SOCKET*)lpParam;
	while (true) {
		if (recv(client, buffercl, sizeof(buffercl), 0) == SOCKET_ERROR) {
			//�᫨ �� 㤠���� ������� ����� ����, ᮮ���� �� �訡�� � ���
			cout << "�� 㤠���� ������� �����: " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffercl, "exit\n") == 0) {
			cout << "������ �⪫�稫��" << endl;
			break;
		}
		else if (strcmp(buffercl, "session_time\n") == 0) {
			sprintf_s(buffersr, "%lld %s", (GetTickCount64()- startTime)/(1000)," ᥪ㭤");
		}
		else if (strcmp(buffercl, "time_zone\n") == 0) {//�ᮢ�� ����
			TIME_ZONE_INFORMATION tzi;
			DWORD result = GetTimeZoneInformation(&tzi);
			int bias = -(tzi.Bias);
			int hoursOffset = bias / 60;
			int minutesOffset = bias % 60;
			if (result == TIME_ZONE_ID_STANDARD) {
				sprintf_s(buffersr, " %s %d %s %d", "���饭�� �⭮�⥫쭮 UTC: -", hoursOffset, ":", minutesOffset);
			}
			else {
				sprintf_s(buffersr, " %s %d %s %d", "���饭�� �⭮�⥫쭮 UTC: +", hoursOffset, ":", minutesOffset);
			}
		}
		if (send(client, buffersr, sizeof(buffersr), 0) == SOCKET_ERROR) {//�訡�� ��ࠢ��
			cout << "�訡�� ��ࠢ�� : " << WSAGetLastError() << endl;
			return -1;
		}
		memset(buffercl, 0, sizeof(buffercl));
		memset(buffersr, 0, sizeof(buffersr));
	}
	return 1;
}

int main() {
	WSADATA WSAData; //����� 
	SOCKET server1, client; //������ �ࢥ� � ������
	SOCKADDR_IN serverAddr, clientAddr; //���� ᮪�⮢
	while(true){
		WSAStartup(MAKEWORD(2, 0), &WSAData);
		server1 = socket(AF_INET, SOCK_STREAM, 0); //������� �ࢥ�
		if (server1 == INVALID_SOCKET) {
			cout << "�訡�� ᮧ����� �ࢥ��:" << WSAGetLastError() << endl;
			return -1;
		}
		serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(5555);
		if (bind(server1, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
			cout << "�訡�� ��뢠���: " << WSAGetLastError() << endl;
			return -1;
		}

		if (listen(server1, 0) == SOCKET_ERROR) { //�᫨ �� 㤠���� ������� �����
			cout << "�訡�� ����祭�� ����� :" << WSAGetLastError() << endl;
			return -1;
		}
		cout << "��ࢥ� ������� ������祭��...." << endl;

		char buffer[1024];  //������� ���� ��� ������
		int clientAddrSize = sizeof(clientAddr);
		if ((client = accept(server1, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
			//�᫨ ᮥ������� ��⠭������
			startTime = GetTickCount64();
			cout << "������ ������祭." << endl;
			cout << "������ �� ����� ���짮������ ���������." << "Enter \"exit\" to disconnect" << endl;

			DWORD tid; //�����䨪���

			HANDLE t2 = CreateThread(NULL, 0, serverSend, &client, 0, &tid); //�������� ��⮪� ��� ��ࠢ�� ������
			if (t2 == NULL) {
				cout << "�訡�� ᮧ����� ��⮪��: " << WSAGetLastError() << endl;
			}
			HANDLE t1 = CreateThread(NULL, 0, serverContorl, &client, 0, &tid); //�������� ��⮪� ��� ��ࠢ�� ������
			if (t2 == NULL) {
				cout << "�訡�� ᮧ����� ��⮪��: " << WSAGetLastError() << endl;
			}
			WaitForSingleObject(t1, INFINITE);
			WaitForSingleObject(t2, INFINITE);
			if (closesocket(server1) == SOCKET_ERROR) { //�訡�� ������� ᮪��
				cout << "�訡�� ������� ᮪��: " << WSAGetLastError() << endl;
				return -1;
			}
		}
	}
	WSACleanup();
}