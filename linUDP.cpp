#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <chrono>

#define CLIENT_PORT 1777
#define SERVER_PORT 4077
#define SERVER_IP "192.168.3.247"

// Funkcja obsługująca odbieranie odpowiedzi od serwera w osobnym wątku
void ReceiveThread(int clientSocket) {
    char buffer[1024];
    sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(serverAddr);

    while (true) {
        int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddr, &addrLen);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "Odebrano: " << buffer << std::endl;
        }
        else {
            std::cerr << "Błąd podczas odbierania danych." << std::endl;
            break;
        }
    }
}

int main() {
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Błąd podczas tworzenia gniazda." << std::endl;
        return 1;
    }

    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(1777); // Wybierz numer portu 1777
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    // Przypisz numer portu do gniazda klienta
    if (bind(clientSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == -1) {
        std::cerr << "Błąd przypisywania portu klienta." << std::endl;
        close(clientSocket);
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT); // Numer portu serwera
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "Błąd konwersji adresu IP." << std::endl;
        close(clientSocket);
        return 1;
    }

    // Uruchom wątek do obsługi odbierania odpowiedzi od serwera
    std::thread receiveThread(ReceiveThread, clientSocket);

    while (true) {
        std::cout << "Wiadomość do wysłania (HEX) - naciśnij 'q', aby zakończyć: ";
        std::string message;
        std::cin >> message;

        if (message == "q") {
            break;
        }

        int bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == -1) {
            std::cerr << "Błąd podczas wysyłania danych." << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Odczekaj 1 sekundę przed kolejnym wprowadzeniem wiadomości
    }

    // Dołącz wątek obsługi odbierania odpowiedzi
    receiveThread.join();

    close(clientSocket);

    return 0;
}
