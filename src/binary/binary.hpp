#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace osu_binary {
    bool read_file_buffer(const std::string& location, std::vector<uint8_t>& out);
    bool write_file_buffer(const std::string& location, const std::vector<uint8_t>& buffer);
};
