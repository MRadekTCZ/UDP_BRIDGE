#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Błąd inicjalizacji Winsock." << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Błąd podczas tworzenia gniazda: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(4077); // numer portu serwera
    if (inet_pton(AF_INET, "192.168.3.59", &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "Błąd konwersji adresu IP." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    int messageCounter = 1;

    while (true) {
        std::string message = "message " + std::to_string(messageCounter);

        int bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Błąd podczas wysyłania danych: " << WSAGetLastError() << std::endl;
        }
        else {
            std::cout << "Wysłano " << bytesSent << " bajtów danych: " << message << std::endl;
        }

        ++messageCounter;
        Sleep(1000); // Opóźnienie programu na 1 sekundę
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
