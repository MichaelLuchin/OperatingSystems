#include <iostream>
#include <string>
#include <winsock2.h>

using namespace std;

int main() {
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cerr << "WSAStartup failed" << endl;
		return 1;
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket == INVALID_SOCKET) {
		cerr << "Socket creation error" << endl;
		WSACleanup();
		return 1;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(12345);

	if(connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		cerr << "Connection failed" << endl;
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	cout << "Connected to server. Enter expressions in format: <number> <operator> <number>" << endl;
	cout << "Supported operators: + - * /" << endl;
	cout << "Enter 'exit' to quit" << endl;

	char buffer[256];
	int bytesReceived;

	while(true) {
		cout << "> ";
		string input;
		getline(cin, input);

		if(input == "exit") break;

		send(clientSocket, input.c_str(), input.size() + 1, 0);

		bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
		if(bytesReceived <= 0) {
			cerr << "Server disconnected" << endl;
			break;
		}

		buffer[bytesReceived] = '\0';
		cout << buffer << endl;
	}

	closesocket(clientSocket);
	WSACleanup();
	return 0;
}