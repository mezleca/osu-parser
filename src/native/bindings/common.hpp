#pragma once

#include <cstring>
#include <napi.h>
#include <optional>
#include <string>
#include <vector>

namespace osu_bindings {
    template <typename T> T* get_ptr(const Napi::CallbackInfo& info, size_t index) {
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

    template <typename T> Napi::Value create_instance(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        T* instance = new T();
        uint64_t handle = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(instance));
        return Napi::BigInt::New(env, handle);
    }

    template <typename T> Napi::Value free_instance(const Napi::CallbackInfo& info) {
        T* instance = get_ptr<T>(info, 0);

        if (instance != nullptr) {
            instance->free_instance();
        }

        return info.Env().Undefined();
    }

    template <typename T> Napi::Value parse_instance(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        T* instance = get_ptr<T>(info, 0);

        if (instance == nullptr || info.Length() < 2 || !info[1].IsString()) {
            return Napi::Boolean::New(env, false);
        }

        std::string location = info[1].As<Napi::String>();
        bool result = instance->parse(location);
        return Napi::Boolean::New(env, result);
    }

    template <typename T> Napi::Value write_instance(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        T* instance = get_ptr<T>(info, 0);

        if (instance == nullptr) {
            Napi::Error::New(env, "invalid parser handle").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        bool result = instance->write();
        if (!result) {
            std::string message =
                instance->parser.last_error.empty() ? "write not implemented" : instance->parser.last_error;
            Napi::Error::New(env, message).ThrowAsJavaScriptException();
            return env.Undefined();
        }

        return Napi::Boolean::New(env, true);
    }

    template <typename T> Napi::Value last_error_instance(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        T* instance = get_ptr<T>(info, 0);

        if (instance == nullptr) {
            return env.Null();
        }

        return Napi::String::New(env, instance->parser.last_error);
    }

    inline Napi::Value bytes_to_uint8array(Napi::Env env, const std::vector<uint8_t>& bytes) {
        Napi::ArrayBuffer buffer = Napi::ArrayBuffer::New(env, bytes.size());

        if (!bytes.empty()) {
            std::memcpy(buffer.Data(), bytes.data(), bytes.size());
        }

        return Napi::Uint8Array::New(env, bytes.size(), buffer, 0);
    }

    inline Napi::Value optional_double_to_js(Napi::Env env, const std::optional<double>& value) {
        if (value.has_value()) {
            return Napi::Number::New(env, value.value());
        }

        return env.Null();
    }
}
