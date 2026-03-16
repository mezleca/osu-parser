#include <napi.h>

namespace osu_bindings {
void register_beatmap(Napi::Env env, Napi::Object exports);
void register_osu_db(Napi::Env env, Napi::Object exports);
void register_collection_db(Napi::Env env, Napi::Object exports);
void register_scores_db(Napi::Env env, Napi::Object exports);
void register_replay(Napi::Env env, Napi::Object exports);
void register_osdb(Napi::Env env, Napi::Object exports);
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
    osu_bindings::register_beatmap(env, exports);
    osu_bindings::register_osu_db(env, exports);
    osu_bindings::register_collection_db(env, exports);
    osu_bindings::register_scores_db(env, exports);
    osu_bindings::register_replay(env, exports);
    osu_bindings::register_osdb(env, exports);
    return exports;
}

NODE_API_MODULE(osu_parser, init)
