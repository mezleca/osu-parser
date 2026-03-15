#include "binary.hpp"

#include <fstream>

namespace osu_binary {
    bool read_file_buffer(const std::string& location, std::vector<uint8_t>& out) {
        std::ifstream file(location, std::ios::binary | std::ios::ate);

        if (!file.is_open()) {
            return false;
        }

        const std::ifstream::pos_type size = file.tellg();

        if (size < 0) {
            return false;
        }

        out.resize(static_cast<size_t>(size));
        file.seekg(0, std::ios::beg);

        if (!file.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(out.size()))) {
            out.clear();
            return false;
        }

        return true;
    }

    bool write_file_buffer(const std::string& location, const std::vector<uint8_t>& buffer) {
        std::ofstream file(location, std::ios::binary | std::ios::trunc);

        if (!file.is_open()) {
            return false;
        }

        if (!buffer.empty()) {
            file.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        }

        return file.good();
    }
}
