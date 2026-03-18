#include "common.hpp"

#include <cmath>

#include "osu/osu.hpp"
#include "osu/filter.hpp"
#include "parser/parser_base.hpp"
#include "update_helpers.hpp"

namespace osu_bindings {
    using osu_db_instance = parser_base<osu_legacy_database, osu_db_parser>;

#define OSU_DB_FIELDS(X)                                                                                               \
    X(get_optional_int32, "version", version)                                                                          \
    X(get_optional_int32, "folder_count", folder_count)                                                                \
    X(get_optional_int32, "account_unlocked", account_unlocked)                                                        \
    X(get_optional_int64, "account_unlock_time", account_unlock_time)                                                  \
    X(get_optional_string, "player_name", player_name)                                                                 \
    X(get_optional_int32, "permissions", permissions)

#define OSU_DB_BEATMAP_FIELDS_STRING(X)                                                                                \
    X(get_optional_string, "artist", artist)                                                                           \
    X(get_optional_string, "artist_unicode", artist_unicode)                                                           \
    X(get_optional_string, "title", title)                                                                             \
    X(get_optional_string, "title_unicode", title_unicode)                                                             \
    X(get_optional_string, "creator", creator)                                                                         \
    X(get_optional_string, "difficulty", difficulty)                                                                   \
    X(get_optional_string, "audio_file_name", audio_file_name)                                                         \
    X(get_optional_string, "md5", md5)                                                                                 \
    X(get_optional_string, "osu_file_name", osu_file_name)                                                             \
    X(get_optional_string, "source", source)                                                                           \
    X(get_optional_string, "tags", tags)                                                                               \
    X(get_optional_string, "title_font", title_font)                                                                   \
    X(get_optional_string, "folder_name", folder_name)

#define OSU_DB_BEATMAP_FIELDS_INT32(X)                                                                                 \
    X(get_optional_int32, "ranked_status", ranked_status)                                                              \
    X(get_optional_int32, "hitcircle", hitcircle)                                                                      \
    X(get_optional_int32, "sliders", sliders)                                                                          \
    X(get_optional_int32, "spinners", spinners)                                                                        \
    X(get_optional_int32, "drain_time", drain_time)                                                                    \
    X(get_optional_int32, "total_time", total_time)                                                                    \
    X(get_optional_int32, "audio_preview_time", audio_preview_time)                                                    \
    X(get_optional_int32, "difficulty_id", difficulty_id)                                                              \
    X(get_optional_int32, "beatmap_id", beatmap_id)                                                                    \
    X(get_optional_int32, "thread_id", thread_id)                                                                      \
    X(get_optional_int32, "grade_standard", grade_standard)                                                            \
    X(get_optional_int32, "grade_taiko", grade_taiko)                                                                  \
    X(get_optional_int32, "grade_ctb", grade_ctb)                                                                      \
    X(get_optional_int32, "grade_mania", grade_mania)                                                                  \
    X(get_optional_int32, "local_offset", local_offset)                                                                \
    X(get_optional_int32, "mode", mode)                                                                                \
    X(get_optional_int32, "online_offset", online_offset)                                                              \
    X(get_optional_int32, "unplayed", unplayed)                                                                        \
    X(get_optional_int32, "is_osz2", is_osz2)                                                                          \
    X(get_optional_int32, "ignore_sounds", ignore_sounds)                                                              \
    X(get_optional_int32, "ignore_skin", ignore_skin)                                                                  \
    X(get_optional_int32, "disable_storyboard", disable_storyboard)                                                    \
    X(get_optional_int32, "disable_video", disable_video)                                                              \
    X(get_optional_int32, "visual_override", visual_override)                                                          \
    X(get_optional_int32, "last_modified", last_modified)                                                              \
    X(get_optional_int32, "mania_scroll_speed", mania_scroll_speed)

#define OSU_DB_BEATMAP_FIELDS_INT64(X)                                                                                 \
    X(get_optional_int64, "last_modification_time", last_modification_time)                                            \
    X(get_optional_int64, "last_played", last_played)                                                                  \
    X(get_optional_int64, "last_checked", last_checked)

#define OSU_DB_BEATMAP_FIELDS_DOUBLE(X)                                                                                \
    X(get_optional_double, "approach_rate", approach_rate)                                                             \
    X(get_optional_double, "circle_size", circle_size)                                                                 \
    X(get_optional_double, "hp_drain", hp_drain)                                                                       \
    X(get_optional_double, "overall_difficulty", overall_difficulty)                                                   \
    X(get_optional_double, "slider_velocity", slider_velocity)                                                         \
    X(get_optional_double, "stack_leniency", stack_leniency)

