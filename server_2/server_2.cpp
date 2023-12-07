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
		else if (strcmp(buffercl, "Memory\n") == 0) {
			MEMORYSTATUSEX memStatus;
			memStatus.dwLength = sizeof(memStatus);

			if (GlobalMemoryStatusEx(&memStatus)) {
				float freeMemoryPercent = (memStatus.ullAvailPhys * 100.0f) / memStatus.ullTotalPhys;
				sprintf_s(buffersr, "%s %f %c", "��������� ������: ", freeMemoryPercent , '%');
			}else{
				sprintf_s(buffersr, "%s", "�訡�� �� ����祭�� ������ � �����");
			}
		}
		else if (strcmp(buffercl, "time\n") == 0) {//�ᮢ�� ����
			// ����砥� ���ਯ�� ⥪�饣� �����
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
			FILETIME createTime, exitTime, kernelTime, userTime;
			GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime);

			sprintf_s(buffersr, "%s %lld %s ","�६� ࠡ��� �����: ",hProcess ,"��");
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
	SOCKET server2, client; //������ �ࢥ� � ������
	SOCKADDR_IN serverAddr, clientAddr; //���� ᮪�⮢
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	pid = GetCurrentProcessId();
	server2 = socket(AF_INET, SOCK_STREAM, 0); //������� �ࢥ�
	if (server2 == INVALID_SOCKET) {
		cout << "�訡�� ᮧ����� �ࢥ��:" << WSAGetLastError() << endl;
		return -1;
	}
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.2");
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5555);
	if (bind(server2, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		cout << "�訡�� ��뢠���: " << WSAGetLastError() << endl;
		return -1;
	}

	if (listen(server2, 0) == SOCKET_ERROR) { //�᫨ �� 㤠���� ������� �����
		cout << "�訡�� ����祭�� ����� :" << WSAGetLastError() << endl;
		return -1;
	}
	cout << "��ࢥ� ������� ������祭��...." << endl;
	int clientAddrSize = sizeof(clientAddr); 
	if ((client = accept(server2, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
		//�᫨ ᮥ������� ��⠭������
		cout << "������ ������祭." << endl;
		cout << "������ �� ����� ���짮������ ���������." << "Enter \"exit\" to disconnect" << endl;

		DWORD tid; //�����䨪���
		HANDLE t1 = CreateThread(NULL, 0, serverContorl, &client, 0, &tid); //�������� ��⮪� ��� ����祭�� ������
		if (t1 == NULL) { //�訡�� ᮧ����� ��⮪��
			cout << "�訡�� ᮧ����� ��⮪��: " << WSAGetLastError() << endl;
		}
		HANDLE t2 = CreateThread(NULL, 0, serverSend, &client, 0, &tid); //�������� ��⮪� ��� ��ࠢ�� ������
		if (t2 == NULL) {
			cout << "�訡�� ᮧ����� ��⮪��: " << WSAGetLastError() << endl;
		}

		WaitForSingleObject(t1, INFINITE);
		WaitForSingleObject(t2, INFINITE);
		if (closesocket(server2) == SOCKET_ERROR) { //�訡�� ������� ᮪��
			cout << "�訡�� ������� ᮪��: " << WSAGetLastError() << endl;
			return -1;
		}
		WSACleanup();
		flag = true;
	}
}