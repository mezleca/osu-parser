#include "osu.hpp"

#include "utils/binary.hpp"

#include <algorithm>
#include <stdexcept>

static osu_int_float_pair read_int_float_pair(osu_binary::binary_cursor& cursor, bool use_float) {
    osu_int_float_pair pair;

    uint8_t marker = osu_binary::read_u8(cursor);
    if (marker != 0x08) {
        throw std::runtime_error("invalid int-float pair marker");
    }

    pair.mod_combination = static_cast<int32_t>(osu_binary::read_i32(cursor));

    uint8_t float_marker = osu_binary::read_u8(cursor);
    if (use_float) {
        if (float_marker != 0x0C) {
            throw std::runtime_error("invalid float marker");
        }
        pair.star_rating = static_cast<double>(osu_binary::read_f32(cursor));
    } else {
        if (float_marker != 0x0D) {
            throw std::runtime_error("invalid double marker");
        }
        pair.star_rating = osu_binary::read_f64(cursor);
    }

    return pair;
}

static std::vector<osu_int_float_pair> read_star_ratings(osu_binary::binary_cursor& cursor, bool use_float) {
    std::vector<osu_int_float_pair> ratings;
    int32_t count = osu_binary::read_i32(cursor);

    if (count < 0) {
        throw std::runtime_error("invalid star rating count");
    }

    if (count == 0) {
        return ratings;
    }

    ratings.reserve(static_cast<size_t>(count));

    for (int32_t i = 0; i < count; i++) {
        ratings.push_back(read_int_float_pair(cursor, use_float));
    }

    return ratings;
}

static void write_int_float_pair(std::vector<uint8_t>& buffer, const osu_int_float_pair& pair, bool use_float) {
    osu_binary::write_u8(buffer, 0x08);
    osu_binary::write_i32(buffer, pair.mod_combination);

    if (use_float) {
        osu_binary::write_u8(buffer, 0x0C);
        osu_binary::write_f32(buffer, static_cast<float>(pair.star_rating));
    } else {
        osu_binary::write_u8(buffer, 0x0D);
        osu_binary::write_f64(buffer, pair.star_rating);
    }
}

static void write_star_ratings(std::vector<uint8_t>& buffer, const std::vector<osu_int_float_pair>& ratings,
                               bool use_float) {
    osu_binary::write_i32(buffer, static_cast<int32_t>(ratings.size()));

    for (const auto& rating : ratings) {
        write_int_float_pair(buffer, rating, use_float);
    }
}

