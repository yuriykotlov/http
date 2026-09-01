#include "win.hpp"

#include <iostream>
#include <format>

#ifdef _WIN32
    #include "win.hpp"
#endif

// constexpr applies to the const pointer
constexpr const char* DEFAULT_PORT{ "27015" };

constexpr int* a{};

int main(){
    #ifdef _WIN32
        return win(DEFAULT_PORT);
    #else
        return 0;
    #endif
}