#include "osu.hpp"

#include "utils/binary.hpp"

#include <algorithm>

bool osu_replay_parser::parse(const std::string& location) {
    if (data == nullptr) {
        last_error = "parser data is null";
        return false;
    }

    this->location = location;
    std::vector<uint8_t> buffer;
    if (!osu_binary::read_file_buffer(this->location, buffer)) {
        last_error = "failed to read file";
        return false;
    }

    try {
        osu_binary::binary_cursor cursor;
        osu_binary::set_cursor(cursor, buffer);

        data->mode = osu_binary::read_u8(cursor);
        data->version = osu_binary::read_i32(cursor);
        data->beatmap_md5 = osu_binary::read_string(cursor);
        data->player_name = osu_binary::read_string(cursor);
        data->replay_md5 = osu_binary::read_string(cursor);
        data->count_300 = osu_binary::read_i16(cursor);
        data->count_100 = osu_binary::read_i16(cursor);
        data->count_50 = osu_binary::read_i16(cursor);
        data->count_geki = osu_binary::read_i16(cursor);
        data->count_katu = osu_binary::read_i16(cursor);
        data->count_miss = osu_binary::read_i16(cursor);
        data->score = osu_binary::read_i32(cursor);
        data->max_combo = osu_binary::read_i16(cursor);
        data->perfect = osu_binary::read_u8(cursor);
        data->mods = osu_binary::read_i32(cursor);
        data->life_bar_graph = osu_binary::read_string(cursor);
        data->timestamp = osu_binary::read_i64(cursor);
        data->replay_data_length = osu_binary::read_i32(cursor);

        if (data->replay_data_length > 0) {
            osu_binary::ensure_range(cursor, static_cast<size_t>(data->replay_data_length));
            data->replay_data.assign(buffer.begin() + static_cast<std::ptrdiff_t>(cursor.offset),
                                     buffer.begin() +
                                         static_cast<std::ptrdiff_t>(cursor.offset + data->replay_data_length));
            cursor.offset += static_cast<size_t>(data->replay_data_length);
        } else {
            data->replay_data.clear();
        }

        data->online_score_id = osu_binary::read_i64(cursor);
        if (data->mods & (1 << 23)) {
            data->additional_mod_info = osu_binary::read_f64(cursor);
        } else {
            data->additional_mod_info = std::nullopt;
        }

        last_error.clear();
        return true;
    } catch (const std::exception& e) {
        last_error = e.what();
        *data = osu_replay();
        return false;
    } catch (...) {
        last_error = "unknown error";
        *data = osu_replay();
        return false;
    }
}

bool osu_replay_parser::write() {
    if (data == nullptr || location.empty()) {
        last_error = data == nullptr ? "parser data is null" : "location is empty";
        return false;
    }

    std::vector<uint8_t> buffer;
    osu_binary::write_u8(buffer, static_cast<uint8_t>(data->mode));
    osu_binary::write_i32(buffer, data->version);
    osu_binary::write_string(buffer, data->beatmap_md5);
    osu_binary::write_string(buffer, data->player_name);
    osu_binary::write_string(buffer, data->replay_md5);
    osu_binary::write_i16(buffer, static_cast<int16_t>(data->count_300));
    osu_binary::write_i16(buffer, static_cast<int16_t>(data->count_100));
    osu_binary::write_i16(buffer, static_cast<int16_t>(data->count_50));
    osu_binary::write_i16(buffer, static_cast<int16_t>(data->count_geki));
    osu_binary::write_i16(buffer, static_cast<int16_t>(data->count_katu));
    osu_binary::write_i16(buffer, static_cast<int16_t>(data->count_miss));
    osu_binary::write_i32(buffer, data->score);
    osu_binary::write_i16(buffer, static_cast<int16_t>(data->max_combo));
    osu_binary::write_u8(buffer, static_cast<uint8_t>(data->perfect));
    osu_binary::write_i32(buffer, data->mods);
    osu_binary::write_string(buffer, data->life_bar_graph);
    osu_binary::write_i64(buffer, data->timestamp);

    const int32_t replay_size =
        data->replay_data.empty() ? data->replay_data_length : static_cast<int32_t>(data->replay_data.size());
    osu_binary::write_i32(buffer, replay_size);
    if (replay_size > 0) {
        if (data->replay_data.size() >= static_cast<size_t>(replay_size)) {
            buffer.insert(buffer.end(), data->replay_data.begin(), data->replay_data.begin() + replay_size);
        } else {
            buffer.insert(buffer.end(), data->replay_data.begin(), data->replay_data.end());
            buffer.resize(buffer.size() + static_cast<size_t>(replay_size - data->replay_data.size()), 0);
        }
    }

    osu_binary::write_i64(buffer, data->online_score_id);
    if (data->mods & (1 << 23)) {
        osu_binary::write_f64(buffer, data->additional_mod_info.value_or(0.0));
    }

    if (!osu_binary::write_file_buffer(location, buffer)) {
        last_error = "failed to write file";
        return false;
    }

    last_error.clear();
    return true;
}
