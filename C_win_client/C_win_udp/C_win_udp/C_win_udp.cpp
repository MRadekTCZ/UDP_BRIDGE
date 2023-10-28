#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#pragma comment(lib, "ws2_32.lib")
#define TWÓJ_WYBRANY_PORT 723
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

    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(TWÓJ_WYBRANY_PORT); // Tutaj wpisz wybrany numer portu dla klienta
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    // Przypisz numer portu do gniazda klienta
    if (bind(clientSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == SOCKET_ERROR) {
        std::cerr << "Błąd podczas przypisywania numeru portu klientowi." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(4077); // numer portu serwera
    if (inet_pton(AF_INET, "192.168.3.247", &(serverAddr.sin_addr)) <= 0) {
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

            char buffer[1024];
            int addrLen = sizeof(serverAddr);
            int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddr, &addrLen);

            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::cout << "Odebrano: " << buffer << std::endl;
            }
            else {
                std::cerr << "Błąd podczas odbierania danych: " << WSAGetLastError() << std::endl;
            }
        }

        ++messageCounter;
        Sleep(5000); // Odczekaj 5 sekund przed wysłaniem kolejnej wiadomości
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
