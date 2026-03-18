#include "osu.hpp"
#include "utils/binary.hpp"

#include <algorithm>

bool osu_collection_db_parser::parse(const std::string& location) {
    if (data == nullptr) {
        last_error = "parser data is null";
        return false;
    }

    std::vector<uint8_t> buffer;

    if (!osu_binary::read_file_buffer(location, buffer)) {
        last_error = "failed to read file";
        return false;
    }

    try {
        this->location = location;
        osu_binary::binary_cursor cursor;
        osu_binary::set_cursor(cursor, buffer);

        data->version = osu_binary::read_i32(cursor);
        data->collections_count = osu_binary::read_i32(cursor);
        if (data->collections_count < 0) {
            throw std::runtime_error("invalid collections count");
        }
        data->collections.clear();
        data->collections.reserve(static_cast<size_t>(std::max(0, data->collections_count)));

        for (int32_t i = 0; i < data->collections_count; i++) {
            osu_collection collection;
            collection.name = osu_binary::read_string(cursor);
            collection.beatmaps_count = osu_binary::read_i32(cursor);
            if (collection.beatmaps_count < 0) {
                throw std::runtime_error("invalid collection beatmaps count");
            }
            collection.beatmap_md5.clear();
            collection.beatmap_md5.reserve(static_cast<size_t>(std::max(0, collection.beatmaps_count)));

            for (int32_t j = 0; j < collection.beatmaps_count; j++) {
                collection.beatmap_md5.push_back(osu_binary::read_string(cursor));
            }

            data->collections.push_back(std::move(collection));
        }

        last_error.clear();
        return true;
    } catch (const std::exception& e) {
        last_error = e.what();
        *data = osu_collection_db();
        return false;
    } catch (...) {
        last_error = "unknown error";
        *data = osu_collection_db();
        return false;
    }
}

bool osu_collection_db_parser::write() {
    if (data == nullptr || location.empty()) {
        last_error = data == nullptr ? "parser data is null" : "location is empty";
        return false;
    }

    std::vector<uint8_t> buffer;
    buffer.reserve(1024);

    data->collections_count = static_cast<int32_t>(data->collections.size());

    osu_binary::write_i32(buffer, data->version);
    osu_binary::write_i32(buffer, data->collections_count);

    for (auto& collection : data->collections) {
        collection.beatmaps_count = static_cast<int32_t>(collection.beatmap_md5.size());
        osu_binary::write_string(buffer, collection.name);
        osu_binary::write_i32(buffer, static_cast<int32_t>(collection.beatmap_md5.size()));

        for (const auto& checksum : collection.beatmap_md5) {
            osu_binary::write_string(buffer, checksum);
        }
    }

    if (!osu_binary::write_file_buffer(location, buffer)) {
        last_error = "failed to write file";
        return false;
    }

    last_error.clear();
    return true;
}