    static bool parse_number_range(const Napi::Object& obj, const char* key, osu_filter::range_filter& out,
                                   std::string& err) {
        if (!obj.Has(key)) {
            return true;
        }

        Napi::Value value = obj.Get(key);

        if (!value.IsObject() || value.IsArray() || value.IsNull()) {
            err = std::string("invalid range for ") + key;
            return false;
        }

        Napi::Object range = value.As<Napi::Object>();

        if (range.Has("min")) {
            Napi::Value min_val = range.Get("min");
            if (!min_val.IsNumber()) {
                err = std::string("invalid range min for ") + key;
                return false;
            }
            out.min = min_val.As<Napi::Number>().DoubleValue();
            out.has_min = true;
        }

        if (range.Has("max")) {
            Napi::Value max_val = range.Get("max");
            if (!max_val.IsNumber()) {
                err = std::string("invalid range max for ") + key;
                return false;
            }
            out.max = max_val.As<Napi::Number>().DoubleValue();
            out.has_max = true;
        }

        if (out.has_min || out.has_max) {
            return true;
        }

        err = std::string("empty range for ") + key;
        return false;
    }

    static bool parse_int_list(const Napi::Object& obj, const char* key, std::vector<int32_t>& out, std::string& err) {
        if (!obj.Has(key)) {
            return true;
        }

        Napi::Value value = obj.Get(key);

        if (value.IsNumber()) {
            const double num = value.As<Napi::Number>().DoubleValue();
            if (!std::isfinite(num) || std::floor(num) != num || num < INT32_MIN || num > INT32_MAX) {
                err = std::string("invalid number for ") + key;
                return false;
            }
            out.push_back(static_cast<int32_t>(num));
            return true;
        }

        if (!value.IsArray()) {
            err = std::string("invalid array for ") + key;
            return false;
        }

        Napi::Array arr = value.As<Napi::Array>();
        out.reserve(out.size() + arr.Length());

        for (uint32_t i = 0; i < arr.Length(); i++) {
            Napi::Value item = arr.Get(i);
            if (!item.IsNumber()) {
                err = std::string("invalid array item for ") + key;
                return false;
            }
            const double num = item.As<Napi::Number>().DoubleValue();
            if (!std::isfinite(num) || std::floor(num) != num || num < INT32_MIN || num > INT32_MAX) {
                err = std::string("invalid array item for ") + key;
                return false;
            }
            out.push_back(static_cast<int32_t>(num));
        }

        return true;
    }

    static bool parse_string_list(const Napi::Object& obj, const char* key, std::vector<std::string>& out,
                                  std::string& err) {
        if (!obj.Has(key)) {
            return true;
        }

        Napi::Value value = obj.Get(key);

        if (value.IsString()) {
            out.push_back(osu_filter::to_lower_copy(value.As<Napi::String>().Utf8Value()));
            return true;
        }

        if (!value.IsArray()) {
            err = std::string("invalid array for ") + key;
            return false;
        }

        Napi::Array arr = value.As<Napi::Array>();
        out.reserve(out.size() + arr.Length());

        for (uint32_t i = 0; i < arr.Length(); i++) {
            Napi::Value item = arr.Get(i);
            if (!item.IsString()) {
                err = std::string("invalid array item for ") + key;
                return false;
            }
            out.push_back(osu_filter::to_lower_copy(item.As<Napi::String>().Utf8Value()));
        }

        return true;
    }

