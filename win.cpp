#ifdef _WIN32

#include "win.hpp"

#include <iostream>
#include <format>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

constexpr uint16_t BUFFER_LEN{512};

int process_client(SOCKET &client_socket){
    int recv_result{};
    int send_result{};

    char buffer[BUFFER_LEN]{};

    // if recv_result returns more than 0, its the bytes it received

    do {
        recv_result = recv(client_socket, buffer, BUFFER_LEN, 0);
        
        if(recv_result == SOCKET_ERROR){
            std::cerr << "Couldn't recieve from client: " << WSAGetLastError() << '\n';
            return 1;
        } else if(recv_result == 0){
            std::cout << "Client disconnected.\n";
            return 0;
        }

        std::cout << std::format("Bytes recieved: {}\n", recv_result);

        send_result = send(client_socket, buffer, recv_result, 0);

        if(send_result == SOCKET_ERROR){
            std::cerr << "Send back to client failed: " << WSAGetLastError() << '\n';
            return 1;
        }

        std::cout << std::format("Sending bytes back to client: {}", send_result);

    } while(recv_result > 0);

    return 0;
}

int win(const char* PORT){
    WSADATA wsaData{};
    
    struct addrinfo *result{ nullptr }, hints{};

    // init
    int feedback{ WSAStartup(MAKEWORD(2, 2), &wsaData) };
    if (feedback != 0){
        std::cerr << std::format("WSAStartup failed: {}\n", feedback);

        WSACleanup();
        return 1;
    }

    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // tcp
    hints.ai_protocol = IPPROTO_TCP; // tcp
    /*
        passive = this is server
        numericserv = passing in port directly so no
                      string->port translation needed
    */
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    
    // resolve server address and port
    feedback = getaddrinfo(nullptr, std::string{PORT}.c_str(), &hints, &result);
    if (feedback != 0) {
        std::cerr << std::format("getaddrinfo failed: {}\n", feedback);
        WSACleanup();
        return 1;
    }

    // listen for client connections
    SOCKET listen_socket{ socket(result->ai_family, result->ai_socktype, result->ai_protocol) };
    if (listen_socket == INVALID_SOCKET) {
        std::cerr << "Listening socket is invalid: " << WSAGetLastError() << '\n';
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // bind socket
    feedback = bind(listen_socket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if(feedback == SOCKET_ERROR){
        std::cerr << "Binding socket failed: " << WSAGetLastError() << '\n';
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    feedback = listen(listen_socket, SOMAXCONN);
    if(feedback == SOCKET_ERROR){
        std::cerr << "Socket failed to listen: " << WSAGetLastError() << '\n';
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }
    
    std::cout << "Running WinSock, waiting for client!";

    // all good so now wait to accept a client connection

    SOCKET client_socket = accept(listen_socket, nullptr, nullptr);
    if(client_socket == INVALID_SOCKET){
        std::cerr << "Failed to accept client socket: " << WSAGetLastError() << '\n';
        closesocket(listen_socket);
        return 1;
    }
    
    // done listening for a client as we have connected now
    closesocket(listen_socket);
    
    int process_result = process_client(client_socket);

    closesocket(client_socket);
    WSACleanup();

    return process_result;
}

#endif