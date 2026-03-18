#include "common.hpp"

#include "osu/osu.hpp"
#include "parser/parser_base.hpp"

namespace osu_bindings {
    using osu_scores_db_instance = parser_base<osu_scores_db, osu_scores_db_parser>;

    Napi::Object score_to_js(Napi::Env& env, const osu_score& score) {
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("mode", score.mode);
        obj.Set("version", score.version);
        obj.Set("beatmap_md5", score.beatmap_md5);
        obj.Set("player_name", score.player_name);
        obj.Set("replay_md5", score.replay_md5);
        obj.Set("count_300", score.count_300);
        obj.Set("count_100", score.count_100);
        obj.Set("count_50", score.count_50);
        obj.Set("count_geki", score.count_geki);
        obj.Set("count_katu", score.count_katu);
        obj.Set("count_miss", score.count_miss);
        obj.Set("score", score.score);
        obj.Set("max_combo", score.max_combo);
        obj.Set("perfect", score.perfect);
        obj.Set("mods", score.mods);
        obj.Set("life_bar_graph", score.life_bar_graph);
        obj.Set("timestamp", Napi::BigInt::New(env, score.timestamp));
        obj.Set("replay_data_length", score.replay_data_length);
        obj.Set("replay_data", bytes_to_uint8array(env, score.replay_data));
        obj.Set("online_score_id", Napi::BigInt::New(env, score.online_score_id));
        obj.Set("additional_mod_info", optional_double_to_js(env, score.additional_mod_info));
        return obj;
    }

    Napi::Object scores_beatmap_to_js(Napi::Env& env, const osu_scores_beatmap& beatmap) {
        Napi::Object obj = Napi::Object::New(env);

        obj.Set("beatmap_md5", beatmap.beatmap_md5);
        obj.Set("scores_count", beatmap.scores_count);

        Napi::Array scores = Napi::Array::New(env, beatmap.scores.size());

        for (size_t i = 0; i < beatmap.scores.size(); i++) {
            scores.Set(static_cast<uint32_t>(i), score_to_js(env, beatmap.scores[i]));
        }

        obj.Set("scores", scores);
        return obj;
    }

    Napi::Object scores_db_to_js(Napi::Env& env, const osu_scores_db& db) {
        Napi::Object obj = Napi::Object::New(env);

        obj.Set("version", db.version);
        obj.Set("beatmaps_count", db.beatmaps_count);

        Napi::Array beatmaps = Napi::Array::New(env, db.beatmaps.size());

        for (size_t i = 0; i < db.beatmaps.size(); i++) {
            beatmaps.Set(static_cast<uint32_t>(i), scores_beatmap_to_js(env, db.beatmaps[i]));
        }

        obj.Set("beatmaps", beatmaps);
        return obj;
    }

    Napi::Value create_osu_scores_db_parser(const Napi::CallbackInfo& info) {
        return create_instance<osu_scores_db_instance>(info);
    }

    Napi::Value free_osu_scores_db_parser(const Napi::CallbackInfo& info) {
        return free_instance<osu_scores_db_instance>(info);
    }

    Napi::Value osu_scores_db_parser_parse(const Napi::CallbackInfo& info) {
        return parse_instance<osu_scores_db_instance>(info);
    }

    Napi::Value osu_scores_db_parser_write(const Napi::CallbackInfo& info) {
        return write_instance<osu_scores_db_instance>(info);
    }

    Napi::Value osu_scores_db_parser_last_error(const Napi::CallbackInfo& info) {
        return last_error_instance<osu_scores_db_instance>(info);
    }

    Napi::Value osu_scores_db_parser_get(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osu_scores_db_instance* instance = get_ptr<osu_scores_db_instance>(info, 0);

        if (instance == nullptr) {
            return env.Null();
        }

        return scores_db_to_js(env, instance->data);
    }

    void register_scores_db(Napi::Env env, Napi::Object exports) {
        exports.Set("create_osu_scores_db_parser", Napi::Function::New(env, create_osu_scores_db_parser));
        exports.Set("free_osu_scores_db_parser", Napi::Function::New(env, free_osu_scores_db_parser));
        exports.Set("osu_scores_db_parser_parse", Napi::Function::New(env, osu_scores_db_parser_parse));
        exports.Set("osu_scores_db_parser_write", Napi::Function::New(env, osu_scores_db_parser_write));
        exports.Set("osu_scores_db_parser_last_error", Napi::Function::New(env, osu_scores_db_parser_last_error));
        exports.Set("osu_scores_db_parser_get", Napi::Function::New(env, osu_scores_db_parser_get));
    }
}
