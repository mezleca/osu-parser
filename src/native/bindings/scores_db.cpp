#include "common.hpp"

#include "osu/osu.hpp"
#include "parser/parser_base.hpp"

namespace osu_bindings {
using osu_scores_db_instance = parser_base<osu_scores_db, osu_scores_db_parser>;

Napi::Value create_osu_scores_db_parser(const Napi::CallbackInfo& info) {
    return create_instance<osu_scores_db_instance>(info);
}

Napi::Value free_osu_scores_db_parser(const Napi::CallbackInfo& info) {
    return free_instance<osu_scores_db_instance>(info);
}

Napi::Value osu_scores_db_parser_parse(const Napi::CallbackInfo& info) {
    return parse_instance<osu_scores_db_instance>(info);
}

Napi::Value osu_scores_db_parser_write(const Napi::CallbackInfo& info) {
    return write_instance<osu_scores_db_instance>(info);
}

void register_scores_db(Napi::Env env, Napi::Object exports) {
    exports.Set("create_osu_scores_db_parser", Napi::Function::New(env, create_osu_scores_db_parser));
    exports.Set("free_osu_scores_db_parser", Napi::Function::New(env, free_osu_scores_db_parser));
    exports.Set("osu_scores_db_parser_parse", Napi::Function::New(env, osu_scores_db_parser_parse));
    exports.Set("osu_scores_db_parser_write", Napi::Function::New(env, osu_scores_db_parser_write));
}
}