    static bool parse_optional_string_prop(const Napi::Object& obj, const char* key, std::string& out, bool& has,
                                           std::string& err) {
        if (!obj.Has(key)) {
            return true;
        }

        Napi::Value value = obj.Get(key);
        if (!value.IsString()) {
            err = std::string("invalid string for ") + key;
            return false;
        }

        out = osu_filter::to_lower_copy(value.As<Napi::String>().Utf8Value());
        has = true;
        return true;
    }

    static bool parse_filter_props(const Napi::Value& value, osu_filter::osu_db_filter_props& out, std::string& err) {
        if (!is_object(value)) {
            err = "properties must be an object";
            return false;
        }

        Napi::Object obj = value.As<Napi::Object>();

        if (obj.Has("query")) {
            Napi::Value q = obj.Get("query");
            if (!q.IsString()) {
                err = "invalid query";
                return false;
            }
            std::string query = q.As<Napi::String>();
            if (!osu_filter::parse_query(query, out, err)) {
                return false;
            }
        }

        if (!parse_int_list(obj, "mode", out.modes, err) ||
            !parse_int_list(obj, "ranked_status", out.ranked_statuses, err) ||
            !parse_int_list(obj, "beatmap_id", out.beatmap_ids, err) ||
            !parse_int_list(obj, "difficulty_id", out.difficulty_ids, err) ||
            !parse_int_list(obj, "thread_id", out.thread_ids, err)) {
            return false;
        }

        if (!parse_string_list(obj, "md5", out.md5_list, err)) {
            return false;
        }

        if (!parse_optional_string_prop(obj, "artist", out.artist, out.has_artist, err) ||
            !parse_optional_string_prop(obj, "title", out.title, out.has_title, err) ||
            !parse_optional_string_prop(obj, "creator", out.creator, out.has_creator, err) ||
            !parse_optional_string_prop(obj, "difficulty", out.difficulty, out.has_difficulty, err) ||
            !parse_optional_string_prop(obj, "source", out.source, out.has_source, err) ||
            !parse_optional_string_prop(obj, "tags", out.tags, out.has_tags, err) ||
            !parse_optional_string_prop(obj, "folder_name", out.folder_name, out.has_folder_name, err) ||
            !parse_optional_string_prop(obj, "audio_file_name", out.audio_file_name, out.has_audio_file_name, err) ||
            !parse_optional_string_prop(obj, "osu_file_name", out.osu_file_name, out.has_osu_file_name, err)) {
            return false;
        }

        if (!parse_number_range(obj, "ar", out.ar, err) || !parse_number_range(obj, "cs", out.cs, err) ||
            !parse_number_range(obj, "hp", out.hp, err) || !parse_number_range(obj, "od", out.od, err) ||
            !parse_number_range(obj, "drain_time", out.drain_time, err) ||
            !parse_number_range(obj, "total_time", out.total_time, err) ||
            !parse_number_range(obj, "duration", out.duration, err) ||
            !parse_number_range(obj, "audio_preview_time", out.audio_preview_time, err) ||
            !parse_number_range(obj, "star_rating", out.star_rating, err)) {
            return false;
        }

        out.has_ar = obj.Has("ar");
        out.has_cs = obj.Has("cs");
        out.has_hp = obj.Has("hp");
        out.has_od = obj.Has("od");
        out.has_drain_time = obj.Has("drain_time");
        out.has_total_time = obj.Has("total_time");
        out.has_duration = obj.Has("duration");
        out.has_audio_preview_time = obj.Has("audio_preview_time");
        out.has_star_rating = obj.Has("star_rating");

        if (obj.Has("id_type")) {
            Napi::Value id_type = obj.Get("id_type");
            if (!id_type.IsString()) {
                err = "invalid id_type";
                return false;
            }
            out.id_type = id_type.As<Napi::String>();
        }

        if (obj.Has("sort")) {
            Napi::Value sort_val = obj.Get("sort");
            if (!is_object(sort_val)) {
                err = "invalid sort";
                return false;
            }

            Napi::Object sort_obj = sort_val.As<Napi::Object>();
            if (!sort_obj.Has("key") || !sort_obj.Get("key").IsString()) {
                err = "invalid sort key";
                return false;
            }

            out.sort_key = sort_obj.Get("key").As<Napi::String>().Utf8Value();
            out.sort_desc = false;

            if (sort_obj.Has("order")) {
                Napi::Value order_val = sort_obj.Get("order");
                if (!order_val.IsString()) {
                    err = "invalid sort order";
                    return false;
                }
                std::string order = osu_filter::to_lower_copy(order_val.As<Napi::String>().Utf8Value());
                if (order == "desc") {
                    out.sort_desc = true;
                } else if (order == "asc") {
                    out.sort_desc = false;
                } else {
                    err = "invalid sort order";
                    return false;
                }
            }
        }

        return true;
    }