static osu_db_beatmap read_beatmap(osu_binary::binary_cursor& cursor, int32_t version) {
    osu_db_beatmap beatmap;

    const bool has_entry_size = version < 20191106;
    const bool old_diff_format = version < 20140609;
    const bool use_float_star = version >= 20250107;
    size_t entry_start = 0;

    if (has_entry_size) {
        beatmap.entry_size = osu_binary::read_i32(cursor);
        entry_start = cursor.offset;
    }

    beatmap.artist = osu_binary::read_string(cursor);
    beatmap.artist_unicode = osu_binary::read_string(cursor);
    beatmap.title = osu_binary::read_string(cursor);
    beatmap.title_unicode = osu_binary::read_string(cursor);
    beatmap.creator = osu_binary::read_string(cursor);
    beatmap.difficulty = osu_binary::read_string(cursor);
    beatmap.audio_file_name = osu_binary::read_string(cursor);
    beatmap.md5 = osu_binary::read_string(cursor);
    beatmap.osu_file_name = osu_binary::read_string(cursor);
    beatmap.ranked_status = osu_binary::read_u8(cursor);
    beatmap.hitcircle = osu_binary::read_u16(cursor);
    beatmap.sliders = osu_binary::read_u16(cursor);
    beatmap.spinners = osu_binary::read_u16(cursor);
    beatmap.last_modification_time = osu_binary::read_i64(cursor);

    if (old_diff_format) {
        beatmap.approach_rate = osu_binary::read_u8(cursor);
        beatmap.circle_size = osu_binary::read_u8(cursor);
        beatmap.hp_drain = osu_binary::read_u8(cursor);
        beatmap.overall_difficulty = osu_binary::read_u8(cursor);
    } else {
        beatmap.approach_rate = osu_binary::read_f32(cursor);
        beatmap.circle_size = osu_binary::read_f32(cursor);
        beatmap.hp_drain = osu_binary::read_f32(cursor);
        beatmap.overall_difficulty = osu_binary::read_f32(cursor);
    }

    beatmap.slider_velocity = osu_binary::read_f64(cursor);

    if (!old_diff_format) {
        beatmap.star_rating_standard = read_star_ratings(cursor, use_float_star);
        beatmap.star_rating_taiko = read_star_ratings(cursor, use_float_star);
        beatmap.star_rating_ctb = read_star_ratings(cursor, use_float_star);
        beatmap.star_rating_mania = read_star_ratings(cursor, use_float_star);
    }

    beatmap.drain_time = osu_binary::read_i32(cursor);
    beatmap.total_time = osu_binary::read_i32(cursor);
    beatmap.audio_preview_time = osu_binary::read_i32(cursor);

    int32_t timing_count = osu_binary::read_i32(cursor);
    beatmap.timing_points.reserve(static_cast<size_t>(std::max(0, timing_count)));

    for (int32_t i = 0; i < timing_count; i++) {
        osu_db_timing_point tp;
        tp.bpm = osu_binary::read_f64(cursor);
        tp.offset = osu_binary::read_f64(cursor);
        tp.inherited = osu_binary::read_bool(cursor) ? 1 : 0;
        beatmap.timing_points.push_back(tp);
    }

    beatmap.difficulty_id = osu_binary::read_i32(cursor);
    beatmap.beatmap_id = osu_binary::read_i32(cursor);
    beatmap.thread_id = osu_binary::read_i32(cursor);
    beatmap.grade_standard = osu_binary::read_u8(cursor);
    beatmap.grade_taiko = osu_binary::read_u8(cursor);
    beatmap.grade_ctb = osu_binary::read_u8(cursor);
    beatmap.grade_mania = osu_binary::read_u8(cursor);
    beatmap.local_offset = osu_binary::read_i16(cursor);
    beatmap.stack_leniency = osu_binary::read_f32(cursor);
    beatmap.mode = osu_binary::read_u8(cursor);
    beatmap.source = osu_binary::read_string(cursor);
    beatmap.tags = osu_binary::read_string(cursor);
    beatmap.online_offset = osu_binary::read_i16(cursor);
    beatmap.title_font = osu_binary::read_string(cursor);
    beatmap.unplayed = osu_binary::read_bool(cursor) ? 1 : 0;
    beatmap.last_played = osu_binary::read_i64(cursor);
    beatmap.is_osz2 = osu_binary::read_bool(cursor) ? 1 : 0;
    beatmap.folder_name = osu_binary::read_string(cursor);
    beatmap.last_checked = osu_binary::read_i64(cursor);
    beatmap.ignore_sounds = osu_binary::read_bool(cursor) ? 1 : 0;
    beatmap.ignore_skin = osu_binary::read_bool(cursor) ? 1 : 0;
    beatmap.disable_storyboard = osu_binary::read_bool(cursor) ? 1 : 0;
    beatmap.disable_video = osu_binary::read_bool(cursor) ? 1 : 0;
    beatmap.visual_override = osu_binary::read_bool(cursor) ? 1 : 0;

    if (old_diff_format) {
        beatmap.unknown = osu_binary::read_i16(cursor);
    }

    beatmap.last_modified = osu_binary::read_i32(cursor);
    beatmap.mania_scroll_speed = osu_binary::read_u8(cursor);

    if (has_entry_size && beatmap.entry_size.has_value()) {
        const size_t bytes_read = cursor.offset - entry_start;
        const int32_t entry_size = beatmap.entry_size.value();
        if (entry_size > 0 && static_cast<size_t>(entry_size) > bytes_read) {
            osu_binary::skip(cursor, static_cast<size_t>(entry_size) - bytes_read);
        }
    }

    return beatmap;
}

