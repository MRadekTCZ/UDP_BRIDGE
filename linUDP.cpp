#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <string>
#include <unistd.h>

int main() {
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Błąd podczas tworzenia gniazda." << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(4077); // Numer portu serwera
    if (inet_pton(AF_INET, "192.168.3.247", &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "Błąd konwersji adresu IP." << std::endl;
        close(clientSocket);
        return 1;
    }

    int messageCounter = 1;

    while (true) {
        std::string message = "message " + std::to_string(messageCounter);

        int bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == -1) {
            std::cerr << "Błąd podczas wysyłania danych." << std::endl;
        }
        else {
            std::cout << "Wysłano " << bytesSent << " bajtów danych: " << message << std::endl;
        }

        ++messageCounter;
        sleep(5); // Odczekaj 5 sekund przed wysłaniem kolejnej wiadomości
    }

    close(clientSocket);

    return 0;
}
