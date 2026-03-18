#include "common.hpp"

#include "osdb/osdb.hpp"
#include "parser/parser_base.hpp"
#include "update_helpers.hpp"

namespace osu_bindings {
    using osdb_instance = parser_base<osdb_data, osdb_parser>;

    bool update_osdb_beatmap(osdb_beatmap& beatmap, const Napi::Object& patch, std::string& err) {
        return get_optional_int32(patch, "difficulty_id", beatmap.difficulty_id, err) &&
               get_optional_int32(patch, "beatmapset_id", beatmap.beatmapset_id, err) &&
               get_optional_string(patch, "artist", beatmap.artist, err) &&
               get_optional_string(patch, "title", beatmap.title, err) &&
               get_optional_string(patch, "difficulty", beatmap.difficulty, err) &&
               get_optional_string(patch, "checksum", beatmap.checksum, err) &&
               get_optional_string(patch, "user_comment", beatmap.user_comment, err) &&
               get_optional_int32(patch, "mode", beatmap.mode, err) &&
               get_optional_double(patch, "difficulty_rating", beatmap.difficulty_rating, err);
    }

    bool parse_osdb_beatmap(const Napi::Value& value, osdb_beatmap& out, std::string& err) {
        if (!is_object(value)) {
            err = "beatmap must be an object";
            return false;
        }
        return update_osdb_beatmap(out, value.As<Napi::Object>(), err);
    }

    bool update_osdb_collection(osdb_collection& collection, const Napi::Object& patch, std::string& err) {
        if (!get_optional_string(patch, "name", collection.name, err) ||
            !get_optional_int32(patch, "online_id", collection.online_id, err)) {
            return false;
        }

        if (!update_array_field(patch, "beatmaps", collection.beatmaps, parse_osdb_beatmap, err)) {
            return false;
        }

        if (!update_string_array_field(patch, "hash_only_beatmaps", collection.hash_only_beatmaps, err)) {
            return false;
        }

        return true;
    }

    bool parse_osdb_collection(const Napi::Value& value, osdb_collection& out, std::string& err) {
        if (!is_object(value)) {
            err = "collection must be an object";
            return false;
        }
        return update_osdb_collection(out, value.As<Napi::Object>(), err);
    }

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

        obj.Set("version_string", data.version_string);
        obj.Set("save_data", Napi::BigInt::New(env, data.save_data));
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
        return parse_instance_async<osdb_instance>(info, "osdb_parse");
    }

    Napi::Value osdb_parser_write(const Napi::CallbackInfo& info) {
        return write_instance_async<osdb_instance>(info, "osdb_write");
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

        return instance->with_lock([&](osdb_data& data, osdb_parser&) { return osdb_data_to_js(env, data); });
    }

    Napi::Value osdb_get_by_key(Napi::Env& env, const osdb_data& data, const std::string& key) {
        if (key == "version_string")
            return Napi::String::New(env, data.version_string);
        if (key == "save_data")
            return Napi::BigInt::New(env, data.save_data);
        if (key == "last_editor")
            return Napi::String::New(env, data.last_editor);
        if (key == "count")
            return Napi::Number::New(env, data.count);
        if (key == "collections") {
            Napi::Array collections = Napi::Array::New(env, data.collections.size());
            for (size_t i = 0; i < data.collections.size(); i++) {
                collections.Set(static_cast<uint32_t>(i), osdb_collection_to_js(env, data.collections[i]));
            }
            return collections;
        }

        return env.Undefined();
    }

    Napi::Value osdb_parser_update(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osdb_instance* instance = get_ptr<osdb_instance>(info, 0);

        if (instance == nullptr) {
            Napi::Error::New(env, "invalid parser handle").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        if (info.Length() < 2 || !is_object(info[1])) {
            Napi::TypeError::New(env, "update patch must be an object").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        std::string err;
        bool ok = instance->with_lock([&](osdb_data& data, osdb_parser& parser) {
            Napi::Object patch = info[1].As<Napi::Object>();
            osdb_data temp = data;

            if (!get_optional_int64(patch, "save_data", temp.save_data, err) ||
                !get_optional_string(patch, "last_editor", temp.last_editor, err) ||
                !get_optional_int32(patch, "count", temp.count, err)) {
                parser.last_error = err;
                return false;
            }

            if (!update_array_field(patch, "collections", temp.collections, parse_osdb_collection, err)) {
                parser.last_error = err;
                return false;
            }

            temp.count = static_cast<int32_t>(temp.collections.size());
            data = std::move(temp);
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

    Napi::Value osdb_parser_get_by_name(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        osdb_instance* instance = get_ptr<osdb_instance>(info, 0);

        if (instance == nullptr || info.Length() < 2 || !info[1].IsString()) {
            return env.Undefined();
        }

        std::string key = info[1].As<Napi::String>();
        return instance->with_lock([&](osdb_data& data, osdb_parser&) { return osdb_get_by_key(env, data, key); });
    }

    void register_osdb(Napi::Env env, Napi::Object exports) {
        exports.Set("create_osdb_parser", Napi::Function::New(env, create_osdb_parser));
        exports.Set("free_osdb_parser", Napi::Function::New(env, free_osdb_parser));
        exports.Set("osdb_parser_parse", Napi::Function::New(env, osdb_parser_parse));
        exports.Set("osdb_parser_write", Napi::Function::New(env, osdb_parser_write));
        exports.Set("osdb_parser_last_error", Napi::Function::New(env, osdb_parser_last_error));
        exports.Set("osdb_parser_get", Napi::Function::New(env, osdb_parser_get));
        exports.Set("osdb_parser_update", Napi::Function::New(env, osdb_parser_update));
        exports.Set("osdb_parser_get_by_name", Napi::Function::New(env, osdb_parser_get_by_name));
    }
}