    bool parse_int_float_pair(const Napi::Value& value, osu_int_float_pair& out, std::string& err) {
        if (!is_object(value)) {
            err = "int-float pair must be an object";
            return false;
        }
        Napi::Object obj = value.As<Napi::Object>();
        return get_optional_int32(obj, "mod_combination", out.mod_combination, err) &&
               get_optional_double(obj, "star_rating", out.star_rating, err);
    }

    bool parse_db_timing_point(const Napi::Value& value, osu_db_timing_point& out, std::string& err) {
        if (!is_object(value)) {
            err = "timing point must be an object";
            return false;
        }
        Napi::Object obj = value.As<Napi::Object>();
        return get_optional_double(obj, "bpm", out.bpm, err) && get_optional_double(obj, "offset", out.offset, err) &&
               get_optional_int32(obj, "inherited", out.inherited, err);
    }

    bool update_osu_db_beatmap(osu_db_beatmap& beatmap, const Napi::Object& patch, std::string& err) {
        if (patch.Has("entry_size")) {
            Napi::Value value = patch.Get("entry_size");
            if (value.IsNull()) {
                beatmap.entry_size.reset();
            } else if (value.IsNumber()) {
                beatmap.entry_size = value.As<Napi::Number>().Int32Value();
            } else {
                err = "invalid type for entry_size";
                return false;
            }
        }

        if (patch.Has("duration")) {
            Napi::Value value = patch.Get("duration");
            if (value.IsNull()) {
                beatmap.duration.reset();
            } else if (value.IsNumber()) {
                beatmap.duration = value.As<Napi::Number>().DoubleValue();
            } else {
                err = "invalid type for duration";
                return false;
            }
        }

#define APPLY_FIELD(fn, key, member)                                                                                   \
    if (!fn(patch, key, beatmap.member, err)) {                                                                        \
        return false;                                                                                                  \
    }
        OSU_DB_BEATMAP_FIELDS_STRING(APPLY_FIELD)
        OSU_DB_BEATMAP_FIELDS_INT32(APPLY_FIELD)
        OSU_DB_BEATMAP_FIELDS_INT64(APPLY_FIELD)
        OSU_DB_BEATMAP_FIELDS_DOUBLE(APPLY_FIELD)
#undef APPLY_FIELD

        if (!update_array_field(patch, "star_rating_standard", beatmap.star_rating_standard, parse_int_float_pair,
                                err) ||
            !update_array_field(patch, "star_rating_taiko", beatmap.star_rating_taiko, parse_int_float_pair, err) ||
            !update_array_field(patch, "star_rating_ctb", beatmap.star_rating_ctb, parse_int_float_pair, err) ||
            !update_array_field(patch, "star_rating_mania", beatmap.star_rating_mania, parse_int_float_pair, err) ||
            !update_array_field(patch, "timing_points", beatmap.timing_points, parse_db_timing_point, err)) {
            return false;
        }

        if (patch.Has("unknown")) {
            Napi::Value value = patch.Get("unknown");
            if (value.IsNull()) {
                beatmap.unknown.reset();
            } else if (value.IsNumber()) {
                beatmap.unknown = value.As<Napi::Number>().Int32Value();
            } else {
                err = "invalid type for unknown";
                return false;
            }
        }

        return true;
    }

