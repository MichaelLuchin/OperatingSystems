//
// Created by m_luc on 14.04.2025.
//

#include <windows.h>
#include <iostream>
#include <string>
#include <stack>
#include <cmath>
#include <algorithm>

#define BUFFER_SIZE 1024

// Функция для проверки приоритета операторов
int getPriority(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

// Функция для применения оператора к двум операндам
double applyOp(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) throw std::runtime_error("Division by zero");
            return a / b;
    }
    return 0;
}

// Функция для вычисления выражения в инфиксной нотации
double evaluateExpression(const std::string& expression) {
    std::stack<double> values;
    std::stack<char> ops;

    for (size_t i = 0; i < expression.length(); i++) {
        if (expression[i] == ' ') continue;

        if (expression[i] == '(') {
            ops.push(expression[i]);
        }
        else if (isdigit(expression[i]) || expression[i] == '.') {
            std::string numStr;
            while (i < expression.length() && (isdigit(expression[i]) || expression[i] == '.')) {
                numStr += expression[i++];
            }
            i--;
            values.push(std::stod(numStr));
        }
        else if (expression[i] == ')') {
            while (!ops.empty() && ops.top() != '(') {
                double val2 = values.top(); values.pop();
                double val1 = values.top(); values.pop();
                char op = ops.top(); ops.pop();
                values.push(applyOp(val1, val2, op));
            }
            if (!ops.empty()) ops.pop();
        }
        else {
            while (!ops.empty() && getPriority(ops.top()) >= getPriority(expression[i])) {
                double val2 = values.top(); values.pop();
                double val1 = values.top(); values.pop();
                char op = ops.top(); ops.pop();
                values.push(applyOp(val1, val2, op));
            }
            ops.push(expression[i]);
        }
    }

    while (!ops.empty()) {
        double val2 = values.top(); values.pop();
        double val1 = values.top(); values.pop();
        char op = ops.top(); ops.pop();
        values.push(applyOp(val1, val2, op));
    }

    return values.top();
}

// Функция для проверки корректности выражения
bool isExpressionValid(const std::string& expression) {
    try {
        evaluateExpression(expression);
        return true;
    }
    catch (...) {
        return false;
    }
}

int server() {
    std::cout << "Server is running..." << std::endl;

    // Создаем именованный канал
    HANDLE hNamedPipe = CreateNamedPipe(
            TEXT("\\\\.\\pipe\\ArithmeticPipe"),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1,
            BUFFER_SIZE,
            BUFFER_SIZE,
            0,
            NULL);

    if (hNamedPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateNamedPipe failed, GLE=" << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Waiting for client connection..." << std::endl;

    // Ожидаем подключения клиента
    BOOL connected = ConnectNamedPipe(hNamedPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    if (!connected) {
        std::cerr << "ConnectNamedPipe failed, GLE=" << GetLastError() << std::endl;
        CloseHandle(hNamedPipe);
        return 1;
    }

    std::cout << "client connected." << std::endl;

    std::string currentExpression;
    char buffer[BUFFER_SIZE];
    DWORD bytesRead;

    while (true) {
        // Читаем данные от клиента
        BOOL success = ReadFile(
                hNamedPipe,
                buffer,
                BUFFER_SIZE * sizeof(char),
                &bytesRead,
                NULL);

        if (!success || bytesRead == 0) {
            if (GetLastError() == ERROR_BROKEN_PIPE) {
                std::cout << "client disconnected." << std::endl;
            }
            else {
                std::cerr << "ReadFile failed, GLE=" << GetLastError() << std::endl;
            }
            break;
        }

        buffer[bytesRead] = '\0';
        std::string clientMessage(buffer);

        std::cout << "Received from client: " << clientMessage << std::endl;

        std::string response;

        // Обработка служебных команд
        if (clientMessage == "CALCULATE") {
            try {
                double result = evaluateExpression(currentExpression);
                response = "Result: " + std::to_string(result);
                currentExpression.clear();
            }
            catch (const std::exception& e) {
                response = "Error: " + std::string(e.what());
            }
        }
        else if (clientMessage == "CLEAR") {
            currentExpression.clear();
            response = "Expression cleared";
        }
        else if (clientMessage == "VALIDATE") {
            response = isExpressionValid(currentExpression) ?
                       "Expression is valid" : "Expression is invalid";
        }
        else {
            // Добавляем введенную операцию к текущему выражению
            currentExpression += clientMessage;
            response = "Current expression: " + currentExpression;
        }

        // Отправляем ответ клиенту
        DWORD bytesWritten;
        success = WriteFile(
                hNamedPipe,
                response.c_str(),
                response.size() + 1,
                &bytesWritten,
                NULL);

        if (!success) {
            std::cerr << "WriteFile failed, GLE=" << GetLastError() << std::endl;
            break;
        }
    }

    // Закрываем канал
    CloseHandle(hNamedPipe);
    std::cout << "Server stopped." << std::endl;
    return 0;
}