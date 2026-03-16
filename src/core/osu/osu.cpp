#include "osu.hpp"

#include "binary/binary.hpp"

bool osu_db_parser::parse(std::string location) {
    if (data == nullptr) {
        return false;
    }

    this->location = std::move(location);
    return osu_binary::read_file_buffer(this->location, data->buffer);
}

bool osu_db_parser::write() {
    if (data == nullptr || location.empty()) {
        return false;
    }

    return osu_binary::write_file_buffer(location, data->buffer);
}

bool osu_collection_db_parser::parse(std::string location) {
    if (data == nullptr) {
        return false;
    }

    this->location = std::move(location);
    return osu_binary::read_file_buffer(this->location, data->buffer);
}

bool osu_collection_db_parser::write() {
    if (data == nullptr || location.empty()) {
        return false;
    }

    return osu_binary::write_file_buffer(location, data->buffer);
}

bool osu_scores_db_parser::parse(std::string location) {
    if (data == nullptr) {
        return false;
    }

    this->location = std::move(location);
    return osu_binary::read_file_buffer(this->location, data->buffer);
}

bool osu_scores_db_parser::write() {
    if (data == nullptr || location.empty()) {
        return false;
    }

    return osu_binary::write_file_buffer(location, data->buffer);
}

bool osu_replay_parser::parse(std::string location) {
    if (data == nullptr) {
        return false;
    }

    this->location = std::move(location);
    return osu_binary::read_file_buffer(this->location, data->buffer);
}

bool osu_replay_parser::write() {
    if (data == nullptr || location.empty()) {
        return false;
    }

    return osu_binary::write_file_buffer(location, data->buffer);
}