    bool parse_osu_db_beatmap(const Napi::Value& value, osu_db_beatmap& out, std::string& err) {
        if (!is_object(value)) {
            err = "beatmap must be an object";
            return false;
        }
        Napi::Object obj = value.As<Napi::Object>();
        return update_osu_db_beatmap(out, obj, err);
    }

    Napi::Value optional_int32_to_js(Napi::Env env, const std::optional<int32_t>& value) {
        if (value.has_value()) {
            return Napi::Number::New(env, value.value());
        }

        return env.Null();
    }

    Napi::Object int_float_pair_to_js(Napi::Env& env, const osu_int_float_pair& pair) {
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("mod_combination", pair.mod_combination);
        obj.Set("star_rating", pair.star_rating);
        return obj;
    }

    Napi::Object db_timing_point_to_js(Napi::Env& env, const osu_db_timing_point& point) {
        Napi::Object obj = Napi::Object::New(env);

        obj.Set("bpm", point.bpm);
        obj.Set("offset", point.offset);
        obj.Set("inherited", point.inherited);

        return obj;
    }

    Napi::Object beatmap_to_js(Napi::Env& env, const osu_db_beatmap& beatmap) {
        Napi::Object obj = Napi::Object::New(env);

        obj.Set("entry_size", optional_int32_to_js(env, beatmap.entry_size));
        obj.Set("artist", beatmap.artist);
        obj.Set("artist_unicode", beatmap.artist_unicode);
        obj.Set("title", beatmap.title);
        obj.Set("title_unicode", beatmap.title_unicode);
        obj.Set("creator", beatmap.creator);
        obj.Set("difficulty", beatmap.difficulty);
        obj.Set("audio_file_name", beatmap.audio_file_name);
        obj.Set("md5", beatmap.md5);
        obj.Set("osu_file_name", beatmap.osu_file_name);
        obj.Set("ranked_status", beatmap.ranked_status);
        obj.Set("hitcircle", beatmap.hitcircle);
        obj.Set("sliders", beatmap.sliders);
        obj.Set("spinners", beatmap.spinners);
        obj.Set("last_modification_time", Napi::BigInt::New(env, beatmap.last_modification_time));
        obj.Set("approach_rate", beatmap.approach_rate);
        obj.Set("circle_size", beatmap.circle_size);
        obj.Set("hp_drain", beatmap.hp_drain);
        obj.Set("overall_difficulty", beatmap.overall_difficulty);
        obj.Set("slider_velocity", beatmap.slider_velocity);

        Napi::Array star_standard = Napi::Array::New(env, beatmap.star_rating_standard.size());

        for (size_t i = 0; i < beatmap.star_rating_standard.size(); i++) {
            star_standard.Set(static_cast<uint32_t>(i), int_float_pair_to_js(env, beatmap.star_rating_standard[i]));
        }
        obj.Set("star_rating_standard", star_standard);

        Napi::Array star_taiko = Napi::Array::New(env, beatmap.star_rating_taiko.size());

        for (size_t i = 0; i < beatmap.star_rating_taiko.size(); i++) {
            star_taiko.Set(static_cast<uint32_t>(i), int_float_pair_to_js(env, beatmap.star_rating_taiko[i]));
        }

        obj.Set("star_rating_taiko", star_taiko);

        Napi::Array star_ctb = Napi::Array::New(env, beatmap.star_rating_ctb.size());

        for (size_t i = 0; i < beatmap.star_rating_ctb.size(); i++) {
            star_ctb.Set(static_cast<uint32_t>(i), int_float_pair_to_js(env, beatmap.star_rating_ctb[i]));
        }
        obj.Set("star_rating_ctb", star_ctb);

        Napi::Array star_mania = Napi::Array::New(env, beatmap.star_rating_mania.size());

        for (size_t i = 0; i < beatmap.star_rating_mania.size(); i++) {
            star_mania.Set(static_cast<uint32_t>(i), int_float_pair_to_js(env, beatmap.star_rating_mania[i]));
        }

        obj.Set("star_rating_mania", star_mania);
        obj.Set("drain_time", beatmap.drain_time);
        obj.Set("total_time", beatmap.total_time);
        obj.Set("duration", optional_double_to_js(env, beatmap.duration));
        obj.Set("audio_preview_time", beatmap.audio_preview_time);

        Napi::Array timing_points = Napi::Array::New(env, beatmap.timing_points.size());

        for (size_t i = 0; i < beatmap.timing_points.size(); i++) {
            timing_points.Set(static_cast<uint32_t>(i), db_timing_point_to_js(env, beatmap.timing_points[i]));
        }

        obj.Set("timing_points", timing_points);
        obj.Set("difficulty_id", beatmap.difficulty_id);
        obj.Set("beatmap_id", beatmap.beatmap_id);
        obj.Set("thread_id", beatmap.thread_id);
        obj.Set("grade_standard", beatmap.grade_standard);
        obj.Set("grade_taiko", beatmap.grade_taiko);
        obj.Set("grade_ctb", beatmap.grade_ctb);
        obj.Set("grade_mania", beatmap.grade_mania);
        obj.Set("local_offset", beatmap.local_offset);
        obj.Set("stack_leniency", beatmap.stack_leniency);
        obj.Set("mode", beatmap.mode);
        obj.Set("source", beatmap.source);
        obj.Set("tags", beatmap.tags);
        obj.Set("online_offset", beatmap.online_offset);
        obj.Set("title_font", beatmap.title_font);
        obj.Set("unplayed", beatmap.unplayed);
        obj.Set("last_played", Napi::BigInt::New(env, beatmap.last_played));
        obj.Set("is_osz2", beatmap.is_osz2);
        obj.Set("folder_name", beatmap.folder_name);
        obj.Set("last_checked", Napi::BigInt::New(env, beatmap.last_checked));
        obj.Set("ignore_sounds", beatmap.ignore_sounds);
        obj.Set("ignore_skin", beatmap.ignore_skin);
        obj.Set("disable_storyboard", beatmap.disable_storyboard);
        obj.Set("disable_video", beatmap.disable_video);
        obj.Set("visual_override", beatmap.visual_override);
        obj.Set("unknown", optional_int32_to_js(env, beatmap.unknown));
        obj.Set("last_modified", beatmap.last_modified);
        obj.Set("mania_scroll_speed", beatmap.mania_scroll_speed);

        return obj;
    }