bool osu_db_parser::parse(const std::string& location) {
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

        data->version = osu_binary::read_i32(cursor);
        data->folder_count = osu_binary::read_i32(cursor);
        data->account_unlocked = osu_binary::read_bool(cursor) ? 1 : 0;
        data->account_unlock_time = osu_binary::read_i64(cursor);
        data->player_name = osu_binary::read_string(cursor);
        data->beatmaps_count = osu_binary::read_i32(cursor);

        data->beatmaps.clear();
        data->beatmaps.reserve(static_cast<size_t>(std::max(0, data->beatmaps_count)));

        for (int32_t i = 0; i < data->beatmaps_count; i++) {
            data->beatmaps.push_back(read_beatmap(cursor, data->version));
        }

        data->permissions = osu_binary::read_i32(cursor);
        last_error.clear();
        return true;
    } catch (const std::exception& e) {
        last_error = e.what();
        *data = osu_legacy_database();
        return false;
    } catch (...) {
        last_error = "unknown error";
        *data = osu_legacy_database();
        return false;
    }
}

bool osu_db_parser::write() {
    if (data == nullptr || location.empty()) {
        last_error = data == nullptr ? "parser data is null" : "location is empty";
        return false;
    }

    std::vector<uint8_t> buffer;
    buffer.reserve(1024);

    const int32_t version = data->version;
    const bool has_entry_size = version < 20191106;
    const bool old_diff_format = version < 20140609;
    const bool use_float_star = version >= 20250107;

    const int32_t beatmaps_count = static_cast<int32_t>(data->beatmaps.size());

    osu_binary::write_i32(buffer, version);
    osu_binary::write_i32(buffer, data->folder_count);
    osu_binary::write_bool(buffer, data->account_unlocked != 0);
    osu_binary::write_i64(buffer, data->account_unlock_time);
    osu_binary::write_string(buffer, data->player_name);
    osu_binary::write_i32(buffer, beatmaps_count);

    for (const auto& beatmap : data->beatmaps) {
        std::vector<uint8_t> entry;
        entry.reserve(512);

        osu_binary::write_string(entry, beatmap.artist);
        osu_binary::write_string(entry, beatmap.artist_unicode);
        osu_binary::write_string(entry, beatmap.title);
        osu_binary::write_string(entry, beatmap.title_unicode);
        osu_binary::write_string(entry, beatmap.creator);
        osu_binary::write_string(entry, beatmap.difficulty);
        osu_binary::write_string(entry, beatmap.audio_file_name);
        osu_binary::write_string(entry, beatmap.md5);
        osu_binary::write_string(entry, beatmap.osu_file_name);
        osu_binary::write_u8(entry, static_cast<uint8_t>(beatmap.ranked_status));
        osu_binary::write_u16(entry, static_cast<uint16_t>(beatmap.hitcircle));
        osu_binary::write_u16(entry, static_cast<uint16_t>(beatmap.sliders));
        osu_binary::write_u16(entry, static_cast<uint16_t>(beatmap.spinners));
        osu_binary::write_i64(entry, beatmap.last_modification_time);

        if (old_diff_format) {
            osu_binary::write_u8(entry, static_cast<uint8_t>(beatmap.approach_rate));
            osu_binary::write_u8(entry, static_cast<uint8_t>(beatmap.circle_size));
            osu_binary::write_u8(entry, static_cast<uint8_t>(beatmap.hp_drain));
            osu_binary::write_u8(entry, static_cast<uint8_t>(beatmap.overall_difficulty));
        } else {
            osu_binary::write_f32(entry, static_cast<float>(beatmap.approach_rate));
            osu_binary::write_f32(entry, static_cast<float>(beatmap.circle_size));
            osu_binary::write_f32(entry, static_cast<float>(beatmap.hp_drain));
            osu_binary::write_f32(entry, static_cast<float>(beatmap.overall_difficulty));
        }

        osu_binary::write_f64(entry, beatmap.slider_velocity);

        if (!old_diff_format) {
            write_star_ratings(entry, beatmap.star_rating_standard, use_float_star);
            write_star_ratings(entry, beatmap.star_rating_taiko, use_float_star);
            write_star_ratings(entry, beatmap.star_rating_ctb, use_float_star);
            write_star_ratings(entry, beatmap.star_rating_mania, use_float_star);
        }

        osu_binary::write_i32(entry, beatmap.drain_time);
        osu_binary::write_i32(entry, beatmap.total_time);
        osu_binary::write_i32(entry, beatmap.audio_preview_time);

        osu_binary::write_i32(entry, static_cast<int32_t>(beatmap.timing_points.size()));

        for (const auto& timing : beatmap.timing_points) {
            osu_binary::write_f64(entry, timing.bpm);
            osu_binary::write_f64(entry, timing.offset);
            osu_binary::write_bool(entry, timing.inherited != 0);
        }

        osu_binary::write_i32(entry, beatmap.difficulty_id);
        osu_binary::write_i32(entry, beatmap.beatmap_id);
        osu_binary::write_i32(entry, beatmap.thread_id);
        osu_binary::write_u8(entry, static_cast<uint8_t>(beatmap.grade_standard));
        osu_binary::write_u8(entry, static_cast<uint8_t>(beatmap.grade_taiko));
        osu_binary::write_u8(entry, static_cast<uint8_t>(beatmap.grade_ctb));
        osu_binary::write_u8(entry, static_cast<uint8_t>(beatmap.grade_mania));
        osu_binary::write_i16(entry, static_cast<int16_t>(beatmap.local_offset));
        osu_binary::write_f32(entry, static_cast<float>(beatmap.stack_leniency));
        osu_binary::write_u8(entry, static_cast<uint8_t>(beatmap.mode));
        osu_binary::write_string(entry, beatmap.source);
        osu_binary::write_string(entry, beatmap.tags);
        osu_binary::write_i16(entry, static_cast<int16_t>(beatmap.online_offset));
        osu_binary::write_string(entry, beatmap.title_font);
        osu_binary::write_bool(entry, beatmap.unplayed != 0);
        osu_binary::write_i64(entry, beatmap.last_played);
        osu_binary::write_bool(entry, beatmap.is_osz2 != 0);
        osu_binary::write_string(entry, beatmap.folder_name);
        osu_binary::write_i64(entry, beatmap.last_checked);
        osu_binary::write_bool(entry, beatmap.ignore_sounds != 0);
        osu_binary::write_bool(entry, beatmap.ignore_skin != 0);
        osu_binary::write_bool(entry, beatmap.disable_storyboard != 0);
        osu_binary::write_bool(entry, beatmap.disable_video != 0);
        osu_binary::write_bool(entry, beatmap.visual_override != 0);

        if (old_diff_format) {
            osu_binary::write_i16(entry, static_cast<int16_t>(beatmap.unknown.value_or(0)));
        }

        osu_binary::write_i32(entry, beatmap.last_modified);
        osu_binary::write_u8(entry, static_cast<uint8_t>(beatmap.mania_scroll_speed));

        if (has_entry_size) {
            osu_binary::write_i32(buffer, static_cast<int32_t>(entry.size()));
        }
        buffer.insert(buffer.end(), entry.begin(), entry.end());
    }

    osu_binary::write_i32(buffer, data->permissions);

    if (!osu_binary::write_file_buffer(location, buffer)) {
        last_error = "failed to write file";
        return false;
    }

    last_error.clear();
    return true;
}
