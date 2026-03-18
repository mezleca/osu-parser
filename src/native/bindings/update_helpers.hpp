#pragma once

#include <cmath>
#include <cstdint>
#include <napi.h>
#include <string>
#include <vector>

namespace osu_bindings {
    inline bool is_object(const Napi::Value& value) {
        return value.IsObject() && !value.IsArray() && !value.IsNull();
    }

    inline bool get_optional_string(const Napi::Object& obj, const char* key, std::string& out, std::string& err) {
        if (!obj.Has(key)) {
            return true;
        }

        Napi::Value value = obj.Get(key);

        if (value.IsNull()) {
            out.clear();
            return true;
        }

        if (!value.IsString()) {
            err = std::string("invalid type for ") + key;
            return false;
        }

        out = value.As<Napi::String>();
        return true;
    }

    inline bool parse_string_value(const Napi::Value& value, std::string& out, std::string& err) {
        if (!value.IsString()) {
            err = "expected string";
            return false;
        }

        out = value.As<Napi::String>();
        return true;
    }

    inline bool get_optional_int32(const Napi::Object& obj, const char* key, int32_t& out, std::string& err) {
        if (!obj.Has(key)) {
            return true;
        }

        Napi::Value value = obj.Get(key);

        if (!value.IsNumber()) {
            err = std::string("invalid type for ") + key;
            return false;
        }

        out = value.As<Napi::Number>().Int32Value();
        return true;
    }

    inline bool get_optional_int64(const Napi::Object& obj, const char* key, int64_t& out, std::string& err) {
        if (!obj.Has(key)) {
            return true;
        }

        Napi::Value value = obj.Get(key);

        if (value.IsBigInt()) {
            bool lossless = false;
            out = value.As<Napi::BigInt>().Int64Value(&lossless);
            if (!lossless) {
                err = std::string("invalid bigint for ") + key;
                return false;
            }
            return true;
        }

        if (!value.IsNumber()) {
            err = std::string("invalid type for ") + key;
            return false;
        }

        const double number = value.As<Napi::Number>().DoubleValue();
        constexpr double max_safe = 9007199254740991.0;

        if (!std::isfinite(number) || std::floor(number) != number || number < -max_safe || number > max_safe) {
            err = std::string("invalid number for ") + key;
            return false;
        }

        out = static_cast<int64_t>(number);
        return true;
    }

    inline bool get_optional_double(const Napi::Object& obj, const char* key, double& out, std::string& err) {
        if (!obj.Has(key)) {
            return true;
        }

        Napi::Value value = obj.Get(key);

        if (!value.IsNumber()) {
            err = std::string("invalid type for ") + key;
            return false;
        }

        out = value.As<Napi::Number>().DoubleValue();
        return true;
    }

    inline bool get_optional_bool_int(const Napi::Object& obj, const char* key, int32_t& out, std::string& err) {
        if (!obj.Has(key)) {
            return true;
        }

        Napi::Value value = obj.Get(key);

        if (value.IsBoolean()) {
            out = value.As<Napi::Boolean>().Value() ? 1 : 0;
            return true;
        }

        if (!value.IsNumber()) {
            err = std::string("invalid type for ") + key;
            return false;
        }

        out = value.As<Napi::Number>().Int32Value();
        return true;
    }

    template <typename T, typename ParseFn>
    bool update_array_field(const Napi::Object& obj, const char* key, std::vector<T>& out, ParseFn parse_fn,
                            std::string& err) {
        if (!obj.Has(key)) {
            return true;
        }

        Napi::Value value = obj.Get(key);

        if (!value.IsArray()) {
            err = std::string(key) + " must be an array";
            return false;
        }

        Napi::Array arr = value.As<Napi::Array>();
        std::vector<T> next;
        next.reserve(arr.Length());

        for (uint32_t i = 0; i < arr.Length(); i++) {
            T item{};
            if (!parse_fn(arr.Get(i), item, err)) {
                return false;
            }
            next.push_back(std::move(item));
        }

        out = std::move(next);
        return true;
    }

    inline bool update_string_array_field(const Napi::Object& obj, const char* key, std::vector<std::string>& out,
                                          std::string& err) {
        return update_array_field<std::string>(obj, key, out, parse_string_value, err);
    }
}
