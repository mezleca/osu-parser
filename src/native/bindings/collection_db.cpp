#include "common.hpp"

#include "osu/osu.hpp"
#include "parser/parser_base.hpp"

namespace osu_bindings {
    using osu_collection_db_instance = parser_base<osu_collection_db, osu_collection_db_parser>;

    Napi::Object collection_to_js(Napi::Env& env, const osu_collection& collection) {
        Napi::Object obj = Napi::Object::New(env);

        obj.Set("name", collection.name);
        obj.Set("beatmaps_count", collection.beatmaps_count);

        Napi::Array md5 = Napi::Array::New(env, collection.beatmap_md5.size());

        for (size_t i = 0; i < collection.beatmap_md5.size(); i++) {
            md5.Set(static_cast<uint32_t>(i), collection.beatmap_md5[i]);
        }

        obj.Set("beatmap_md5", md5);
        return obj;
    }

    Napi::Object collection_db_to_js(Napi::Env& env, const osu_collection_db& db) {
        Napi::Object obj = Napi::Object::New(env);

        obj.Set("version", db.version);
        obj.Set("collections_count", db.collections_count);

        Napi::Array collections = Napi::Array::New(env, db.collections.size());

        for (size_t i = 0; i < db.collections.size(); i++) {
            collections.Set(static_cast<uint32_t>(i), collection_to_js(env, db.collections[i]));
        }

        obj.Set("collections", collections);
        return obj;
    }

    Napi::Value create_osu_collection_db_parser(const Napi::CallbackInfo& info) {
        return create_instance<osu_collection_db_instance>(info);
    }

    Napi::Value free_osu_collection_db_parser(const Napi::CallbackInfo& info) {
        return free_instance<osu_collection_db_instance>(info);
    }

    Napi::Value osu_collection_db_parser_parse(const Napi::CallbackInfo& info) {
        return parse_instance<osu_collection_db_instance>(info);
    }

    Napi::Value osu_collection_db_parser_write(const Napi::CallbackInfo& info) {
        return write_instance<osu_collection_db_instance>(info);
    }

    Napi::Value osu_collection_db_parser_last_error(const Napi::CallbackInfo& info) {
        return last_error_instance<osu_collection_db_instance>(info);
    }

    Napi::Value osu_collection_db_parser_get(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osu_collection_db_instance* instance = get_ptr<osu_collection_db_instance>(info, 0);

        if (instance == nullptr) {
            return env.Null();
        }

        return collection_db_to_js(env, instance->data);
    }

    void register_collection_db(Napi::Env env, Napi::Object exports) {
        exports.Set("create_osu_collection_db_parser", Napi::Function::New(env, create_osu_collection_db_parser));
        exports.Set("free_osu_collection_db_parser", Napi::Function::New(env, free_osu_collection_db_parser));
        exports.Set("osu_collection_db_parser_parse", Napi::Function::New(env, osu_collection_db_parser_parse));
        exports.Set("osu_collection_db_parser_write", Napi::Function::New(env, osu_collection_db_parser_write));
        exports.Set("osu_collection_db_parser_last_error",
                    Napi::Function::New(env, osu_collection_db_parser_last_error));
        exports.Set("osu_collection_db_parser_get", Napi::Function::New(env, osu_collection_db_parser_get));
    }
}
