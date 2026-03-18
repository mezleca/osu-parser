#pragma once

#include <mutex>
#include <string>

template <typename DataT, typename ParserT> struct parser_base {
    DataT data;
    ParserT parser;
    mutable std::mutex mutex;

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

    virtual void free_instance() {
        delete this;
    }
};
