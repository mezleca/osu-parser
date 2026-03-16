#include "common.hpp"

#include "beatmap/beatmap.hpp"
#include "parser/parser_base.hpp"

namespace osu_bindings {
using beatmap_instance = parser_base<osu_beatmap, beatmap_parser>;

Napi::Value create_beatmap_parser(const Napi::CallbackInfo& info) {
    return create_instance<beatmap_instance>(info);
}

Napi::Value free_beatmap_parser(const Napi::CallbackInfo& info) {
    return free_instance<beatmap_instance>(info);
}

Napi::Value beatmap_parser_parse(const Napi::CallbackInfo& info) {
    return parse_instance<beatmap_instance>(info);
}

Napi::Value beatmap_parser_write(const Napi::CallbackInfo& info) {
    return write_instance<beatmap_instance>(info);
}

void register_beatmap(Napi::Env env, Napi::Object exports) {
    exports.Set("create_beatmap_parser", Napi::Function::New(env, create_beatmap_parser));
    exports.Set("free_beatmap_parser", Napi::Function::New(env, free_beatmap_parser));
    exports.Set("beatmap_parser_parse", Napi::Function::New(env, beatmap_parser_parse));
    exports.Set("beatmap_parser_write", Napi::Function::New(env, beatmap_parser_write));
}
}
