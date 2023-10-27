#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Błąd inicjalizacji Winsock." << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Błąd podczas tworzenia gniazda." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    if (inet_pton(AF_INET, "192.168.100.201", &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "Błąd konwersji adresu IP." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    while (true) {
        std::cout << "Wprowadź wiadomość do przesłania na serwer (lub wprowadź 'q' aby zakończyć): ";
        std::string message;
        std::cin >> message;

        if (message == "q") {
            break; // Wyjście z pętli
        }

        int bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Błąd podczas wysyłania danych." << std::endl;
        }
        else {
            std::cout << "Wysłano " << bytesSent << " bajtów danych." << std::endl;

            char buffer[1024];
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::cout << "Odebrano: " << buffer << std::endl;
            }
            else {
                std::cerr << "Błąd podczas odbierania danych." << std::endl;
            }
        }
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
