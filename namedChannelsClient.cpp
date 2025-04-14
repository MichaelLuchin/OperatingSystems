//
// Created by m_luc on 14.04.2025.
//

#include <windows.h>
#include <iostream>
#include <string>

#define BUFFER_SIZE 1024

int client() {
    std::cout << "Connecting to server..." << std::endl;

    // Подключаемся к именованному каналу
    HANDLE hNamedPipe = CreateFile(
            TEXT("\\\\.\\pipe\\ArithmeticPipe"),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

    if (hNamedPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to connect to named pipe, GLE=" << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Connected to server. Enter arithmetic operations (e.g., +322, -55, /400.54, *322)" << std::endl;
    std::cout << "Commands: CALCULATE, CLEAR, VALIDATE, EXIT" << std::endl;

    char buffer[BUFFER_SIZE];
    DWORD bytesWritten, bytesRead;

    while (true) {
        std::string input;
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "EXIT") {
            break;
        }

        // Отправляем сообщение серверу
        BOOL success = WriteFile(
                hNamedPipe,
                input.c_str(),
                input.size() + 1,
                &bytesWritten,
                NULL);

        if (!success) {
            std::cerr << "WriteFile failed, GLE=" << GetLastError() << std::endl;
            break;
        }

        // Читаем ответ сервера
        success = ReadFile(
                hNamedPipe,
                buffer,
                BUFFER_SIZE * sizeof(char),
                &bytesRead,
                NULL);

        if (!success) {
            if (GetLastError() == ERROR_BROKEN_PIPE) {
                std::cout << "Server disconnected." << std::endl;
            }
            else {
                std::cerr << "ReadFile failed, GLE=" << GetLastError() << std::endl;
            }
            break;
        }

        buffer[bytesRead] = '\0';
        std::cout << "Server response: " << buffer << std::endl;
    }

    CloseHandle(hNamedPipe);
    return 0;
}
