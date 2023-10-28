#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define CLIENT_PORT 723
#define SERVER_PORT 4077
#define SERVER_IP "192.168.3.247"

// Funkcja obsługująca odbieranie odpowiedzi od serwera w osobnym wątku
void ReceiveThread(SOCKET clientSocket) {
    char buffer[1024];
    sockaddr_in serverAddr;
    int addrLen = sizeof(serverAddr);

    while (true) {
        int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddr, &addrLen);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "Odebrano: " << buffer << std::endl;
        }
        else {
            std::cerr << "Błąd podczas odbierania danych: " << WSAGetLastError() << std::endl;
            break;
        }
    }
}

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
    clientAddr.sin_port = htons(CLIENT_PORT); // Wybierz dowolny numer portu klienta
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    // Przypisz numer portu do gniazda klienta
    if (bind(clientSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == SOCKET_ERROR) {
        std::cerr << "Error in client port" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT); // Numer portu serwera
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "Error IP." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Uruchom wątek do obsługi odbierania odpowiedzi od serwera
    std::thread receiveThread(ReceiveThread, clientSocket);

    while (true) {
        std::cout << "Message to send (HEX) - od press 'q' to leave";
        std::string message;
        std::cin >> message;

        if (message == "q") {
            break;
        }

        int bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Error while sending the data: " << WSAGetLastError() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Odczekaj 1 sekundę przed kolejnym wprowadzeniem wiadomości
    }

    // Dołącz wątek obsługi odbierania odpowiedzi
    receiveThread.join();

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
