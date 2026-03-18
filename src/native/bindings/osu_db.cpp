#include "common.hpp"

#include "osu/osu.hpp"
#include "parser/parser_base.hpp"

namespace osu_bindings {
    using osu_db_instance = parser_base<osu_legacy_database, osu_db_parser>;

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
        obj.Set("last_modification_time", static_cast<double>(beatmap.last_modification_time));
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
        obj.Set("last_played", static_cast<double>(beatmap.last_played));
        obj.Set("is_osz2", beatmap.is_osz2);
        obj.Set("folder_name", beatmap.folder_name);
        obj.Set("last_checked", static_cast<double>(beatmap.last_checked));
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
        obj.Set("account_unlock_time", static_cast<double>(db.account_unlock_time));
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
        return parse_instance<osu_db_instance>(info);
    }

    Napi::Value osu_db_parser_write(const Napi::CallbackInfo& info) {
        return write_instance<osu_db_instance>(info);
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

        return osu_db_to_js(env, instance->data);
    }

    void register_osu_db(Napi::Env env, Napi::Object exports) {
        exports.Set("create_osu_db_parser", Napi::Function::New(env, create_osu_db_parser));
        exports.Set("free_osu_db_parser", Napi::Function::New(env, free_osu_db_parser));
        exports.Set("osu_db_parser_parse", Napi::Function::New(env, osu_db_parser_parse));
        exports.Set("osu_db_parser_write", Napi::Function::New(env, osu_db_parser_write));
        exports.Set("osu_db_parser_last_error", Napi::Function::New(env, osu_db_parser_last_error));
        exports.Set("osu_db_parser_get", Napi::Function::New(env, osu_db_parser_get));
    }
}
