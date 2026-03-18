#include "filter.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstring>
#include <optional>
#include <unordered_map>
#include <cstdlib>

namespace osu_filter {
    std::string to_lower_copy(std::string_view value) {
        std::string out(value);
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    static query_op parse_operator(const std::string& raw) {
        if (raw == "!=" || raw == "!:") {
            return query_op::ne;
        }
        if (raw == "<") {
            return query_op::lt;
        }
        if (raw == "<=") {
            return query_op::lte;
        }
        if (raw == ">") {
            return query_op::gt;
        }
        if (raw == ">=") {
            return query_op::gte;
        }
        return query_op::eq;
    }

    static bool parse_numeric(const std::string& value, double& out_number) {
        char* end = nullptr;
        out_number = std::strtod(value.c_str(), &end);
        return end != value.c_str() && *end == '\0' && std::isfinite(out_number);
    }

    static std::vector<std::string> split_csv(std::string_view value) {
        std::vector<std::string> parts;
        std::string current;
        for (char c : value) {
            if (c == ',') {
                if (!current.empty()) {
                    parts.push_back(std::move(current));
                    current.clear();
                }
            } else {
                current.push_back(c);
            }
        }
        if (!current.empty()) {
            parts.push_back(std::move(current));
        }
        return parts;
    }

    static bool parse_mode_token(const std::string& value, std::vector<int32_t>& out_modes, std::string& out_err) {
        static const std::unordered_map<std::string, int32_t> MODE_MAP = {
            {"osu", 0}, {"o", 0},      {"taiko", 1}, {"t", 1},     {"catch", 2},
            {"c", 2},   {"fruits", 2}, {"f", 2},     {"mania", 3}, {"m", 3},
        };

        for (const auto& token : split_csv(to_lower_copy(value))) {
            auto it = MODE_MAP.find(token);
            if (it == MODE_MAP.end()) {
                out_err = "invalid mode value";
                return false;
            }
            out_modes.push_back(it->second);
        }
        return true;
    }

    static bool parse_status_token(const std::string& value, std::vector<int32_t>& out_status, std::string& out_err) {
        static const std::unordered_map<std::string, int32_t> STATUS_MAP = {
            {"ranked", 4},       {"r", 4}, {"approved", 5}, {"a", 5}, {"pending", 2}, {"p", 2},
            {"notsubmitted", 1}, {"n", 1}, {"unknown", 0},  {"u", 0}, {"loved", 7},   {"l", 7},
        };

        for (const auto& token : split_csv(to_lower_copy(value))) {
            auto it = STATUS_MAP.find(token);
            if (it == STATUS_MAP.end()) {
                out_err = "invalid status value";
                return false;
            }
            out_status.push_back(it->second);
        }
        return true;
    }

    static std::optional<query_field> parse_key(const std::string& raw_key) {
        const std::string key = to_lower_copy(raw_key);

        if (key == "artist")
            return query_field::artist;
        if (key == "creator" || key == "author" || key == "mapper")
            return query_field::creator;
        if (key == "title")
            return query_field::title;
        if (key == "difficulty" || key == "diff")
            return query_field::difficulty;
        if (key == "ar")
            return query_field::ar;
        if (key == "cs")
            return query_field::cs;
        if (key == "od")
            return query_field::od;
        if (key == "hp" || key == "dr")
            return query_field::hp;
        if (key == "key" || key == "keys")
            return query_field::keys;
        if (key == "star" || key == "stars" || key == "sr")
            return query_field::star;
        if (key == "bpm")
            return query_field::bpm;
        if (key == "length")
            return query_field::length;
        if (key == "drain")
            return query_field::drain;
        if (key == "mode")
            return query_field::mode;
        if (key == "status")
            return query_field::status;
        if (key == "played")
            return query_field::played;
        if (key == "unplayed")
            return query_field::unplayed;
        if (key == "speed")
            return query_field::speed;
        if (key == "source")
            return query_field::source;
        if (key == "tag" || key == "tags")
            return query_field::tags;

        return std::nullopt;
    }

    bool parse_query(const std::string& query, osu_db_filter_props& out, std::string& err) {
        out.has_query = true;

        auto push_text_tokens = [&](const std::string& text) {
            std::string lower = to_lower_copy(text);
            std::string token;

            for (char c : lower) {
                if (std::isspace(static_cast<unsigned char>(c))) {
                    if (!token.empty()) {
                        out.query_text_tokens.push_back(token);
                        token.clear();
                    }
                } else {
                    token.push_back(c);
                }
            }
            if (!token.empty()) {
                out.query_text_tokens.push_back(token);
            }
        };

        size_t i = 0;
        const size_t len = query.size();

        while (i < len) {
            while (i < len && std::isspace(static_cast<unsigned char>(query[i]))) {
                i++;
            }
            if (i >= len) {
                break;
            }

            const size_t token_start = i;
            while (i < len && (std::isalnum(static_cast<unsigned char>(query[i])) || query[i] == '_')) {
                i++;
            }

            if (i == token_start) {
                size_t end = i;
                while (end < len && !std::isspace(static_cast<unsigned char>(query[end]))) {
                    end++;
                }
                push_text_tokens(query.substr(i, end - i));
                i = end;
                continue;
            }

            std::string key = query.substr(token_start, i - token_start);
            std::string op;

            auto peek = [&](const char* candidate) -> bool {
                size_t c_len = std::strlen(candidate);
                return i + c_len <= len && query.compare(i, c_len, candidate) == 0;
            };

            if (peek("==")) {
                op = "==";
                i += 2;
            } else if (peek("!=")) {
                op = "!=";
                i += 2;
            } else if (peek(">=")) {
                op = ">=";
                i += 2;
            } else if (peek("<=")) {
                op = "<=";
                i += 2;
            } else if (peek("!:")) {
                op = "!:";
                i += 2;
            } else if (peek("=")) {
                op = "=";
                i += 1;
            } else if (peek(":")) {
                op = ":";
                i += 1;
            } else if (peek(">")) {
                op = ">";
                i += 1;
            } else if (peek("<")) {
                op = "<";
                i += 1;
            } else {
                push_text_tokens(key);
                continue;
            }

            while (i < len && std::isspace(static_cast<unsigned char>(query[i]))) {
                i++;
            }

            std::string value;
            if (i < len && query[i] == '"') {
                i++;
                size_t start = i;
                while (i < len && query[i] != '"') {
                    i++;
                }
                value = query.substr(start, i - start);
                if (i < len && query[i] == '"') {
                    i++;
                }
            } else {
                size_t start = i;
                while (i < len && !std::isspace(static_cast<unsigned char>(query[i]))) {
                    i++;
                }
                value = query.substr(start, i - start);
            }

            auto field = parse_key(key);
            if (!field.has_value()) {
                if (!value.empty()) {
                    push_text_tokens(value);
                }
                continue;
            }

            if (value.empty() && field.value() != query_field::unplayed) {
                continue;
            }

            query_filter filter;
            filter.field = field.value();
            filter.op = parse_operator(op);

            if (filter.field == query_field::mode) {
                if (!parse_mode_token(value, filter.int_list, err)) {
                    return false;
                }
            } else if (filter.field == query_field::status) {
                if (!parse_status_token(value, filter.int_list, err)) {
                    return false;
                }
            } else if (filter.field == query_field::unplayed) {
                filter.flag = true;
            } else if (filter.field == query_field::artist || filter.field == query_field::creator ||
                       filter.field == query_field::title || filter.field == query_field::difficulty ||
                       filter.field == query_field::source || filter.field == query_field::tags) {
                filter.text = to_lower_copy(value);
            } else {
                double num = 0.0;
                if (!parse_numeric(value, num)) {
                    err = "invalid numeric filter";
                    return false;
                }
                filter.number = num;
                filter.has_number = true;
            }

            out.query_filters.push_back(std::move(filter));
        }

        return true;
    }

    template <typename T> static bool in_list(const std::vector<T>& values, const T& target) {
        if (values.empty()) {
            return true;
        }
        return std::find(values.begin(), values.end(), target) != values.end();
    }

    static bool range_ok(const range_filter& range, double value) {
        if (range.has_min && value < range.min) {
            return false;
        }
        if (range.has_max && value > range.max) {
            return false;
        }
        return true;
    }

    static double get_nomod_star_rating(const std::vector<osu_int_float_pair>& ratings) {
        if (ratings.empty()) {
            return 0.0;
        }
        for (const auto& pair : ratings) {
            if (pair.mod_combination == 0) {
                return pair.star_rating;
            }
        }
        return ratings.front().star_rating;
    }

    static double get_star_rating_for_mode(const osu_db_beatmap& beatmap) {
        switch (beatmap.mode) {
            case 1:
                return get_nomod_star_rating(beatmap.star_rating_taiko);
            case 2:
                return get_nomod_star_rating(beatmap.star_rating_ctb);
            case 3:
                return get_nomod_star_rating(beatmap.star_rating_mania);
            case 0:
            default:
                return get_nomod_star_rating(beatmap.star_rating_standard);
        }
    }

    double get_common_bpm(const std::vector<osu_db_timing_point>& timing_points, int32_t length) {
        if (timing_points.empty()) {
            return 0.0;
        }

        const double last_time = length > 0 ? static_cast<double>(length) : timing_points.back().offset;
        std::unordered_map<int32_t, double> duration_by_bpm;

        for (size_t i = 0; i < timing_points.size(); i++) {
            const auto& point = timing_points[i];
            if (point.offset > last_time) {
                continue;
            }

            if (point.bpm == 0.0) {
                continue;
            }

            const double bpm_raw = 60000.0 / point.bpm;
            const double bpm = std::round(bpm_raw * 1000.0) / 1000.0;
            const double current_time = i == 0 ? 0.0 : timing_points[i].offset;
            const double next_time = i + 1 >= timing_points.size() ? last_time : timing_points[i + 1].offset;
            const double duration = next_time - current_time;

            int32_t bpm_key = static_cast<int32_t>(std::round(bpm * 1000.0));
            duration_by_bpm[bpm_key] += duration;
        }

        double best_duration = 0.0;
        double best_bpm = 0.0;
        for (const auto& pair : duration_by_bpm) {
            if (pair.second > best_duration) {
                best_duration = pair.second;
                best_bpm = static_cast<double>(pair.first) / 1000.0;
            }
        }

        return best_bpm;
    }

    static bool contains_case_insensitive(const std::string& haystack, const std::string& needle) {
        if (needle.empty()) {
            return true;
        }
        std::string lower = to_lower_copy(haystack);
        return lower.find(needle) != std::string::npos;
    }

    static bool compare_numeric(double value, double target, query_op op) {
        switch (op) {
            case query_op::eq:
                return value == target;
            case query_op::ne:
                return value != target;
            case query_op::lt:
                return value < target;
            case query_op::lte:
                return value <= target;
            case query_op::gt:
                return value > target;
            case query_op::gte:
                return value >= target;
        }
        return false;
    }

    static bool compare_text(const std::string& value, const std::string& target, query_op op) {
        if (op == query_op::eq) {
            return contains_case_insensitive(value, target);
        }
        if (op == query_op::ne) {
            return !contains_case_insensitive(value, target);
        }
        return false;
    }

    static double ticks_to_days_since(int64_t ticks) {
        if (ticks <= 0) {
            return -1.0;
        }

        constexpr int64_t unix_epoch_ticks = 621355968000000000LL;
        constexpr int64_t ticks_per_millisecond = 10000LL;
        constexpr double milliseconds_per_day = 1000.0 * 60.0 * 60.0 * 24.0;

        const int64_t unix_ticks = ticks - unix_epoch_ticks;
        const double unix_ms = static_cast<double>(unix_ticks) / static_cast<double>(ticks_per_millisecond);
        const auto now = std::chrono::system_clock::now();
        const double now_ms =
            static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
        return (now_ms - unix_ms) / milliseconds_per_day;
    }

    static bool matches_query_filter(const osu_db_beatmap& beatmap, const query_filter& filter) {
        switch (filter.field) {
            case query_field::artist:
                return compare_text(beatmap.artist, filter.text, filter.op);
            case query_field::creator:
                return compare_text(beatmap.creator, filter.text, filter.op);
            case query_field::title:
                return compare_text(beatmap.title, filter.text, filter.op);
            case query_field::difficulty:
                return compare_text(beatmap.difficulty, filter.text, filter.op);
            case query_field::source:
                return compare_text(beatmap.source, filter.text, filter.op);
            case query_field::tags:
                return compare_text(beatmap.tags, filter.text, filter.op);
            case query_field::ar:
                return filter.has_number && compare_numeric(beatmap.approach_rate, filter.number, filter.op);
            case query_field::cs:
                return filter.has_number && compare_numeric(beatmap.circle_size, filter.number, filter.op);
            case query_field::od:
                return filter.has_number && compare_numeric(beatmap.overall_difficulty, filter.number, filter.op);
            case query_field::hp:
                return filter.has_number && compare_numeric(beatmap.hp_drain, filter.number, filter.op);
            case query_field::keys:
                return filter.has_number && compare_numeric(beatmap.circle_size, filter.number, filter.op);
            case query_field::star: {
                if (!filter.has_number) {
                    return false;
                }
                const double sr = get_star_rating_for_mode(beatmap);
                return compare_numeric(sr, filter.number, filter.op);
            }
            case query_field::bpm: {
                if (!filter.has_number) {
                    return false;
                }
                const double bpm = get_common_bpm(beatmap.timing_points, beatmap.total_time);
                return compare_numeric(bpm, filter.number, filter.op);
            }
            case query_field::length: {
                if (!filter.has_number) {
                    return false;
                }
                const double length = static_cast<double>(beatmap.total_time) / 1000.0;
                return compare_numeric(length, filter.number, filter.op);
            }
            case query_field::drain:
                return filter.has_number &&
                       compare_numeric(static_cast<double>(beatmap.drain_time), filter.number, filter.op);
            case query_field::mode: {
                if (filter.int_list.empty()) {
                    return false;
                }
                const bool in_list_result = in_list(filter.int_list, beatmap.mode);
                return filter.op == query_op::ne ? !in_list_result : in_list_result;
            }
            case query_field::status: {
                if (filter.int_list.empty()) {
                    return false;
                }
                const bool in_list_result = in_list(filter.int_list, beatmap.ranked_status);
                return filter.op == query_op::ne ? !in_list_result : in_list_result;
            }
            case query_field::played: {
                if (!filter.has_number) {
                    return false;
                }
                const double days = ticks_to_days_since(beatmap.last_played);
                if (days < 0.0) {
                    return false;
                }
                return compare_numeric(days, filter.number, filter.op);
            }
            case query_field::unplayed: {
                const bool is_unplayed = beatmap.unplayed != 0 || beatmap.last_played <= 0;
                return is_unplayed;
            }
            case query_field::speed:
                return filter.has_number &&
                       compare_numeric(static_cast<double>(beatmap.mania_scroll_speed), filter.number, filter.op);
        }

        return false;
    }

    bool matches_filter(const osu_db_beatmap& beatmap, const osu_db_filter_props& props) {
        if (!in_list(props.modes, beatmap.mode) || !in_list(props.ranked_statuses, beatmap.ranked_status) ||
            !in_list(props.beatmap_ids, beatmap.beatmap_id) || !in_list(props.difficulty_ids, beatmap.difficulty_id) ||
            !in_list(props.thread_ids, beatmap.thread_id)) {
            return false;
        }

        if (!props.md5_list.empty()) {
            const std::string md5_lower = to_lower_copy(beatmap.md5);
            if (!in_list(props.md5_list, md5_lower)) {
                return false;
            }
        }

        if (props.has_artist && !contains_case_insensitive(beatmap.artist, props.artist)) {
            return false;
        }
        if (props.has_title && !contains_case_insensitive(beatmap.title, props.title)) {
            return false;
        }
        if (props.has_creator && !contains_case_insensitive(beatmap.creator, props.creator)) {
            return false;
        }
        if (props.has_difficulty && !contains_case_insensitive(beatmap.difficulty, props.difficulty)) {
            return false;
        }
        if (props.has_source && !contains_case_insensitive(beatmap.source, props.source)) {
            return false;
        }
        if (props.has_tags && !contains_case_insensitive(beatmap.tags, props.tags)) {
            return false;
        }
        if (props.has_folder_name && !contains_case_insensitive(beatmap.folder_name, props.folder_name)) {
            return false;
        }
        if (props.has_audio_file_name && !contains_case_insensitive(beatmap.audio_file_name, props.audio_file_name)) {
            return false;
        }
        if (props.has_osu_file_name && !contains_case_insensitive(beatmap.osu_file_name, props.osu_file_name)) {
            return false;
        }

        if (props.has_query) {
            for (const auto& filter : props.query_filters) {
                if (!matches_query_filter(beatmap, filter)) {
                    return false;
                }
            }

            if (!props.query_text_tokens.empty()) {
                std::string combined = beatmap.artist;
                combined.append(" ")
                    .append(beatmap.title)
                    .append(" ")
                    .append(beatmap.creator)
                    .append(" ")
                    .append(beatmap.difficulty)
                    .append(" ")
                    .append(beatmap.source)
                    .append(" ")
                    .append(beatmap.tags);
                std::string combined_lower = to_lower_copy(combined);

                for (const auto& token : props.query_text_tokens) {
                    if (combined_lower.find(token) == std::string::npos) {
                        return false;
                    }
                }
            }
        }

        if (props.has_ar && !range_ok(props.ar, beatmap.approach_rate)) {
            return false;
        }
        if (props.has_cs && !range_ok(props.cs, beatmap.circle_size)) {
            return false;
        }
        if (props.has_hp && !range_ok(props.hp, beatmap.hp_drain)) {
            return false;
        }
        if (props.has_od && !range_ok(props.od, beatmap.overall_difficulty)) {
            return false;
        }
        if (props.has_drain_time && !range_ok(props.drain_time, static_cast<double>(beatmap.drain_time))) {
            return false;
        }
        if (props.has_total_time && !range_ok(props.total_time, static_cast<double>(beatmap.total_time))) {
            return false;
        }
        if (props.has_duration) {
            if (!beatmap.duration.has_value()) {
                return false;
            }
            if (!range_ok(props.duration, beatmap.duration.value())) {
                return false;
            }
        }
        if (props.has_audio_preview_time &&
            !range_ok(props.audio_preview_time, static_cast<double>(beatmap.audio_preview_time))) {
            return false;
        }

        if (props.has_star_rating) {
            double sr = get_star_rating_for_mode(beatmap);
            if (!range_ok(props.star_rating, sr)) {
                return false;
            }
        }

        return true;
    }

    struct sort_key_value {
        const osu_db_beatmap* beatmap = nullptr;
        bool is_string = false;
        std::string text;
        double number = 0.0;
        bool has_number = false;
    };

    static sort_key_value build_sort_key(const osu_db_beatmap& beatmap, const std::string& key) {
        sort_key_value out;
        out.beatmap = &beatmap;
        const std::string lower = to_lower_copy(key);

        if (lower == "artist") {
            out.is_string = true;
            out.text = to_lower_copy(beatmap.artist);
            return out;
        }
        if (lower == "title") {
            out.is_string = true;
            out.text = to_lower_copy(beatmap.title);
            return out;
        }
        if (lower == "creator") {
            out.is_string = true;
            out.text = to_lower_copy(beatmap.creator);
            return out;
        }
        if (lower == "difficulty") {
            out.is_string = true;
            out.text = to_lower_copy(beatmap.difficulty);
            return out;
        }
        if (lower == "source") {
            out.is_string = true;
            out.text = to_lower_copy(beatmap.source);
            return out;
        }
        if (lower == "tags") {
            out.is_string = true;
            out.text = to_lower_copy(beatmap.tags);
            return out;
        }
        if (lower == "folder_name") {
            out.is_string = true;
            out.text = to_lower_copy(beatmap.folder_name);
            return out;
        }
        if (lower == "audio_file_name") {
            out.is_string = true;
            out.text = to_lower_copy(beatmap.audio_file_name);
            return out;
        }
        if (lower == "osu_file_name") {
            out.is_string = true;
            out.text = to_lower_copy(beatmap.osu_file_name);
            return out;
        }

        out.is_string = false;
        out.has_number = true;

        if (lower == "ar") {
            out.number = beatmap.approach_rate;
        } else if (lower == "cs") {
            out.number = beatmap.circle_size;
        } else if (lower == "od") {
            out.number = beatmap.overall_difficulty;
        } else if (lower == "hp") {
            out.number = beatmap.hp_drain;
        } else if (lower == "star") {
            out.number = get_star_rating_for_mode(beatmap);
        } else if (lower == "bpm") {
            out.number = get_common_bpm(beatmap.timing_points, beatmap.total_time);
        } else if (lower == "length") {
            out.number = static_cast<double>(beatmap.total_time) / 1000.0;
        } else if (lower == "drain_time") {
            out.number = static_cast<double>(beatmap.drain_time);
        } else if (lower == "total_time") {
            out.number = static_cast<double>(beatmap.total_time);
        } else if (lower == "duration") {
            if (beatmap.duration.has_value()) {
                out.number = beatmap.duration.value();
            } else {
                out.has_number = false;
            }
        } else if (lower == "audio_preview_time") {
            out.number = static_cast<double>(beatmap.audio_preview_time);
        } else if (lower == "mode") {
            out.number = static_cast<double>(beatmap.mode);
        } else if (lower == "ranked_status") {
            out.number = static_cast<double>(beatmap.ranked_status);
        } else if (lower == "beatmap_id") {
            out.number = static_cast<double>(beatmap.beatmap_id);
        } else if (lower == "difficulty_id") {
            out.number = static_cast<double>(beatmap.difficulty_id);
        } else if (lower == "thread_id") {
            out.number = static_cast<double>(beatmap.thread_id);
        } else if (lower == "last_played") {
            out.number = static_cast<double>(beatmap.last_played);
        } else if (lower == "last_checked") {
            out.number = static_cast<double>(beatmap.last_checked);
        } else if (lower == "last_modified") {
            out.number = static_cast<double>(beatmap.last_modified);
        } else if (lower == "last_modification_time") {
            out.number = static_cast<double>(beatmap.last_modification_time);
        } else if (lower == "mania_scroll_speed") {
            out.number = static_cast<double>(beatmap.mania_scroll_speed);
        } else if (lower == "sliders") {
            out.number = static_cast<double>(beatmap.sliders);
        } else if (lower == "spinners") {
            out.number = static_cast<double>(beatmap.spinners);
        } else if (lower == "hitcircle") {
            out.number = static_cast<double>(beatmap.hitcircle);
        } else {
            out.has_number = false;
        }

        return out;
    }

    static void sort_matches(std::vector<const osu_db_beatmap*>& matches, const osu_db_filter_props& props) {
        if (props.sort_key.empty() || matches.size() < 2) {
            return;
        }

        std::vector<sort_key_value> keys;
        keys.reserve(matches.size());
        for (const auto* beatmap : matches) {
            keys.push_back(build_sort_key(*beatmap, props.sort_key));
        }

        const bool desc = props.sort_desc;

        std::stable_sort(keys.begin(), keys.end(), [desc](const sort_key_value& a, const sort_key_value& b) {
            if (a.is_string || b.is_string) {
                const bool a_empty = a.text.empty();
                const bool b_empty = b.text.empty();
                if (a_empty != b_empty) {
                    return !a_empty && b_empty;
                }
                if (a.text == b.text) {
                    return false;
                }
                const bool less = a.text < b.text;
                return desc ? !less : less;
            }

            if (!a.has_number && !b.has_number) {
                return false;
            }
            if (!a.has_number) {
                return false;
            }
            if (!b.has_number) {
                return true;
            }
            if (a.number == b.number) {
                return false;
            }
            const bool less = a.number < b.number;
            return desc ? !less : less;
        });

        for (size_t i = 0; i < keys.size(); i++) {
            matches[i] = keys[i].beatmap;
        }
    }

    std::vector<const osu_db_beatmap*> filter_by_properties(const std::vector<osu_db_beatmap>& beatmaps,
                                                            const osu_db_filter_props& props) {
        std::vector<const osu_db_beatmap*> out;
        out.reserve(beatmaps.size());

        for (const auto& beatmap : beatmaps) {
            if (matches_filter(beatmap, props)) {
                out.push_back(&beatmap);
            }
        }

        sort_matches(out, props);
        return out;
    }

    std::vector<const osu_db_beatmap*> filter_by_properties(const osu_legacy_database& db,
                                                            const osu_db_filter_props& props) {
        return filter_by_properties(db.beatmaps, props);
    }

    std::vector<std::string> filter_md5_by_properties(const std::vector<osu_db_beatmap>& beatmaps,
                                                      const osu_db_filter_props& props) {
        std::vector<const osu_db_beatmap*> matches = filter_by_properties(beatmaps, props);
        std::vector<std::string> out;
        out.reserve(matches.size());

        for (const auto* beatmap : matches) {
            out.push_back(beatmap->md5);
        }

        return out;
    }

    std::vector<std::string> filter_md5_by_properties(const osu_legacy_database& db, const osu_db_filter_props& props) {
        return filter_md5_by_properties(db.beatmaps, props);
    }

    std::vector<int32_t> filter_ids_by_properties(const std::vector<osu_db_beatmap>& beatmaps,
                                                  const osu_db_filter_props& props, id_type type) {
        std::vector<const osu_db_beatmap*> matches = filter_by_properties(beatmaps, props);
        std::vector<int32_t> out;
        out.reserve(matches.size());

        for (const auto* beatmap : matches) {
            out.push_back(type == id_type::beatmap_id ? beatmap->beatmap_id : beatmap->difficulty_id);
        }

        return out;
    }

    std::vector<int32_t> filter_ids_by_properties(const osu_legacy_database& db, const osu_db_filter_props& props,
                                                  id_type type) {
        return filter_ids_by_properties(db.beatmaps, props, type);
    }
}