    Napi::Object osu_db_to_js(Napi::Env& env, const osu_legacy_database& db) {
        Napi::Object obj = Napi::Object::New(env);

        obj.Set("version", db.version);
        obj.Set("folder_count", db.folder_count);
        obj.Set("account_unlocked", db.account_unlocked);
        obj.Set("account_unlock_time", Napi::BigInt::New(env, db.account_unlock_time));
        obj.Set("player_name", db.player_name);
        obj.Set("beatmaps_count", db.beatmaps_count);

        Napi::Array beatmaps = Napi::Array::New(env, db.beatmaps.size());

        for (size_t i = 0; i < db.beatmaps.size(); i++) {
            beatmaps.Set(static_cast<uint32_t>(i), beatmap_to_js(env, db.beatmaps[i]));
        }

        obj.Set("beatmaps", beatmaps);
        obj.Set("permissions", db.permissions);

        return obj;
    }

    Napi::Value create_osu_db_parser(const Napi::CallbackInfo& info) {
        return create_instance<osu_db_instance>(info);
    }

    Napi::Value free_osu_db_parser(const Napi::CallbackInfo& info) {
        return free_instance<osu_db_instance>(info);
    }

    Napi::Value osu_db_parser_parse(const Napi::CallbackInfo& info) {
        return parse_instance_async<osu_db_instance>(info, "osu_db_parse");
    }

    Napi::Value osu_db_parser_write(const Napi::CallbackInfo& info) {
        return write_instance_async<osu_db_instance>(info, "osu_db_write");
    }

    Napi::Value osu_db_parser_last_error(const Napi::CallbackInfo& info) {
        return last_error_instance<osu_db_instance>(info);
    }

