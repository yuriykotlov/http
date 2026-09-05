#ifndef MAIN_HPP
#define MAIN_HPP

#include <future>
#include <utility>

std::future<void> create_thread(auto&& func, auto&& ... args){
    return std::async(
        std::launch::async,
        std::forward<decltype(func)>(func),
        std::forward<decltype(args)>(args)...
    );
}

#endif