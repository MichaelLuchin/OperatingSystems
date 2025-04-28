#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <windows.h>
#include <sstream>
#include <cmath>
#include <algorithm>

using namespace std;

HANDLE hSemaphore;
vector<SOCKET> clientSockets;
int clientCount = 0;
const int MAX_CLIENTS = 2;

struct Calculation {
    double num1;
    double num2;
    char op;
};

struct ThreadParams {
    SOCKET socket;
    int id;
};

double calculate(double a, double b, char op) {
    switch(op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if(b == 0) throw runtime_error("Division by zero");
            return a / b;
        default: throw runtime_error("Invalid operator");
    }
}

void handleClient(SOCKET clientSocket, int clientId) {
    char buffer[256];
    int bytesReceived;

    while(true) {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if(bytesReceived <= 0) break;

        buffer[bytesReceived] = '\0';
        string expression(buffer);

        try {
            // Используем std::remove для удаления пробелов
            expression.erase(std::remove(expression.begin(), expression.end(), ' '), expression.end());

            // Ищем оператор в строке
            size_t op_pos = expression.find_first_of("+-*/");
            if(op_pos == string::npos || op_pos == 0 || op_pos == expression.length()-1) {
                throw runtime_error("Invalid input format - operator not found or misplaced");
            }

            Calculation calc;
            calc.op = expression[op_pos];

            try {
                calc.num1 = stod(expression.substr(0, op_pos));
                calc.num2 = stod(expression.substr(op_pos+1));
            } catch(const exception&) {
                throw runtime_error("Invalid numbers format");
            }

            double result = calculate(calc.num1, calc.num2, calc.op);

            string response = "Received from client #" + to_string(clientId) +
                             ": " + expression + ". Result: " + to_string(result);

            send(clientSocket, response.c_str(), response.size() + 1, 0);
        } catch(const exception& e) {
            string error = "Error: " + string(e.what());
            send(clientSocket, error.c_str(), error.size() + 1, 0);
        }
    }

    closesocket(clientSocket);
    ReleaseSemaphore(hSemaphore, 1, NULL);
    clientCount--;
    cout << "Client #" << clientId << " disconnected. Current clients: " << clientCount << endl;
}

DWORD WINAPI ClientThread(LPVOID lpParam) {
    ThreadParams* params = reinterpret_cast<ThreadParams*>(lpParam);
    handleClient(params->socket, params->id);
    delete params;
    return 0;
}

int main() {
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed" << endl;
        return 1;
    }

    hSemaphore = CreateSemaphoreA(NULL, MAX_CLIENTS, MAX_CLIENTS, "CalculatorSemaphore");
    if(hSemaphore == NULL) {
        cerr << "CreateSemaphore error: " << GetLastError() << endl;
        WSACleanup();
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation error" << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345);

    if(bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if(listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server started. Waiting for connections..." << endl;

    while(true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if(clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed" << endl;
            continue;
        }

        WaitForSingleObject(hSemaphore, INFINITE);

        clientCount++;
        int clientId = clientCount;
        cout << "Client #" << clientId << " connected. Current clients: " << clientCount << endl;

        ThreadParams* params = new ThreadParams{clientSocket, clientId};
        HANDLE hThread = CreateThread(NULL, 0, ClientThread, params, 0, NULL);

        if(hThread == NULL) {
            delete params;
            closesocket(clientSocket);
            ReleaseSemaphore(hSemaphore, 1, NULL);
            clientCount--;
            continue;
        }

        CloseHandle(hThread);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}