#pragma once

#include <atomic>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <napi.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "utils/log.hpp"
#include "osu/osu.hpp"

namespace osu_bindings {
    struct handle_entry {
        void* ptr = nullptr;
        void (*deleter)(void*) = nullptr;
        std::atomic<int> refs{0};
        std::atomic<bool> freed{false};
    };

    inline std::atomic<uint64_t> handle_counter{1};
    inline std::mutex handle_mutex;
    inline std::unordered_map<uint64_t, handle_entry> handle_table;

    inline uint64_t alloc_handle(void* ptr, void (*deleter)(void*)) {
        std::lock_guard<std::mutex> lock(handle_mutex);
        uint64_t handle = handle_counter.fetch_add(1, std::memory_order_relaxed);
        auto [it, inserted] =
            handle_table.emplace(std::piecewise_construct, std::forward_as_tuple(handle), std::forward_as_tuple());
        it->second.ptr = ptr;
        it->second.deleter = deleter;
        return handle;
    }

    inline void* lookup_handle(uint64_t handle) {
        std::lock_guard<std::mutex> lock(handle_mutex);
        auto it = handle_table.find(handle);
        if (it == handle_table.end() || it->second.freed.load()) {
            return nullptr;
        }
        return it->second.ptr;
    }

    inline void* retain_handle(uint64_t handle) {
        std::lock_guard<std::mutex> lock(handle_mutex);
        auto it = handle_table.find(handle);
        if (it == handle_table.end() || it->second.freed.load()) {
            return nullptr;
        }
        it->second.refs.fetch_add(1);
        return it->second.ptr;
    }

    inline void release_handle(uint64_t handle) {
        void* ptr = nullptr;
        void (*deleter)(void*) = nullptr;
        {
            std::lock_guard<std::mutex> lock(handle_mutex);
            auto it = handle_table.find(handle);
            if (it == handle_table.end()) {
                return;
            }
            const int refs = it->second.refs.fetch_sub(1) - 1;
            if (!it->second.freed.load() || refs > 0) {
                return;
            }
            ptr = it->second.ptr;
            deleter = it->second.deleter;
            handle_table.erase(it);
        }

        if (deleter && ptr) {
            deleter(ptr);
        }
    }

    inline void free_handle(uint64_t handle) {
        void* ptr = nullptr;
        void (*deleter)(void*) = nullptr;
        {
            std::lock_guard<std::mutex> lock(handle_mutex);
            auto it = handle_table.find(handle);
            if (it == handle_table.end()) {
                return;
            }
            it->second.freed.store(true);
            if (it->second.refs.load() > 0) {
                return;
            }
            ptr = it->second.ptr;
            deleter = it->second.deleter;
            handle_table.erase(it);
        }

        if (deleter && ptr) {
            deleter(ptr);
        }
    }

    inline bool read_handle(const Napi::CallbackInfo& info, size_t index, uint64_t& handle) {
        if (info.Length() <= index || !info[index].IsBigInt()) {
            return false;
        }

        bool lossless = false;
        handle = info[index].As<Napi::BigInt>().Uint64Value(&lossless);

        return lossless && handle != 0;
    }

    template <typename T> auto get_last_error_impl(T* instance, int) -> decltype(instance->parser.last_error) {
        return instance->parser.last_error;
    }

    template <typename T> auto get_last_error_impl(T* instance, long) -> decltype(instance->last_error()) {
        return instance->last_error();
    }

    inline std::string get_last_error_impl(...) {
        return {};
    }

    template <typename T> std::string get_last_error(T* instance) {
        return get_last_error_impl(instance, 0);
    }

    template <typename T> void delete_instance(void* ptr) {
        static_cast<T*>(ptr)->free_instance();
    }

    struct handle_ref {
        uint64_t handle = 0;
        void* ptr = nullptr;

        explicit handle_ref(uint64_t handle_value) : handle(handle_value), ptr(retain_handle(handle_value)) {
        }

        handle_ref(const handle_ref&) = delete;
        handle_ref& operator=(const handle_ref&) = delete;

        handle_ref(handle_ref&& other) noexcept : handle(other.handle), ptr(other.ptr) {
            other.handle = 0;
            other.ptr = nullptr;
        }

        handle_ref& operator=(handle_ref&& other) noexcept {
            if (this != &other) {
                release();
                handle = other.handle;
                ptr = other.ptr;
                other.handle = 0;
                other.ptr = nullptr;
            }
            return *this;
        }

        ~handle_ref() {
            release();
        }

        void release() {
            if (handle != 0) {
                release_handle(handle);
                handle = 0;
                ptr = nullptr;
            }
        }
    };

    inline Napi::Promise resolve_promise(Napi::Env env, const Napi::Value& value) {
        Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
        deferred.Resolve(value);
        return deferred.Promise();
    }

    inline Napi::Promise reject_promise(Napi::Env env, const char* message) {
        Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
        deferred.Reject(Napi::Error::New(env, message).Value());
        return deferred.Promise();
    }

    class LambdaAsyncWorker : public Napi::AsyncWorker {
      public:
        using execute_fn = std::function<void()>;
        using resolve_fn = std::function<Napi::Value(Napi::Env)>;

