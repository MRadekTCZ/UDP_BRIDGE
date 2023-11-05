//////////////
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <chrono>

#define CLIENT_PORT 1777
#define SERVER_PORT 4077
#define SERVER_IP "192.168.3.247"



// Another thread for receiving data via udp
void ReceiveThread(int clientSocket) {
    char buffer[1024];
    sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(serverAddr);

    while (true) {
        int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddr, &addrLen);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "Response: " << buffer << std::endl;
        }
        else {
            std::cerr << "Error receiving data." << std::endl;
            break;
        }
    }
}

int main() {
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(CLIENT_PORT); 
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    // Binding socket with port number
    if (bind(clientSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == -1) {
        std::cerr << "Port error." << std::endl;
        close(clientSocket);
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "IP error" << std::endl;
        close(clientSocket);
        return 1;
    }

    // Thread with receving data from server - start
    std::thread receiveThread(ReceiveThread, clientSocket);

    while (true) {
        std::cout << "Send message: (press 'q' to leave)";
        std::string message;
        std::getline (std::cin,message);

        if (message == "q") {
            break;
        }

        int bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == -1) {
            std::cerr << "Error sending data." << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); //1 sec delay between asking

    
    receiveThread.join();

    close(clientSocket);

    return 0;
}
