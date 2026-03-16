#include "beatmap.hpp"

#include "utils/binary.hpp"

bool beatmap_parser::parse(std::string location) {
    if (data == nullptr) {
        return false;
    }

    this->location = std::move(location);
    return osu_binary::read_file_buffer(this->location, buffer);
}

bool beatmap_parser::write() {
    if (data == nullptr || location.empty()) {
        return false;
    }

    return osu_binary::write_file_buffer(location, buffer);
}