    Napi::Value osu_db_parser_get(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osu_db_instance* instance = get_ptr<osu_db_instance>(info, 0);

        if (instance == nullptr) {
            return env.Null();
        }

        return instance->with_lock([&](osu_legacy_database& data, osu_db_parser&) { return osu_db_to_js(env, data); });
    }

    Napi::Value osu_db_get_by_key(Napi::Env& env, const osu_legacy_database& data, const std::string& key) {
        if (key == "version")
            return Napi::Number::New(env, data.version);
        if (key == "folder_count")
            return Napi::Number::New(env, data.folder_count);
        if (key == "account_unlocked")
            return Napi::Number::New(env, data.account_unlocked);
        if (key == "account_unlock_time")
            return Napi::BigInt::New(env, data.account_unlock_time);
        if (key == "player_name")
            return Napi::String::New(env, data.player_name);
        if (key == "beatmaps_count")
            return Napi::Number::New(env, data.beatmaps_count);
        if (key == "permissions")
            return Napi::Number::New(env, data.permissions);
        if (key == "beatmaps") {
            Napi::Array beatmaps = Napi::Array::New(env, data.beatmaps.size());
            for (size_t i = 0; i < data.beatmaps.size(); i++) {
                beatmaps.Set(static_cast<uint32_t>(i), beatmap_to_js(env, data.beatmaps[i]));
            }
            return beatmaps;
        }

        return env.Undefined();
    }

