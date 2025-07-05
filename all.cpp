#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

bool running = true;

void receiveMessages(int sockfd) {
    char buffer[1024];
    while (running) {
        ssize_t bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            cout << "Соединение закрыто или ошибка." << endl;
            running = false;
            break;
        }
        buffer[bytes] = '\0';
        cout << "Получено: " << buffer << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Использование: " << argv[0] << " <mode> <local_port> <peer_ip>:<peer_port>\n";
        cout << "mode: 'server' или 'client'\n";
        return 1;
    }

    string mode = argv[1];
    int local_port = stoi(argv[2]);
    string peer_info = argv[3];

    // Создаем сокет
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in local_addr{}, peer_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(local_port);

    if (bind(sockfd, (sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    if (mode == "server") {
        // В режиме сервера слушаем входящие соединения
        listen(sockfd, 1);
        cout << "Ожидание входящего соединения..." << endl;
        socklen_t addr_len = sizeof(peer_addr);
        int client_sock = accept(sockfd, (sockaddr*)&peer_addr, &addr_len);
        if (client_sock < 0) {
            perror("accept");
            close(sockfd);
            return 1;
        }
        close(sockfd); // закрываем слушающий сокет
        sockfd = client_sock;
        cout << "Соединение установлено." << endl;
    } else if (mode == "client") {
        // В режиме клиента подключаемся к серверу
        size_t colon_pos = peer_info.find(':');
        if (colon_pos == string::npos) {
            cout << "Некорректный формат peer_ip:peer_port\n";
            close(sockfd);
            return 1;
        }
        string peer_ip = peer_info.substr(0, colon_pos);
        int peer_port = stoi(peer_info.substr(colon_pos + 1));

        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = htons(peer_port);
        inet_pton(AF_INET, peer_ip.c_str(), &peer_addr.sin_addr);

        cout << "Подключение к " << peer_ip << ":" << peer_port << "..." << endl;
        if (connect(sockfd, (sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
            perror("connect");
            close(sockfd);
            return 1;
        }
        cout << "Соединение установлено." << endl;
    } else {
        cout << "Некорректный режим. Используйте 'server' или 'client'.\n";
        close(sockfd);
        return 1;
    }

    // Запускаем поток для приема сообщений
    thread receiver(receiveMessages, sockfd);

    // Основной цикл отправки сообщений
    string message;
    while (running) {
        getline(cin, message);
        if (message == "/quit") {
            running = false;
            break;
        }
        send(sockfd, message.c_str(), message.size(), 0);
    }

    close(sockfd);
    receiver.join();

    return 0;
}
