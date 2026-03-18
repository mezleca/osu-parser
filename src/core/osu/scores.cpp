#include "osu.hpp"

#include "utils/binary.hpp"

bool osu_scores_db_parser::parse(std::string location) {
    if (data == nullptr) {
        last_error = "parser data is null";
        return false;
    }

    this->location = std::move(location);
    std::vector<uint8_t> temp;
    if (!osu_binary::read_file_buffer(this->location, temp)) {
        last_error = "failed to read file";
        return false;
    }

    last_error.clear();
    return true;
}

bool osu_scores_db_parser::write() {
    if (data == nullptr || location.empty()) {
        last_error = data == nullptr ? "parser data is null" : "location is empty";
        return false;
    }

    last_error = "write not implemented";
    return false;
}
