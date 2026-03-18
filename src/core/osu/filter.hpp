#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "osu.hpp"

namespace osu_filter {
    struct range_filter {
        bool has_min = false;
        double min = 0.0;
        bool has_max = false;
        double max = 0.0;
    };

    enum class query_field {
        artist,
        creator,
        title,
        difficulty,
        ar,
        cs,
        od,
        hp,
        keys,
        star,
        bpm,
        length,
        drain,
        mode,
        status,
        played,
        unplayed,
        speed,
        source,
        tags
    };

    enum class query_op { eq, ne, lt, lte, gt, gte };

    struct query_filter {
        query_field field;
        query_op op = query_op::eq;
        std::string text;
        double number = 0.0;
        bool has_number = false;
        std::vector<int32_t> int_list;
        bool flag = false;
    };

    struct osu_db_filter_props {
        bool has_query = false;
        std::vector<std::string> query_text_tokens;
        std::vector<query_filter> query_filters;

        std::vector<int32_t> modes;
        std::vector<int32_t> ranked_statuses;
        std::vector<int32_t> beatmap_ids;
        std::vector<int32_t> difficulty_ids;
        std::vector<int32_t> thread_ids;
        std::vector<std::string> md5_list;

        bool has_artist = false;
        bool has_title = false;
        bool has_creator = false;
        bool has_difficulty = false;
        bool has_source = false;
        bool has_tags = false;
        bool has_folder_name = false;
        bool has_audio_file_name = false;
        bool has_osu_file_name = false;

        std::string artist;
        std::string title;
        std::string creator;
        std::string difficulty;
        std::string source;
        std::string tags;
        std::string folder_name;
        std::string audio_file_name;
        std::string osu_file_name;

        bool has_ar = false;
        bool has_cs = false;
        bool has_hp = false;
        bool has_od = false;
        bool has_drain_time = false;
        bool has_total_time = false;
        bool has_duration = false;
        bool has_audio_preview_time = false;
        bool has_star_rating = false;

        range_filter ar;
        range_filter cs;
        range_filter hp;
        range_filter od;
        range_filter drain_time;
        range_filter total_time;
        range_filter duration;
        range_filter audio_preview_time;
        range_filter star_rating;

        std::string id_type;
        std::string sort_key;
        bool sort_desc = false;
    };

    std::string to_lower_copy(std::string_view value);
    double get_common_bpm(const std::vector<osu_db_timing_point>& timing_points, int32_t length);
    bool parse_query(const std::string& query, osu_db_filter_props& out, std::string& err);
    bool matches_filter(const osu_db_beatmap& beatmap, const osu_db_filter_props& props);

    enum class id_type { difficulty_id, beatmap_id };

    std::vector<const osu_db_beatmap*> filter_by_properties(const std::vector<osu_db_beatmap>& beatmaps,
                                                            const osu_db_filter_props& props);
    std::vector<const osu_db_beatmap*> filter_by_properties(const osu_legacy_database& db,
                                                            const osu_db_filter_props& props);
    std::vector<std::string> filter_md5_by_properties(const std::vector<osu_db_beatmap>& beatmaps,
                                                      const osu_db_filter_props& props);
    std::vector<std::string> filter_md5_by_properties(const osu_legacy_database& db, const osu_db_filter_props& props);
    std::vector<int32_t> filter_ids_by_properties(const std::vector<osu_db_beatmap>& beatmaps,
                                                  const osu_db_filter_props& props, id_type type);
    std::vector<int32_t> filter_ids_by_properties(const osu_legacy_database& db, const osu_db_filter_props& props,
                                                  id_type type);
}
