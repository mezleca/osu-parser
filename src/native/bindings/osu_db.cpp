#include "common.hpp"

#include "osu/osu.hpp"
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

#define APPLY_FIELD(fn, key, member)                                                                                   \
    if (!fn(patch, key, beatmap.member, err)) {                                                                        \
        return false;                                                                                                  \
    }
        OSU_DB_BEATMAP_FIELDS_STRING(APPLY_FIELD)
        OSU_DB_BEATMAP_FIELDS_INT32(APPLY_FIELD)
        OSU_DB_BEATMAP_FIELDS_INT64(APPLY_FIELD)
        OSU_DB_BEATMAP_FIELDS_DOUBLE(APPLY_FIELD)
#undef APPLY_FIELD

        if (patch.Has("star_rating_standard")) {
            Napi::Value value = patch.Get("star_rating_standard");
            if (!value.IsArray()) {
                err = "star_rating_standard must be an array";
                return false;
            }
            Napi::Array arr = value.As<Napi::Array>();
            std::vector<osu_int_float_pair> next;
            next.reserve(arr.Length());
            for (uint32_t i = 0; i < arr.Length(); i++) {
                osu_int_float_pair item{};
                if (!parse_int_float_pair(arr.Get(i), item, err)) {
                    return false;
                }
                next.push_back(std::move(item));
            }
            beatmap.star_rating_standard = std::move(next);
        }

        if (patch.Has("star_rating_taiko")) {
            Napi::Value value = patch.Get("star_rating_taiko");
            if (!value.IsArray()) {
                err = "star_rating_taiko must be an array";
                return false;
            }
            Napi::Array arr = value.As<Napi::Array>();
            std::vector<osu_int_float_pair> next;
            next.reserve(arr.Length());
            for (uint32_t i = 0; i < arr.Length(); i++) {
                osu_int_float_pair item{};
                if (!parse_int_float_pair(arr.Get(i), item, err)) {
                    return false;
                }
                next.push_back(std::move(item));
            }
            beatmap.star_rating_taiko = std::move(next);
        }

        if (patch.Has("star_rating_ctb")) {
            Napi::Value value = patch.Get("star_rating_ctb");
            if (!value.IsArray()) {
                err = "star_rating_ctb must be an array";
                return false;
            }
            Napi::Array arr = value.As<Napi::Array>();
            std::vector<osu_int_float_pair> next;
            next.reserve(arr.Length());
            for (uint32_t i = 0; i < arr.Length(); i++) {
                osu_int_float_pair item{};
                if (!parse_int_float_pair(arr.Get(i), item, err)) {
                    return false;
                }
                next.push_back(std::move(item));
            }
            beatmap.star_rating_ctb = std::move(next);
        }

        if (patch.Has("star_rating_mania")) {
            Napi::Value value = patch.Get("star_rating_mania");
            if (!value.IsArray()) {
                err = "star_rating_mania must be an array";
                return false;
            }
            Napi::Array arr = value.As<Napi::Array>();
            std::vector<osu_int_float_pair> next;
            next.reserve(arr.Length());
            for (uint32_t i = 0; i < arr.Length(); i++) {
                osu_int_float_pair item{};
                if (!parse_int_float_pair(arr.Get(i), item, err)) {
                    return false;
                }
                next.push_back(std::move(item));
            }
            beatmap.star_rating_mania = std::move(next);
        }

        if (patch.Has("timing_points")) {
            Napi::Value value = patch.Get("timing_points");
            if (!value.IsArray()) {
                err = "timing_points must be an array";
                return false;
            }
            Napi::Array arr = value.As<Napi::Array>();
            std::vector<osu_db_timing_point> next;
            next.reserve(arr.Length());
            for (uint32_t i = 0; i < arr.Length(); i++) {
                osu_db_timing_point item{};
                if (!parse_db_timing_point(arr.Get(i), item, err)) {
                    return false;
                }
                next.push_back(std::move(item));
            }
            beatmap.timing_points = std::move(next);
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

    Napi::Object timing_point_to_js(Napi::Env& env, const osu_db_timing_point& point) {
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
        obj.Set("audio_preview_time", beatmap.audio_preview_time);

        Napi::Array timing_points = Napi::Array::New(env, beatmap.timing_points.size());

        for (size_t i = 0; i < beatmap.timing_points.size(); i++) {
            timing_points.Set(static_cast<uint32_t>(i), timing_point_to_js(env, beatmap.timing_points[i]));
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

#define APPLY_FIELD(fn, key, member)                                                                                   \
    if (!fn(patch, key, data.member, err)) {                                                                           \
        parser.last_error = err;                                                                                       \
        return false;                                                                                                  \
    }
            OSU_DB_FIELDS(APPLY_FIELD)
#undef APPLY_FIELD

            if (patch.Has("beatmaps")) {
                Napi::Value value = patch.Get("beatmaps");
                if (!value.IsArray()) {
                    parser.last_error = "beatmaps must be an array";
                    return false;
                }
                Napi::Array arr = value.As<Napi::Array>();
                std::vector<osu_db_beatmap> next;
                next.reserve(arr.Length());
                for (uint32_t i = 0; i < arr.Length(); i++) {
                    osu_db_beatmap item{};
                    if (!parse_osu_db_beatmap(arr.Get(i), item, err)) {
                        parser.last_error = err;
                        return false;
                    }
                    next.push_back(std::move(item));
                }
                data.beatmaps = std::move(next);
            }

            data.beatmaps_count = static_cast<int32_t>(data.beatmaps.size());
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

    void register_osu_db(Napi::Env env, Napi::Object exports) {
        exports.Set("create_osu_db_parser", Napi::Function::New(env, create_osu_db_parser));
        exports.Set("free_osu_db_parser", Napi::Function::New(env, free_osu_db_parser));
        exports.Set("osu_db_parser_parse", Napi::Function::New(env, osu_db_parser_parse));
        exports.Set("osu_db_parser_write", Napi::Function::New(env, osu_db_parser_write));
        exports.Set("osu_db_parser_last_error", Napi::Function::New(env, osu_db_parser_last_error));
        exports.Set("osu_db_parser_get", Napi::Function::New(env, osu_db_parser_get));
        exports.Set("osu_db_parser_update", Napi::Function::New(env, osu_db_parser_update));
        exports.Set("osu_db_parser_get_by_name", Napi::Function::New(env, osu_db_parser_get_by_name));
    }
}
