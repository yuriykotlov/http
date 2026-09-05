#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "win_server.hpp"

#include <iostream>
#include <format>
#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>

constexpr uint16_t BUFFER_LEN{512};

void process_connection(SOCKET &client_socket){
    int recv_result{};
    int send_result{};

    char buffer[BUFFER_LEN]{};

    // if recv_result returns more than 0, its the bytes it received

    do {
        recv_result = recv(client_socket, buffer, BUFFER_LEN, 0);
        
        if(recv_result == SOCKET_ERROR){
            std::cout << std::format("win_server | Couldn't recieve from client: {}\n", WSAGetLastError());
        } else if(recv_result == 0){
            std::cout << "win_server | Client disconnected.\n";
        }

        std::cout << std::format("win_server | Bytes recieved: {}\n", recv_result);

        send_result = send(client_socket, buffer, recv_result, 0);

        if(send_result == SOCKET_ERROR){
            std::cout << std::format("win_server | Send back to client failed: {}\n", WSAGetLastError());
        }

        std::cout << std::format("win_server | Sending bytes back to client: {}\n", send_result);

    } while(recv_result > 0);
}

SOCKET create_listen_socket(const char* const &PORT){
    struct addrinfo *result{ nullptr }, hints{};

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
    int feedback{ getaddrinfo(nullptr, PORT, &hints, &result) };
    if (feedback != 0) {
        std::cerr << std::format("win_server | getaddrinfo failed: {}\n", feedback);
        WSACleanup();
        return INVALID_SOCKET;
    }

    // listen for client connections
    SOCKET listen_socket{ socket(result->ai_family, result->ai_socktype, result->ai_protocol) };
    if (listen_socket == INVALID_SOCKET) {
        std::cerr << std::format("win_server | Listening socket is invalid: {}\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return INVALID_SOCKET;
    }

    // bind socket
    if(bind(listen_socket, result->ai_addr, static_cast<int>(result->ai_addrlen)) == SOCKET_ERROR){
        std::cerr << std::format("win_server | Binding socket failed: {}\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if(listen(listen_socket, SOMAXCONN) == SOCKET_ERROR){
        std::cerr << std::format("win_server | Socket failed to listen: {}\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    return listen_socket;
}

void win_server(const char* PORT){
    WSADATA wsaData{};

    // init
    int feedback{ WSAStartup(MAKEWORD(2, 2), &wsaData) };
    if (feedback != 0){
        std::cerr << std::format("win_server | WSAStartup failed: {}\n", feedback);
        WSACleanup();
        return;
    }

    std::cout << "\n### server started ###\n\n";

    SOCKET listen_socket{ create_listen_socket(PORT) };

    if(listen_socket == INVALID_SOCKET){
        std::cerr << "win_server | Could not get listen_socket.\n";
        WSACleanup();
        return;
    }

    std::cout << "win_server | init timeout\n";
    
    struct timeval timeout{};
    timeout.tv_sec = 15;
    timeout.tv_usec = 0;

    // sockets to be checked for activity
    fd_set set{};
    FD_SET(listen_socket, &set);

    int activity = select(0, &set, nullptr, nullptr, &timeout);
    if(activity == 0){
        std::cerr << "win_server | Timeout for client connection.\n";
        closesocket(listen_socket);
        WSACleanup();
        return;

    } else if(activity == SOCKET_ERROR){
        std::cerr << std::format("win_server | Select error: {}\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return;
    }
    
    std::cout << "win_server | Running WinSock, waiting for a connection!\n";

    // all good so now wait to accept a connection
    SOCKET new_connection{ accept(listen_socket, nullptr, nullptr) };
    if(new_connection == INVALID_SOCKET){
        std::cerr << "win_server | Failed to accept client socket: " << WSAGetLastError() << '\n';
        closesocket(listen_socket);
        WSACleanup();
    }
    
    // done listening for a client as we have connected now
    closesocket(listen_socket);
    
    process_connection(new_connection);

    closesocket(new_connection);
    WSACleanup();
}

#endif