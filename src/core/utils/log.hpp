#pragma once

#ifdef __EMSCRIPTEN__
#define OSU_PARSER_LOG(...)                                                                                            \
    do {                                                                                                               \
    } while (0)
#define OSU_PARSER_LOG_LINE(...)                                                                                       \
    do {                                                                                                               \
    } while (0)

#define LOG(...) OSU_PARSER_LOG(__VA_ARGS__)
#define LOG_LINE(...) OSU_PARSER_LOG_LINE(__VA_ARGS__)
#else
#include <iostream>

inline void log_print() {
}

template <typename T> inline void log_print(const T& value) {
    std::cout << value;
}

template <typename T, typename... Args> inline void log_print(const T& value, const Args&... args) {
    std::cout << value << " ";
    log_print(args...);
}

#define LOG(...)                                                                                                       \
    do {                                                                                                               \
        log_print(__VA_ARGS__);                                                                                        \
    } while (0)
#define LOG_LINE(...)                                                                                                  \
    do {                                                                                                               \
        log_print(__VA_ARGS__);                                                                                        \
        std::cout << '\n';                                                                                             \
    } while (0)

#define OSU_PARSER_LOG(...)                                                                                            \
    do {                                                                                                               \
        log_print(__VA_ARGS__);                                                                                        \
    } while (0)
#define OSU_PARSER_LOG_LINE(...)                                                                                       \
    do {                                                                                                               \
        log_print(__VA_ARGS__);                                                                                        \
        std::cout << '\n';                                                                                             \
    } while (0)
#endif
