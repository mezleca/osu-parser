#include "common.hpp"

#include "osdb/osdb.hpp"
#include "parser/parser_base.hpp"

namespace osu_bindings {
    using osdb_instance = parser_base<osdb_data, osdb_parser>;

    Napi::Object osdb_beatmap_to_js(Napi::Env& env, const osdb_beatmap& beatmap) {
        Napi::Object obj = Napi::Object::New(env);

        obj.Set("difficulty_id", beatmap.difficulty_id);
        obj.Set("beatmapset_id", beatmap.beatmapset_id);
        obj.Set("artist", beatmap.artist);
        obj.Set("title", beatmap.title);
        obj.Set("difficulty", beatmap.difficulty);
        obj.Set("checksum", beatmap.checksum);
        obj.Set("user_comment", beatmap.user_comment);
        obj.Set("mode", beatmap.mode);
        obj.Set("difficulty_rating", beatmap.difficulty_rating);

        return obj;
    }

    Napi::Object osdb_collection_to_js(Napi::Env& env, const osdb_collection& collection) {
        Napi::Object obj = Napi::Object::New(env);

        obj.Set("name", collection.name);
        obj.Set("online_id", collection.online_id);

        Napi::Array beatmaps = Napi::Array::New(env, collection.beatmaps.size());

        for (size_t i = 0; i < collection.beatmaps.size(); i++) {
            beatmaps.Set(static_cast<uint32_t>(i), osdb_beatmap_to_js(env, collection.beatmaps[i]));
        }

        obj.Set("beatmaps", beatmaps);

        Napi::Array hash_only = Napi::Array::New(env, collection.hash_only_beatmaps.size());

        for (size_t i = 0; i < collection.hash_only_beatmaps.size(); i++) {
            hash_only.Set(static_cast<uint32_t>(i), collection.hash_only_beatmaps[i]);
        }

        obj.Set("hash_only_beatmaps", hash_only);
        return obj;
    }

    Napi::Object osdb_data_to_js(Napi::Env& env, const osdb_data& data) {
        Napi::Object obj = Napi::Object::New(env);

        obj.Set("save_data", static_cast<double>(data.save_data));
        obj.Set("last_editor", data.last_editor);
        obj.Set("count", data.count);

        Napi::Array collections = Napi::Array::New(env, data.collections.size());

        for (size_t i = 0; i < data.collections.size(); i++) {
            collections.Set(static_cast<uint32_t>(i), osdb_collection_to_js(env, data.collections[i]));
        }

        obj.Set("collections", collections);
        return obj;
    }

    Napi::Value create_osdb_parser(const Napi::CallbackInfo& info) {
        return create_instance<osdb_instance>(info);
    }

    Napi::Value free_osdb_parser(const Napi::CallbackInfo& info) {
        return free_instance<osdb_instance>(info);
    }

    Napi::Value osdb_parser_parse(const Napi::CallbackInfo& info) {
        return parse_instance<osdb_instance>(info);
    }

    Napi::Value osdb_parser_write(const Napi::CallbackInfo& info) {
        return write_instance<osdb_instance>(info);
    }

    Napi::Value osdb_parser_last_error(const Napi::CallbackInfo& info) {
        return last_error_instance<osdb_instance>(info);
    }

    Napi::Value osdb_parser_get(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osdb_instance* instance = get_ptr<osdb_instance>(info, 0);

        if (instance == nullptr) {
            return env.Null();
        }

        return osdb_data_to_js(env, instance->data);
    }

    void register_osdb(Napi::Env env, Napi::Object exports) {
        exports.Set("create_osdb_parser", Napi::Function::New(env, create_osdb_parser));
        exports.Set("free_osdb_parser", Napi::Function::New(env, free_osdb_parser));
        exports.Set("osdb_parser_parse", Napi::Function::New(env, osdb_parser_parse));
        exports.Set("osdb_parser_write", Napi::Function::New(env, osdb_parser_write));
        exports.Set("osdb_parser_last_error", Napi::Function::New(env, osdb_parser_last_error));
        exports.Set("osdb_parser_get", Napi::Function::New(env, osdb_parser_get));
    }
}
