#include "osu.hpp"

#include "utils/binary.hpp"

#include <algorithm>

bool osu_scores_db_parser::parse(std::string location) {
    if (data == nullptr) {
        last_error = "parser data is null";
        return false;
    }

    this->location = std::move(location);
    std::vector<uint8_t> buffer;
    if (!osu_binary::read_file_buffer(this->location, buffer)) {
        last_error = "failed to read file";
        return false;
    }

    try {
        osu_binary::binary_cursor cursor;
        osu_binary::set_cursor(cursor, buffer);

        data->version = osu_binary::read_i32(cursor);
        data->beatmaps_count = osu_binary::read_i32(cursor);
        data->beatmaps.clear();
        data->beatmaps.reserve(static_cast<size_t>(std::max(0, data->beatmaps_count)));

        for (int32_t i = 0; i < data->beatmaps_count; i++) {
            osu_scores_beatmap beatmap;
            beatmap.beatmap_md5 = osu_binary::read_string(cursor);
            beatmap.scores_count = osu_binary::read_i32(cursor);
            beatmap.scores.clear();
            beatmap.scores.reserve(static_cast<size_t>(std::max(0, beatmap.scores_count)));

            for (int32_t j = 0; j < beatmap.scores_count; j++) {
                osu_score score;
                score.mode = osu_binary::read_u8(cursor);
                score.version = osu_binary::read_i32(cursor);
                score.beatmap_md5 = osu_binary::read_string(cursor);
                score.player_name = osu_binary::read_string(cursor);
                score.replay_md5 = osu_binary::read_string(cursor);
                score.count_300 = osu_binary::read_i16(cursor);
                score.count_100 = osu_binary::read_i16(cursor);
                score.count_50 = osu_binary::read_i16(cursor);
                score.count_geki = osu_binary::read_i16(cursor);
                score.count_katu = osu_binary::read_i16(cursor);
                score.count_miss = osu_binary::read_i16(cursor);
                score.score = osu_binary::read_i32(cursor);
                score.max_combo = osu_binary::read_i16(cursor);
                score.perfect = osu_binary::read_bool(cursor) ? 1 : 0;
                score.mods = osu_binary::read_i32(cursor);
                score.life_bar_graph = osu_binary::read_string(cursor);
                score.timestamp = osu_binary::read_i64(cursor);
                score.replay_data_length = osu_binary::read_i32(cursor);
                score.replay_data.clear();
                if (score.replay_data_length > 0) {
                    osu_binary::skip(cursor, static_cast<size_t>(score.replay_data_length));
                }
                score.online_score_id = osu_binary::read_i64(cursor);

                if (score.mods & (1 << 23)) {
                    score.additional_mod_info = osu_binary::read_f64(cursor);
                } else {
                    score.additional_mod_info = std::nullopt;
                }

                beatmap.scores.push_back(std::move(score));
            }

            data->beatmaps.push_back(std::move(beatmap));
        }

        last_error.clear();
        return true;
    } catch (const std::exception& e) {
        last_error = e.what();
        *data = osu_scores_db();
        return false;
    } catch (...) {
        last_error = "unknown error";
        *data = osu_scores_db();
        return false;
    }
}

bool osu_scores_db_parser::write() {
    if (data == nullptr || location.empty()) {
        last_error = data == nullptr ? "parser data is null" : "location is empty";
        return false;
    }

    std::vector<uint8_t> buffer;
    buffer.reserve(1024);

    osu_binary::write_i32(buffer, data->version);
    osu_binary::write_i32(buffer, static_cast<int32_t>(data->beatmaps.size()));

    for (const auto& beatmap : data->beatmaps) {
        osu_binary::write_string(buffer, beatmap.beatmap_md5);
        osu_binary::write_i32(buffer, static_cast<int32_t>(beatmap.scores.size()));

        for (const auto& score : beatmap.scores) {
            osu_binary::write_u8(buffer, static_cast<uint8_t>(score.mode));
            osu_binary::write_i32(buffer, score.version);
            osu_binary::write_string(buffer, score.beatmap_md5);
            osu_binary::write_string(buffer, score.player_name);
            osu_binary::write_string(buffer, score.replay_md5);
            osu_binary::write_i16(buffer, static_cast<int16_t>(score.count_300));
            osu_binary::write_i16(buffer, static_cast<int16_t>(score.count_100));
            osu_binary::write_i16(buffer, static_cast<int16_t>(score.count_50));
            osu_binary::write_i16(buffer, static_cast<int16_t>(score.count_geki));
            osu_binary::write_i16(buffer, static_cast<int16_t>(score.count_katu));
            osu_binary::write_i16(buffer, static_cast<int16_t>(score.count_miss));
            osu_binary::write_i32(buffer, score.score);
            osu_binary::write_i16(buffer, static_cast<int16_t>(score.max_combo));
            osu_binary::write_bool(buffer, score.perfect != 0);
            osu_binary::write_i32(buffer, score.mods);
            osu_binary::write_string(buffer, score.life_bar_graph);
            osu_binary::write_i64(buffer, score.timestamp);
            int32_t replay_len = score.replay_data_length;
            if (replay_len > 0) {
                replay_len = -1;
            }
            osu_binary::write_i32(buffer, replay_len);
            osu_binary::write_i64(buffer, score.online_score_id);

            if (score.mods & (1 << 23)) {
                osu_binary::write_f64(buffer, score.additional_mod_info.value_or(0.0));
            }
        }
    }

    if (!osu_binary::write_file_buffer(location, buffer)) {
        last_error = "failed to write file";
        return false;
    }

    last_error.clear();
    return true;
}
