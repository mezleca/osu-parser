#include "common.hpp"

#include "osdb/osdb.hpp"
#include "parser/parser_base.hpp"

namespace osu_bindings {
    using osdb_instance = parser_base<osdb_data, osdb_parser>;

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

    void register_osdb(Napi::Env env, Napi::Object exports) {
        exports.Set("create_osdb_parser", Napi::Function::New(env, create_osdb_parser));
        exports.Set("free_osdb_parser", Napi::Function::New(env, free_osdb_parser));
        exports.Set("osdb_parser_parse", Napi::Function::New(env, osdb_parser_parse));
        exports.Set("osdb_parser_write", Napi::Function::New(env, osdb_parser_write));
    }
}