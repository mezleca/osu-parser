#include <napi.h>

#include "../beatmap/beatmap.hpp"
#include "../osdb/osdb.hpp"
#include "../osu/osu.hpp"

namespace {
    template <typename T>
    T* get_ptr(const Napi::CallbackInfo& info, size_t index) {
        if (info.Length() <= index || !info[index].IsBigInt()) {
            return nullptr;
        }

        bool lossless = false;
        uint64_t value = info[index].As<Napi::BigInt>().Uint64Value(&lossless);
        if (!lossless || value == 0) {
            return nullptr;
        }

        return reinterpret_cast<T*>(static_cast<uintptr_t>(value));
    }

    template <typename DataT, typename ParserT>
    struct parser_base {
        DataT data;
        ParserT parser;

        parser_base() {
            parser.data = &data;
        }

        virtual ~parser_base() = default;

        bool parse(const std::string& location) {
            return parser.parse(location);
        }

        bool write() {
            return parser.write();
        }

        virtual void free_instance() {
            delete this;
        }
    };

    template <typename T>
    Napi::Value create_instance(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        T* instance = new T();
        uint64_t handle = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(instance));
        return Napi::BigInt::New(env, handle);
    }

    template <typename T>
    Napi::Value free_instance(const Napi::CallbackInfo& info) {
        T* instance = get_ptr<T>(info, 0);

        if (instance != nullptr) {
            instance->free_instance();
        }

        return info.Env().Undefined();
    }
}

using beatmap_instance = parser_base<osu_beatmap, beatmap_parser>;
using osu_db_instance = parser_base<osu_legacy_database, osu_db_parser>;
using osu_collection_db_instance = parser_base<osu_collection_db, osu_collection_db_parser>;
using osu_scores_db_instance = parser_base<osu_scores_db, osu_scores_db_parser>;
using osu_replay_instance = parser_base<osu_replay, osu_replay_parser>;
using osdb_instance = parser_base<osdb_data, osdb_parser>;

Napi::Value create_beatmap_parser(const Napi::CallbackInfo& info) {
    return create_instance<beatmap_instance>(info);
}

Napi::Value free_beatmap_parser(const Napi::CallbackInfo& info) {
    return free_instance<beatmap_instance>(info);
}

Napi::Value beatmap_parser_parse(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    beatmap_instance* instance = get_ptr<beatmap_instance>(info, 0);

    if (instance == nullptr || info.Length() < 2 || !info[1].IsString()) {
        return Napi::Boolean::New(env, false);
    }

    std::string location = info[1].As<Napi::String>();
    bool result = instance->parse(location);
    return Napi::Boolean::New(env, result);
}

Napi::Value beatmap_parser_write(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    beatmap_instance* instance = get_ptr<beatmap_instance>(info, 0);

    if (instance == nullptr) {
        return Napi::Boolean::New(env, false);
    }

    bool result = instance->write();
    return Napi::Boolean::New(env, result);
}

Napi::Value create_osu_db_parser(const Napi::CallbackInfo& info) {
    return create_instance<osu_db_instance>(info);
}

Napi::Value free_osu_db_parser(const Napi::CallbackInfo& info) {
    return free_instance<osu_db_instance>(info);
}

Napi::Value osu_db_parser_parse(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    osu_db_instance* instance = get_ptr<osu_db_instance>(info, 0);

    if (instance == nullptr || info.Length() < 2 || !info[1].IsString()) {
        return Napi::Boolean::New(env, false);
    }

    std::string location = info[1].As<Napi::String>();
    bool result = instance->parse(location);
    return Napi::Boolean::New(env, result);
}

Napi::Value osu_db_parser_write(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    osu_db_instance* instance = get_ptr<osu_db_instance>(info, 0);

    if (instance == nullptr) {
        return Napi::Boolean::New(env, false);
    }

    bool result = instance->write();
    return Napi::Boolean::New(env, result);
}

Napi::Value create_osu_collection_db_parser(const Napi::CallbackInfo& info) {
    return create_instance<osu_collection_db_instance>(info);
}

Napi::Value free_osu_collection_db_parser(const Napi::CallbackInfo& info) {
    return free_instance<osu_collection_db_instance>(info);
}

Napi::Value osu_collection_db_parser_parse(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    osu_collection_db_instance* instance = get_ptr<osu_collection_db_instance>(info, 0);

    if (instance == nullptr || info.Length() < 2 || !info[1].IsString()) {
        return Napi::Boolean::New(env, false);
    }

    std::string location = info[1].As<Napi::String>();
    bool result = instance->parse(location);
    return Napi::Boolean::New(env, result);
}

Napi::Value osu_collection_db_parser_write(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    osu_collection_db_instance* instance = get_ptr<osu_collection_db_instance>(info, 0);
    if (instance == nullptr) {
        return Napi::Boolean::New(env, false);
    }

    bool result = instance->write();
    return Napi::Boolean::New(env, result);
}

Napi::Value create_osu_scores_db_parser(const Napi::CallbackInfo& info) {
    return create_instance<osu_scores_db_instance>(info);
}

Napi::Value free_osu_scores_db_parser(const Napi::CallbackInfo& info) {
    return free_instance<osu_scores_db_instance>(info);
}

