#pragma once

#include <atomic>
#include <mutex>
#include <string>

template <typename DataT, typename ParserT> struct parser_base {
    DataT data;
    ParserT parser;
    mutable std::mutex mutex;
    std::atomic<bool> freed{false};

    parser_base() {
        parser.data = &data;
    }

    virtual ~parser_base() = default;

    bool parse(const std::string& location) {
        std::lock_guard<std::mutex> lock(mutex);
        return parser.parse(location);
    }

    bool write() {
        std::lock_guard<std::mutex> lock(mutex);
        return parser.write();
    }

    template <typename Fn> auto with_lock(Fn fn) -> decltype(fn(data, parser)) {
        std::lock_guard<std::mutex> lock(mutex);
        return fn(data, parser);
    }

    virtual void free_instance() {
        if (freed.exchange(true)) {
            return;
        }

        delete this;
    }
};
