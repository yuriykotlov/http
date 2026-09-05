#include "main.hpp"

#ifdef _WIN32
    #include "win_server.hpp"
    #include "win_client.hpp"
#endif

#include <iostream>
#include <format>
#include <future>

const char* DEFAULT_PORT{ "27015" };

int main(int argc, char* argv[]){ 
    if(argc < 2){
        std::cerr << "argc < 2, client won't start\n";
    } else{
        std::cout << std::format(
        "args: count == {0},\nfirst arg == {1},\nsecond arg == {2}\n",
        argc,
        argv[1],
        argv[2]
    );
    }

    #ifdef _WIN32
        auto server_thread{ create_thread(win_server, DEFAULT_PORT) };
        auto client_thread{ create_thread(win_client, argc, argv, DEFAULT_PORT) };

        server_thread.get();
        client_thread.get();

        return 0;
    #else
        return 0;
    #endif
}