Napi::Value osu_scores_db_parser_parse(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    osu_scores_db_instance* instance = get_ptr<osu_scores_db_instance>(info, 0);
    if (instance == nullptr || info.Length() < 2 || !info[1].IsString()) {
        return Napi::Boolean::New(env, false);
    }

    std::string location = info[1].As<Napi::String>();
    bool result = instance->parse(location);
    return Napi::Boolean::New(env, result);
}

Napi::Value osu_scores_db_parser_write(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    osu_scores_db_instance* instance = get_ptr<osu_scores_db_instance>(info, 0);
    if (instance == nullptr) {
        return Napi::Boolean::New(env, false);
    }

    bool result = instance->write();
    return Napi::Boolean::New(env, result);
}

Napi::Value create_osu_replay_parser(const Napi::CallbackInfo& info) {
    return create_instance<osu_replay_instance>(info);
}

Napi::Value free_osu_replay_parser(const Napi::CallbackInfo& info) {
    return free_instance<osu_replay_instance>(info);
}

Napi::Value osu_replay_parser_parse(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    osu_replay_instance* instance = get_ptr<osu_replay_instance>(info, 0);
    if (instance == nullptr || info.Length() < 2 || !info[1].IsString()) {
        return Napi::Boolean::New(env, false);
    }

    std::string location = info[1].As<Napi::String>();
    bool result = instance->parse(location);
    return Napi::Boolean::New(env, result);
}

Napi::Value osu_replay_parser_write(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    osu_replay_instance* instance = get_ptr<osu_replay_instance>(info, 0);

    if (instance == nullptr) {
        return Napi::Boolean::New(env, false);
    }

    bool result = instance->write();
    return Napi::Boolean::New(env, result);
}

Napi::Value create_osdb_parser(const Napi::CallbackInfo& info) {
    return create_instance<osdb_instance>(info);
}

Napi::Value free_osdb_parser(const Napi::CallbackInfo& info) {
    return free_instance<osdb_instance>(info);
}

Napi::Value osdb_parser_parse(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    osdb_instance* instance = get_ptr<osdb_instance>(info, 0);

    if (instance == nullptr || info.Length() < 2 || !info[1].IsString()) {
        return Napi::Boolean::New(env, false);
    }

    std::string location = info[1].As<Napi::String>();
    bool result = instance->parse(location);
    return Napi::Boolean::New(env, result);
}

Napi::Value osdb_parser_write(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    osdb_instance* instance = get_ptr<osdb_instance>(info, 0);

    if (instance == nullptr) {
        return Napi::Boolean::New(env, false);
    }

    bool result = instance->write();
    return Napi::Boolean::New(env, result);
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
    exports.Set("create_beatmap_parser", Napi::Function::New(env, create_beatmap_parser));
    exports.Set("free_beatmap_parser", Napi::Function::New(env, free_beatmap_parser));
    exports.Set("beatmap_parser_parse", Napi::Function::New(env, beatmap_parser_parse));
    exports.Set("beatmap_parser_write", Napi::Function::New(env, beatmap_parser_write));

    exports.Set("create_osu_db_parser", Napi::Function::New(env, create_osu_db_parser));
    exports.Set("free_osu_db_parser", Napi::Function::New(env, free_osu_db_parser));
    exports.Set("osu_db_parser_parse", Napi::Function::New(env, osu_db_parser_parse));
    exports.Set("osu_db_parser_write", Napi::Function::New(env, osu_db_parser_write));

    exports.Set("create_osu_collection_db_parser", Napi::Function::New(env, create_osu_collection_db_parser));
    exports.Set("free_osu_collection_db_parser", Napi::Function::New(env, free_osu_collection_db_parser));
    exports.Set("osu_collection_db_parser_parse", Napi::Function::New(env, osu_collection_db_parser_parse));
    exports.Set("osu_collection_db_parser_write", Napi::Function::New(env, osu_collection_db_parser_write));

    exports.Set("create_osu_scores_db_parser", Napi::Function::New(env, create_osu_scores_db_parser));
    exports.Set("free_osu_scores_db_parser", Napi::Function::New(env, free_osu_scores_db_parser));
    exports.Set("osu_scores_db_parser_parse", Napi::Function::New(env, osu_scores_db_parser_parse));
    exports.Set("osu_scores_db_parser_write", Napi::Function::New(env, osu_scores_db_parser_write));

    exports.Set("create_osu_replay_parser", Napi::Function::New(env, create_osu_replay_parser));
    exports.Set("free_osu_replay_parser", Napi::Function::New(env, free_osu_replay_parser));
    exports.Set("osu_replay_parser_parse", Napi::Function::New(env, osu_replay_parser_parse));
    exports.Set("osu_replay_parser_write", Napi::Function::New(env, osu_replay_parser_write));

    exports.Set("create_osdb_parser", Napi::Function::New(env, create_osdb_parser));
    exports.Set("free_osdb_parser", Napi::Function::New(env, free_osdb_parser));
    exports.Set("osdb_parser_parse", Napi::Function::New(env, osdb_parser_parse));
    exports.Set("osdb_parser_write", Napi::Function::New(env, osdb_parser_write));

    return exports;
}

NODE_API_MODULE(osu_parser, init)
