#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <clocale>
using namespace std;
bool flag = true;
void* clientReceive(void* lpParam) {
    char buffer[1024] = { 0 };
    setlocale(LC_ALL,NULL);
    int server = *(int*)lpParam;
    while (flag) {
        if (!flag) {
            break;
        }
        if (recv(server, buffer, sizeof(buffer), 0) == -1) {
            cout << "Ошибка получения ответа" << endl;
            return nullptr;
        }
        if (strcmp(buffer, "exit\n") == 0) {
            cout << "Сервер отключен" << endl;
            flag = false;
            break;
        }
        if (strcmp(buffer, "") != 0) {
        cout << "Server: " << buffer << endl;
        memset(buffer, 0, sizeof(buffer));
        }
    }
    return nullptr;
}

void* clientSend(void* lpParam) {
    char buffer[1024] = { 0 };
    int server = *(int*)lpParam;
    while (flag) {
        fgets(buffer, 1024, stdin);
        if (send(server, buffer, strlen(buffer), 0) == -1) {
            cout << "Ошибка отправки запроса" << endl;
            return nullptr;
        }
        if (strcmp(buffer, "exit\n") == 0) {
            cout << "Выход с сервера" << endl;
            flag = false;
            break;
        }
    }
    return nullptr;
}

int main() {
    int client;
    sockaddr_in addr;
    char buffer[1024] = { 0 };
    char buffer2[1024] = { 0 };
    addr.sin_family = AF_INET;
    

    while (true) {
        if ((client = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Ошибка создагня сокета");
            return -1;
        }
        cout << "Ожидание подключения к серверу, введите IP сервера:" << endl;
        fgets(buffer, 1024, stdin);
        if (strcmp(buffer, "exit\n") == 0) {
            break;
            }
        cout << "Введите порт сервера:" << endl;
        fgets(buffer2, 1024, stdin);
        if (strcmp(buffer2, "exit\n") == 0) {
            break;
            }
        addr.sin_port = htons(atoi(buffer2));
        addr.sin_addr.s_addr = inet_addr(buffer); //получения адресса
        if (connect(client, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("Ошибка подключения");
        }
        else {
            cout << "Успешное подключение к серверу" << endl;
            cout << "Теперь вы можете отправлять запросы путём ввода текста. Enter \"exit\" to disconnect" << endl;

            pthread_t t1, t2;
            if (pthread_create(&t1, NULL, clientReceive, &client) != 0) {
                cout << "Ошибка создания потока получения" << endl;
                return -1;
            }
            if (pthread_create(&t2, NULL, clientSend, &client) != 0) {
                cout << "Ошибка создания потока отправки" << endl;
                return -1;
            }
            pthread_join(t2, NULL);
            pthread_cancel(t1);
            pthread_join(t1, NULL);
            flag = true;
            if (close(client) == -1) { //Закрытие сокета
                cout << "Ошибка закрытия сокета" << endl;
                return -1;
                }
              }
          }
        return 0;
        }
        
