#include <windows.h>
#include <iostream>
#include <fstream>
#include <ctime>
#define _CRT_SECURE_NO_WARNINGS

void WriteToLog(const std::string& logMessage) {
    // Открываем файл лога для записи
    std::ofstream logFile("log.txt", std::ios::app);
    // Get the current time
    std::time_t currentTime = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &currentTime);
    char timeString[100];
    std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &localTime);
    if (logFile.is_open()) {
        // Записываем сообщение в файл
        logFile << "(" << timeString << ")" << ":" + logMessage << std::endl;
        logFile.close();
        std::cout << "(" << timeString << ")" << ":" + logMessage << std::endl;
    }
    else {
        std::cerr << "Не удалось открыть файл лога." << std::endl;
    }
}

int main() {
    // Имя именованного канала
    const wchar_t* pipeName = L"\\\\.\\pipe\\TaskServerPipe";

    // Создаем именованный канал для связи с сервером логирования
    HANDLE pipe = CreateNamedPipe(
        pipeName,                  // Имя канала
        PIPE_ACCESS_INBOUND,        // Поток данных в канале передается только от клиента к серверу. 
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // Режим сообщений и ожидание
        10,  // Количество экземпляров канала
        0,                      // Размер выходного буфера
        4096,                      // Размер входного буфера
        0,                         // Таймаут клиента
        NULL);                     // Атрибуты защиты по умолчанию

    if (pipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Не удалось создать именованный канал. Код ошибки: " << GetLastError() << std::endl;
        return 1;
    }
char buffer[100] = { 0 };
do{
    // Ожидаем подключения к серверу логирования
    if (ConnectNamedPipe(pipe, NULL) != FALSE) {
        
        // Отправляем сообщение на сервер логирования
        DWORD bytesRead;
        ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, NULL);
        WriteToLog(buffer);
        // Закрываем канал после отправки сообщения
        DisconnectNamedPipe(pipe);
    }
} while (strcmp(buffer, "Отключение сервера") != 0);
    // Закрываем канал
    CloseHandle(pipe);

    return 0;
}