        LambdaAsyncWorker(Napi::Env env, std::string task_name, execute_fn exec, resolve_fn resolve)
            : Napi::AsyncWorker(env), deferred(Napi::Promise::Deferred::New(env)), task_name(std::move(task_name)),
              exec(std::move(exec)), resolve(std::move(resolve)) {
        }

        void Execute() override {
            try {
                exec();
            } catch (const std::exception& e) {
                SetError(e.what());
            }
        }

        void OnOK() override {
            deferred.Resolve(resolve(Env()));
        }

        void OnError(const Napi::Error& e) override {
            LOG_LINE("[addon]", task_name.c_str(), "error:", e.Message());
            deferred.Reject(e.Value());
        }

        Napi::Promise GetPromise() {
            return deferred.Promise();
        }

      private:
        Napi::Promise::Deferred deferred;
        std::string task_name;
        execute_fn exec;
        resolve_fn resolve;
    };

    inline Napi::Promise queue_worker(LambdaAsyncWorker* worker) {
        Napi::Promise promise = worker->GetPromise();
        worker->Queue();
        return promise;
    }

    template <typename T> Napi::Value parse_instance_async(const Napi::CallbackInfo& info, const char* task_name) {
        Napi::Env env = info.Env();
        uint64_t handle = 0;
        if (!read_handle(info, 0, handle) || info.Length() < 2 || !info[1].IsString()) {
            return reject_promise(env, "invalid parser handle or location");
        }

        std::string location = info[1].As<Napi::String>();
        auto ref = std::make_shared<handle_ref>(handle);
        if (ref->ptr == nullptr) {
            return reject_promise(env, "invalid parser handle");
        }

        return queue_worker(new LambdaAsyncWorker(
            env, task_name,
            [ref, location = std::move(location)]() {
                auto* instance = static_cast<T*>(ref->ptr);
                if (!instance->parse(location)) {
                    const std::string message = get_last_error(instance);
                    throw std::runtime_error(message.empty() ? "parse failed" : message);
                }
            },
            [](Napi::Env env) { return Napi::Boolean::New(env, true); }));
    }

    template <typename T> Napi::Value write_instance_async(const Napi::CallbackInfo& info, const char* task_name) {
        Napi::Env env = info.Env();
        uint64_t handle = 0;
        if (!read_handle(info, 0, handle)) {
            return reject_promise(env, "invalid parser handle");
        }

        auto ref = std::make_shared<handle_ref>(handle);
        if (ref->ptr == nullptr) {
            return reject_promise(env, "invalid parser handle");
        }

        return queue_worker(new LambdaAsyncWorker(
            env, task_name,
            [ref]() {
                auto* instance = static_cast<T*>(ref->ptr);
                if (!instance->write()) {
                    const std::string message = get_last_error(instance);
                    throw std::runtime_error(message.empty() ? "write failed" : message);
                }
            },
            [](Napi::Env env) { return Napi::Boolean::New(env, true); }));
    }

    template <typename T> T* get_ptr(const Napi::CallbackInfo& info, size_t index) {
        uint64_t handle = 0;
        if (!read_handle(info, index, handle)) {
            return nullptr;
        }

        return static_cast<T*>(lookup_handle(handle));
    }

    template <typename T> Napi::Value create_instance(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        auto* instance = new T();
        uint64_t handle = alloc_handle(instance, &delete_instance<T>);
        return Napi::BigInt::New(env, handle);
    }

    template <typename T> Napi::Value free_instance(const Napi::CallbackInfo& info) {
        // T must implement free_instance() for cleanup.
        uint64_t handle = 0;

        if (!read_handle(info, 0, handle)) {
            return info.Env().Undefined();
        }

        free_handle(handle);

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
            const std::string message = get_last_error(instance);
            const std::string final_message = message.empty() ? "write not implemented" : message;
            Napi::Error::New(env, final_message).ThrowAsJavaScriptException();
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

    inline void score_base_to_js(Napi::Env& env, Napi::Object& obj, const osu_score_base& s) {
        obj.Set("mode", s.mode);
        obj.Set("version", s.version);
        obj.Set("beatmap_md5", s.beatmap_md5);
        obj.Set("player_name", s.player_name);
        obj.Set("replay_md5", s.replay_md5);
        obj.Set("count_300", s.count_300);
        obj.Set("count_100", s.count_100);
        obj.Set("count_50", s.count_50);
        obj.Set("count_geki", s.count_geki);
        obj.Set("count_katu", s.count_katu);
        obj.Set("count_miss", s.count_miss);
        obj.Set("score", s.score);
        obj.Set("max_combo", s.max_combo);
        obj.Set("perfect", s.perfect);
        obj.Set("mods", s.mods);
        obj.Set("life_bar_graph", s.life_bar_graph);
        obj.Set("timestamp", Napi::BigInt::New(env, s.timestamp));
        obj.Set("additional_mod_info", optional_double_to_js(env, s.additional_mod_info));
    }

    template <typename T> Napi::Object full_score_to_js(Napi::Env& env, const T& s) {
        Napi::Object obj = Napi::Object::New(env);
        score_base_to_js(env, obj, s);
        obj.Set("replay_data_length", s.replay_data_length);
        obj.Set("replay_data", bytes_to_uint8array(env, s.replay_data));
        obj.Set("online_score_id", Napi::BigInt::New(env, s.online_score_id));
        return obj;
    }
}
