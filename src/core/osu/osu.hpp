#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// === shared ===
struct osu_int_float_pair {
    int32_t mod_combination = 0;
    double star_rating = 0.0;
};

struct osu_db_timing_point {
    double bpm = 0.0;
    double offset = 0.0;
    int32_t inherited = 0;
};

// === osu!.db ===
struct osu_db_beatmap {
    std::optional<int32_t> entry_size;
    std::string artist;
    std::string artist_unicode;
    std::string title;
    std::string title_unicode;
    std::string creator;
    std::string difficulty;
    std::string audio_file_name;
    std::string md5;
    std::string osu_file_name;
    int32_t ranked_status = 0;
    int32_t hitcircle = 0;
    int32_t sliders = 0;
    int32_t spinners = 0;
    int64_t last_modification_time = 0;
    double approach_rate = 0.0;
    double circle_size = 0.0;
    double hp_drain = 0.0;
    double overall_difficulty = 0.0;
    double slider_velocity = 0.0;
    std::vector<osu_int_float_pair> star_rating_standard;
    std::vector<osu_int_float_pair> star_rating_taiko;
    std::vector<osu_int_float_pair> star_rating_ctb;
    std::vector<osu_int_float_pair> star_rating_mania;
    int32_t drain_time = 0;
    int32_t total_time = 0;
    int32_t audio_preview_time = 0;
    std::vector<osu_db_timing_point> timing_points;
    int32_t difficulty_id = 0;
    int32_t beatmap_id = 0;
    int32_t thread_id = 0;
    int32_t grade_standard = 0;
    int32_t grade_taiko = 0;
    int32_t grade_ctb = 0;
    int32_t grade_mania = 0;
    int32_t local_offset = 0;
    double stack_leniency = 0.0;
    int32_t mode = 0;
    std::string source;
    std::string tags;
    int32_t online_offset = 0;
    std::string title_font;
    int32_t unplayed = 0;
    int64_t last_played = 0;
    int32_t is_osz2 = 0;
    std::string folder_name;
    int64_t last_checked = 0;
    int32_t ignore_sounds = 0;
    int32_t ignore_skin = 0;
    int32_t disable_storyboard = 0;
    int32_t disable_video = 0;
    int32_t visual_override = 0;
    std::optional<int32_t> unknown;
    int32_t last_modified = 0;
    int32_t mania_scroll_speed = 0;
};

struct osu_legacy_database {
    int32_t version = 0;
    int32_t folder_count = 0;
    int32_t account_unlocked = 0;
    int64_t account_unlock_time = 0;
    std::string player_name;
    int32_t beatmaps_count = 0;
    std::vector<osu_db_beatmap> beatmaps;
    int32_t permissions = 0;
};

// === collection.db ===
struct osu_collection {
    std::string name;
    int32_t beatmaps_count = 0;
    std::vector<std::string> beatmap_md5;
};

struct osu_collection_db {
    int32_t version = 0;
    int32_t collections_count = 0;
    std::vector<osu_collection> collections;
};

// === scores.db ===
struct osu_score_base {
    int32_t mode = 0;
    int32_t version = 0;
    std::string beatmap_md5;
    std::string player_name;
    std::string replay_md5;
    int32_t count_300 = 0;
    int32_t count_100 = 0;
    int32_t count_50 = 0;
    int32_t count_geki = 0;
    int32_t count_katu = 0;
    int32_t count_miss = 0;
    int32_t score = 0;
    int32_t max_combo = 0;
    int32_t perfect = 0;
    int32_t mods = 0;
    std::string life_bar_graph;
    int64_t timestamp = 0;
    std::optional<double> additional_mod_info;
};

struct osu_score : osu_score_base {
    int32_t replay_data_length = -1;
    std::vector<uint8_t> replay_data;
    int64_t online_score_id = 0;
};

struct osu_scores_beatmap {
    std::string beatmap_md5;
    int32_t scores_count = 0;
    std::vector<osu_score> scores;
};

struct osu_scores_db {
    int32_t version = 0;
    int32_t beatmaps_count = 0;
    std::vector<osu_scores_beatmap> beatmaps;
};

// === replay (.osr) ===
struct osu_replay : osu_score_base {
    int32_t replay_data_length = 0;
    std::vector<uint8_t> replay_data;
    int64_t online_score_id = 0;
};

// === parsers ===
struct osu_db_parser {
    // data is non-owning and managed by parser_base
    osu_legacy_database* data = nullptr;
    std::string location;
    std::string last_error;

    bool parse(const std::string& location);
    bool write();
};

struct osu_collection_db_parser {
    // data is non-owning and managed by parser_base
    osu_collection_db* data = nullptr;
    std::string location;
    std::string last_error;

    bool parse(const std::string& location);
    bool write();
};

struct osu_scores_db_parser {
    // data is non-owning and managed by parser_base
    osu_scores_db* data = nullptr;
    std::string location;
    std::string last_error;

    bool parse(const std::string& location);
    bool write();
};

struct osu_replay_parser {
    // data is non-owning and managed by parser_base
    osu_replay* data = nullptr;
    std::string location;
    std::string last_error;

    bool parse(const std::string& location);
    bool write();
};
