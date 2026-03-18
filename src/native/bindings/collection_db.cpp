#include "common.hpp"

#include "osu/osu.hpp"
#include "parser/parser_base.hpp"
#include "update_helpers.hpp"

namespace osu_bindings {
    using osu_collection_db_instance = parser_base<osu_collection_db, osu_collection_db_parser>;

    bool update_collection(osu_collection& collection, const Napi::Object& patch, std::string& err) {
        if (!get_optional_string(patch, "name", collection.name, err)) {
            return false;
        }

        const bool has_count = patch.Has("beatmaps_count");
        if (has_count && !get_optional_int32(patch, "beatmaps_count", collection.beatmaps_count, err)) {
            return false;
        }

        if (patch.Has("beatmap_md5")) {
            Napi::Value value = patch.Get("beatmap_md5");
            auto parse_md5 = [](const Napi::Value& v, std::string& out, std::string& err) {
                return parse_string_value(v, out, err);
            };
            if (!value.IsArray()) {
                err = "beatmap_md5 must be an array";
                return false;
            }
            Napi::Array arr = value.As<Napi::Array>();
            std::vector<std::string> next;
            next.reserve(arr.Length());
            for (uint32_t i = 0; i < arr.Length(); i++) {
                std::string item;
                if (!parse_md5(arr.Get(i), item, err)) {
                    return false;
                }
                next.push_back(std::move(item));
            }
            collection.beatmap_md5 = std::move(next);
        }

        if (!has_count) {
            collection.beatmaps_count = static_cast<int32_t>(collection.beatmap_md5.size());
        }
        return true;
    }

    bool parse_collection(const Napi::Value& value, osu_collection& out, std::string& err) {
        if (!is_object(value)) {
            err = "collection must be an object";
            return false;
        }
        Napi::Object obj = value.As<Napi::Object>();
        return update_collection(out, obj, err);
    }

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
        return parse_instance_async<osu_collection_db_instance>(info, "osu_collection_db_parse");
    }

    Napi::Value osu_collection_db_parser_write(const Napi::CallbackInfo& info) {
        return write_instance_async<osu_collection_db_instance>(info, "osu_collection_db_write");
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

        return instance->with_lock(
            [&](osu_collection_db& data, osu_collection_db_parser&) { return collection_db_to_js(env, data); });
    }

    Napi::Value collection_db_get_by_key(Napi::Env& env, const osu_collection_db& data, const std::string& key) {
        if (key == "version")
            return Napi::Number::New(env, data.version);
        if (key == "collections_count")
            return Napi::Number::New(env, data.collections_count);
        if (key == "collections") {
            Napi::Array collections = Napi::Array::New(env, data.collections.size());
            for (size_t i = 0; i < data.collections.size(); i++) {
                collections.Set(static_cast<uint32_t>(i), collection_to_js(env, data.collections[i]));
            }
            return collections;
        }

        return env.Undefined();
    }

    Napi::Value osu_collection_db_parser_update(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osu_collection_db_instance* instance = get_ptr<osu_collection_db_instance>(info, 0);

        if (instance == nullptr) {
            Napi::Error::New(env, "invalid parser handle").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        if (info.Length() < 2 || !is_object(info[1])) {
            Napi::TypeError::New(env, "update patch must be an object").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        std::string err;

        bool ok = instance->with_lock([&](osu_collection_db& data, osu_collection_db_parser& parser) {
            Napi::Object patch = info[1].As<Napi::Object>();

            if (!get_optional_int32(patch, "version", data.version, err)) {
                parser.last_error = err;
                return false;
            }

            if (patch.Has("collections")) {
                Napi::Value value = patch.Get("collections");
                if (!value.IsArray()) {
                    parser.last_error = "collections must be an array";
                    return false;
                }
                Napi::Array arr = value.As<Napi::Array>();
                std::vector<osu_collection> next;
                next.reserve(arr.Length());
                for (uint32_t i = 0; i < arr.Length(); i++) {
                    osu_collection item{};
                    if (!parse_collection(arr.Get(i), item, err)) {
                        parser.last_error = err;
                        return false;
                    }
                    next.push_back(std::move(item));
                }
                data.collections = std::move(next);
            }

            data.collections_count = static_cast<int32_t>(data.collections.size());

            for (auto& collection : data.collections) {
                collection.beatmaps_count = static_cast<int32_t>(collection.beatmap_md5.size());
            }

            parser.last_error.clear();
            return true;
        });

        if (!ok) {
            std::string message = instance->parser.last_error.empty() ? "update failed" : instance->parser.last_error;
            Napi::Error::New(env, message).ThrowAsJavaScriptException();
            return env.Undefined();
        }

        return Napi::Boolean::New(env, true);
    }

    Napi::Value osu_collection_db_parser_get_by_name(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osu_collection_db_instance* instance = get_ptr<osu_collection_db_instance>(info, 0);

        if (instance == nullptr || info.Length() < 2 || !info[1].IsString()) {
            return env.Undefined();
        }

        std::string key = info[1].As<Napi::String>();

        return instance->with_lock([&](osu_collection_db& data, osu_collection_db_parser&) {
            return collection_db_get_by_key(env, data, key);
        });
    }

    void register_collection_db(Napi::Env env, Napi::Object exports) {
        exports.Set("create_osu_collection_db_parser", Napi::Function::New(env, create_osu_collection_db_parser));
        exports.Set("free_osu_collection_db_parser", Napi::Function::New(env, free_osu_collection_db_parser));
        exports.Set("osu_collection_db_parser_parse", Napi::Function::New(env, osu_collection_db_parser_parse));
        exports.Set("osu_collection_db_parser_write", Napi::Function::New(env, osu_collection_db_parser_write));
        exports.Set("osu_collection_db_parser_last_error",
                    Napi::Function::New(env, osu_collection_db_parser_last_error));
        exports.Set("osu_collection_db_parser_get", Napi::Function::New(env, osu_collection_db_parser_get));
        exports.Set("osu_collection_db_parser_update", Napi::Function::New(env, osu_collection_db_parser_update));
        exports.Set("osu_collection_db_parser_get_by_name",
                    Napi::Function::New(env, osu_collection_db_parser_get_by_name));
    }
}
