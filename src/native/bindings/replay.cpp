#include "common.hpp"

#include "osu/osu.hpp"
#include "parser/parser_base.hpp"

namespace osu_bindings {
    using osu_replay_instance = parser_base<osu_replay, osu_replay_parser>;

    Napi::Object replay_to_js(Napi::Env& env, const osu_replay& replay) {
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("mode", replay.mode);
        obj.Set("version", replay.version);
        obj.Set("beatmap_md5", replay.beatmap_md5);
        obj.Set("player_name", replay.player_name);
        obj.Set("replay_md5", replay.replay_md5);
        obj.Set("count_300", replay.count_300);
        obj.Set("count_100", replay.count_100);
        obj.Set("count_50", replay.count_50);
        obj.Set("count_geki", replay.count_geki);
        obj.Set("count_katu", replay.count_katu);
        obj.Set("count_miss", replay.count_miss);
        obj.Set("score", replay.score);
        obj.Set("max_combo", replay.max_combo);
        obj.Set("perfect", replay.perfect);
        obj.Set("mods", replay.mods);
        obj.Set("life_bar_graph", replay.life_bar_graph);
        obj.Set("timestamp", Napi::BigInt::New(env, replay.timestamp));
        obj.Set("replay_data_length", replay.replay_data_length);
        obj.Set("replay_data", bytes_to_uint8array(env, replay.replay_data));
        obj.Set("online_score_id", Napi::BigInt::New(env, replay.online_score_id));
        obj.Set("additional_mod_info", optional_double_to_js(env, replay.additional_mod_info));
        return obj;
    }

    Napi::Value create_osu_replay_parser(const Napi::CallbackInfo& info) {
        return create_instance<osu_replay_instance>(info);
    }

    Napi::Value free_osu_replay_parser(const Napi::CallbackInfo& info) {
        return free_instance<osu_replay_instance>(info);
    }

    Napi::Value osu_replay_parser_parse(const Napi::CallbackInfo& info) {
        return parse_instance_async<osu_replay_instance>(info, "osu_replay_parse");
    }

    Napi::Value osu_replay_parser_write(const Napi::CallbackInfo& info) {
        return write_instance_async<osu_replay_instance>(info, "osu_replay_write");
    }

    Napi::Value osu_replay_parser_last_error(const Napi::CallbackInfo& info) {
        return last_error_instance<osu_replay_instance>(info);
    }

    Napi::Value osu_replay_parser_get(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osu_replay_instance* instance = get_ptr<osu_replay_instance>(info, 0);

        if (instance == nullptr) {
            return env.Null();
        }

        return replay_to_js(env, instance->data);
    }

    void register_replay(Napi::Env env, Napi::Object exports) {
        exports.Set("create_osu_replay_parser", Napi::Function::New(env, create_osu_replay_parser));
        exports.Set("free_osu_replay_parser", Napi::Function::New(env, free_osu_replay_parser));
        exports.Set("osu_replay_parser_parse", Napi::Function::New(env, osu_replay_parser_parse));
        exports.Set("osu_replay_parser_write", Napi::Function::New(env, osu_replay_parser_write));
        exports.Set("osu_replay_parser_last_error", Napi::Function::New(env, osu_replay_parser_last_error));
        exports.Set("osu_replay_parser_get", Napi::Function::New(env, osu_replay_parser_get));
    }
}
