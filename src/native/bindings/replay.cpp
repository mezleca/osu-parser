#include "common.hpp"

#include "osu/osu.hpp"
#include "parser/parser_base.hpp"

namespace osu_bindings {
    using osu_replay_instance = parser_base<osu_replay, osu_replay_parser>;

    Napi::Value create_osu_replay_parser(const Napi::CallbackInfo& info) {
        return create_instance<osu_replay_instance>(info);
    }

    Napi::Value free_osu_replay_parser(const Napi::CallbackInfo& info) {
        return free_instance<osu_replay_instance>(info);
    }

    Napi::Value osu_replay_parser_parse(const Napi::CallbackInfo& info) {
        return parse_instance<osu_replay_instance>(info);
    }

    Napi::Value osu_replay_parser_write(const Napi::CallbackInfo& info) {
        return write_instance<osu_replay_instance>(info);
    }

    void register_replay(Napi::Env env, Napi::Object exports) {
        exports.Set("create_osu_replay_parser", Napi::Function::New(env, create_osu_replay_parser));
        exports.Set("free_osu_replay_parser", Napi::Function::New(env, free_osu_replay_parser));
        exports.Set("osu_replay_parser_parse", Napi::Function::New(env, osu_replay_parser_parse));
        exports.Set("osu_replay_parser_write", Napi::Function::New(env, osu_replay_parser_write));
    }
}