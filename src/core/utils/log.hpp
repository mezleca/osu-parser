#pragma once

#ifdef __EMSCRIPTEN__
    #define LOG(...)
    #define LOG_LINE(...)
#else
    #include <iostream>

    template<typename T>
    void log_print(const T& value) {
        std::cout << value;
    }

    template<typename T, typename... Args>
    void log_print(const T& value, const Args&... args) {
        std::cout << value << " ";
        log_print(args...);
    }

    #define LOG(...) log_print(__VA_ARGS__)
    #define LOG_LINE(...) do { log_print(__VA_ARGS__); std::cout << std::endl; } while(0)
#endif
