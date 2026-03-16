#pragma once

#include <string>

template <typename DataT, typename ParserT>
struct parser_base {
    DataT data;
    ParserT parser;

    parser_base() {
        parser.data = &data;
    }

    virtual ~parser_base() = default;

    bool parse(const std::string& location) {
        return parser.parse(location);
    }

    bool write() {
        return parser.write();
    }

    virtual void free_instance() {
        delete this;
    }
};
