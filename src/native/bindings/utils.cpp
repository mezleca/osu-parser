#include "common.hpp"

#include "osu/filter.hpp"
#include "osu/osu.hpp"
#include "update_helpers.hpp"

namespace osu_bindings {
    static bool parse_timing_points(const Napi::Value& value, std::vector<osu_db_timing_point>& out, std::string& err) {
        if (!value.IsArray()) {
            err = "timing_points must be an array";
            return false;
        }

        Napi::Array arr = value.As<Napi::Array>();
        out.reserve(arr.Length());

        for (uint32_t i = 0; i < arr.Length(); i++) {
            Napi::Value item = arr.Get(i);
            if (!is_object(item)) {
                err = "invalid timing_point";
                return false;
            }

            Napi::Object obj = item.As<Napi::Object>();
            osu_db_timing_point point;

            if (!get_optional_double(obj, "bpm", point.bpm, err) ||
                !get_optional_double(obj, "offset", point.offset, err) ||
                !get_optional_int32(obj, "inherited", point.inherited, err)) {
                return false;
            }

            out.push_back(point);
        }

        return true;
    }

    Napi::Value get_common_bpm(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1) {
            Napi::TypeError::New(env, "timing_points is required").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        std::vector<osu_db_timing_point> timing_points;
        std::string err;

        if (!parse_timing_points(info[0], timing_points, err)) {
            Napi::Error::New(env, err).ThrowAsJavaScriptException();
            return env.Undefined();
        }

        int32_t length = 0;
        if (info.Length() > 1) {
            if (!info[1].IsNumber()) {
                Napi::TypeError::New(env, "length must be a number").ThrowAsJavaScriptException();
                return env.Undefined();
            }
            length = info[1].As<Napi::Number>().Int32Value();
        }

        const double bpm = osu_filter::get_common_bpm(timing_points, length);
        return Napi::Number::New(env, bpm);
    }

    void register_utils(Napi::Env env, Napi::Object exports) {
        exports.Set("get_common_bpm", Napi::Function::New(env, get_common_bpm));
    }
}
