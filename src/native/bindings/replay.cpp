#include "common.hpp"

#include "osu/osu.hpp"
#include "parser/parser_base.hpp"

namespace osu_bindings {
    using osu_replay_instance = parser_base<osu_replay, osu_replay_parser>;

    Napi::Object replay_to_js(Napi::Env& env, const osu_replay& replay) {
        return full_score_to_js(env, replay);
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

        return instance->with_lock([&](osu_replay& data, osu_replay_parser&) { return replay_to_js(env, data); });
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