    Napi::Value osu_db_parser_update(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osu_db_instance* instance = get_ptr<osu_db_instance>(info, 0);

        if (instance == nullptr) {
            Napi::Error::New(env, "invalid parser handle").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        if (info.Length() < 2 || !is_object(info[1])) {
            Napi::TypeError::New(env, "update patch must be an object").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        std::string err;
        bool ok = instance->with_lock([&](osu_legacy_database& data, osu_db_parser& parser) {
            Napi::Object patch = info[1].As<Napi::Object>();
            osu_legacy_database temp = data;

#define APPLY_FIELD(fn, key, member)                                                                                   \
    if (!fn(patch, key, temp.member, err)) {                                                                           \
        parser.last_error = err;                                                                                       \
        return false;                                                                                                  \
    }
            OSU_DB_FIELDS(APPLY_FIELD)
#undef APPLY_FIELD

            if (!update_array_field(patch, "beatmaps", temp.beatmaps, parse_osu_db_beatmap, err)) {
                parser.last_error = err;
                return false;
            }

            temp.beatmaps_count = static_cast<int32_t>(temp.beatmaps.size());
            data = std::move(temp);
            parser.last_error.clear();
            return true;
        });

        if (!ok) {
            std::string message = instance->parser.last_error.empty() ? "update failed" : instance->parser.last_error;
            Napi::Error::New(env, message).ThrowAsJavaScriptException();
            return env.Undefined();
        }

        return Napi::Boolean::New(env, true);
    }

    Napi::Value osu_db_parser_get_by_name(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osu_db_instance* instance = get_ptr<osu_db_instance>(info, 0);

        if (instance == nullptr || info.Length() < 2 || !info[1].IsString()) {
            return env.Undefined();
        }

        std::string key = info[1].As<Napi::String>();
        return instance->with_lock(
            [&](osu_legacy_database& data, osu_db_parser&) { return osu_db_get_by_key(env, data, key); });
    }

    Napi::Value osu_db_parser_filter_by_properties(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osu_db_instance* instance = get_ptr<osu_db_instance>(info, 0);

        if (instance == nullptr) {
            Napi::Error::New(env, "invalid parser handle").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        if (info.Length() < 2 || !is_object(info[1])) {
            Napi::TypeError::New(env, "properties must be an object").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        osu_filter::osu_db_filter_props props;
        std::string err;

        if (!parse_filter_props(info[1], props, err)) {
            Napi::Error::New(env, err).ThrowAsJavaScriptException();
            return env.Undefined();
        }

        return instance->with_lock([&](osu_legacy_database& data, osu_db_parser&) {
            std::vector<const osu_db_beatmap*> matches = osu_filter::filter_by_properties(data, props);

            Napi::Array result = Napi::Array::New(env, matches.size());
            for (size_t i = 0; i < matches.size(); i++) {
                result.Set(static_cast<uint32_t>(i), beatmap_to_js(env, *matches[i]));
            }

            return result;
        });
    }

    Napi::Value osu_db_parser_filter_md5_by_properties(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osu_db_instance* instance = get_ptr<osu_db_instance>(info, 0);

        if (instance == nullptr) {
            Napi::Error::New(env, "invalid parser handle").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        if (info.Length() < 2 || !is_object(info[1])) {
            Napi::TypeError::New(env, "properties must be an object").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        osu_filter::osu_db_filter_props props;
        std::string err;

        if (!parse_filter_props(info[1], props, err)) {
            Napi::Error::New(env, err).ThrowAsJavaScriptException();
            return env.Undefined();
        }

        return instance->with_lock([&](osu_legacy_database& data, osu_db_parser&) {
            std::vector<std::string> matches = osu_filter::filter_md5_by_properties(data, props);

            Napi::Array result = Napi::Array::New(env, matches.size());
            for (size_t i = 0; i < matches.size(); i++) {
                result.Set(static_cast<uint32_t>(i), Napi::String::New(env, matches[i]));
            }

            return result;
        });
    }

    Napi::Value osu_db_parser_filter_ids_by_properties(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osu_db_instance* instance = get_ptr<osu_db_instance>(info, 0);

        if (instance == nullptr) {
            Napi::Error::New(env, "invalid parser handle").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        if (info.Length() < 2 || !is_object(info[1])) {
            Napi::TypeError::New(env, "properties must be an object").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        osu_filter::osu_db_filter_props props;
        std::string err;

        if (!parse_filter_props(info[1], props, err)) {
            Napi::Error::New(env, err).ThrowAsJavaScriptException();
            return env.Undefined();
        }

        const std::string id_type = props.id_type.empty() ? "difficulty_id" : props.id_type;

        if (id_type != "difficulty_id" && id_type != "beatmap_id") {
            Napi::Error::New(env, "invalid id_type").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        const osu_filter::id_type type =
            id_type == "beatmap_id" ? osu_filter::id_type::beatmap_id : osu_filter::id_type::difficulty_id;

        return instance->with_lock([&](osu_legacy_database& data, osu_db_parser&) {
            std::vector<int32_t> matches = osu_filter::filter_ids_by_properties(data, props, type);

            Napi::Array result = Napi::Array::New(env, matches.size());
            for (size_t i = 0; i < matches.size(); i++) {
                result.Set(static_cast<uint32_t>(i), Napi::Number::New(env, matches[i]));
            }

            return result;
        });
    }

    void register_osu_db(Napi::Env env, Napi::Object exports) {
        exports.Set("create_osu_db_parser", Napi::Function::New(env, create_osu_db_parser));
        exports.Set("free_osu_db_parser", Napi::Function::New(env, free_osu_db_parser));
        exports.Set("osu_db_parser_parse", Napi::Function::New(env, osu_db_parser_parse));
        exports.Set("osu_db_parser_write", Napi::Function::New(env, osu_db_parser_write));
        exports.Set("osu_db_parser_last_error", Napi::Function::New(env, osu_db_parser_last_error));
        exports.Set("osu_db_parser_get", Napi::Function::New(env, osu_db_parser_get));
        exports.Set("osu_db_parser_update", Napi::Function::New(env, osu_db_parser_update));
        exports.Set("osu_db_parser_get_by_name", Napi::Function::New(env, osu_db_parser_get_by_name));
        exports.Set("osu_db_parser_filter_by_properties", Napi::Function::New(env, osu_db_parser_filter_by_properties));
        exports.Set("osu_db_parser_filter_md5_by_properties",
                    Napi::Function::New(env, osu_db_parser_filter_md5_by_properties));
        exports.Set("osu_db_parser_filter_ids_by_properties",
                    Napi::Function::New(env, osu_db_parser_filter_ids_by_properties));
    }
}
