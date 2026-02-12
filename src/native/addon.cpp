#include "definitions.hpp"
#include "log.hpp"
#include "osu/parser.hpp"
#include <napi.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

void set_optional_color(Napi::Object& obj, Napi::Env& env, const char* key,
                        const std::optional<std::array<int, 3>>& color) {
    if (color.has_value()) {
        Napi::Array arr = Napi::Array::New(env, 3);
        arr.Set((uint32_t)0, color->at(0));
        arr.Set((uint32_t)1, color->at(1));
        arr.Set((uint32_t)2, color->at(2));
        obj.Set(key, arr);
    } else {
        obj.Set(key, env.Null());
    }
}

Napi::Object general_to_js(Napi::Env& env, const general_section& s) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("AudioFilename", s.audio_filename);
    obj.Set("AudioLeadIn", s.audio_lead_in);
    obj.Set("AudioHash", s.audio_hash);
    obj.Set("PreviewTime", s.preview_time);
    obj.Set("Countdown", s.countdown);
    obj.Set("SampleSet", s.sample_set);
    obj.Set("StackLeniency", s.stack_leniency);
    obj.Set("Mode", s.mode);
    obj.Set("LetterboxInBreaks", s.letterbox_in_breaks);
    obj.Set("StoryFireInFront", s.story_fire_in_front);
    obj.Set("UseSkinSprites", s.use_skin_sprites);
    obj.Set("AlwaysShowPlayfield", s.always_show_playfield);
    obj.Set("OverlayPosition", s.overlay_position);
    obj.Set("SkinPreference", s.skin_preference);
    obj.Set("EpilepsyWarning", s.epilepsy_warning);
    obj.Set("CountdownOffset", s.countdown_offset);
    obj.Set("SpecialStyle", s.special_style);
    obj.Set("WidescreenStoryboard", s.widescreen_storyboard);
    obj.Set("SamplesMatchPlaybackRate", s.samples_match_playback_rate);
    return obj;
}

Napi::Object editor_to_js(Napi::Env& env, const editor_section& s) {
    Napi::Object obj = Napi::Object::New(env);
    Napi::Array bookmarks = Napi::Array::New(env, s.bookmarks.size());
    for (size_t i = 0; i < s.bookmarks.size(); i++) {
        bookmarks.Set(static_cast<uint32_t>(i), s.bookmarks[i]);
    }
    obj.Set("Bookmarks", bookmarks);
    obj.Set("DistanceSpacing", s.distance_spacing);
    obj.Set("BeatDivisor", s.beat_divisor);
    obj.Set("GridSize", s.grid_size);
    obj.Set("TimelineZoom", s.timeline_zoom);
    return obj;
}

Napi::Object metadata_to_js(Napi::Env& env, const metadata_section& s) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("Title", s.title);
    obj.Set("TitleUnicode", s.title_unicode);
    obj.Set("Artist", s.artist);
    obj.Set("ArtistUnicode", s.artist_unicode);
    obj.Set("Creator", s.creator);
    obj.Set("Version", s.version);
    obj.Set("Source", s.source);
    obj.Set("Tags", s.tags);
    obj.Set("BeatmapID", s.beatmap_id);
    obj.Set("BeatmapSetID", s.beatmap_set_id);
    return obj;
}

Napi::Object difficulty_to_js(Napi::Env& env, const difficulty_section& s) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("HPDrainRate", s.hp_drain_rate);
    obj.Set("CircleSize", s.circle_size);
    obj.Set("OverallDifficulty", s.overall_difficulty);
    obj.Set("ApproachRate", s.approach_rate);
    obj.Set("SliderMultiplier", s.slider_multiplier);
    obj.Set("SliderTickRate", s.slider_tick_rate);
    return obj;
}

Napi::Object background_to_js(Napi::Env& env, const event_background& b) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("filename", b.filename);
    obj.Set("xOffset", b.x_offset);
    obj.Set("yOffset", b.y_offset);
    return obj;
}

Napi::Object video_to_js(Napi::Env& env, const event_video& v) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("filename", v.filename);
    obj.Set("startTime", v.start_time);
    obj.Set("xOffset", v.x_offset);
    obj.Set("yOffset", v.y_offset);
    return obj;
}

Napi::Object break_to_js(Napi::Env& env, const event_break& b) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("startTime", b.start_time);
    obj.Set("endTime", b.end_time);
    return obj;
}

Napi::Object timing_point_to_js(Napi::Env& env, const timing_point& tp) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("time", tp.time);
    obj.Set("beatLength", tp.beat_length);
    obj.Set("meter", tp.meter);
    obj.Set("sampleSet", tp.sample_set);
    obj.Set("sampleIndex", tp.sample_index);
    obj.Set("volume", tp.volume);
    obj.Set("uninherited", tp.uninherited);
    obj.Set("effects", tp.effects);
    return obj;
}

Napi::Object colours_to_js(Napi::Env& env, const colour_section& c) {
    Napi::Object obj = Napi::Object::New(env);
    Napi::Array combos = Napi::Array::New(env, c.combos.size());
    for (size_t i = 0; i < c.combos.size(); i++) {
        Napi::Array color = Napi::Array::New(env, 3);
        color.Set((uint32_t)0, c.combos[i][0]);
        color.Set((uint32_t)1, c.combos[i][1]);
        color.Set((uint32_t)2, c.combos[i][2]);
        combos.Set(static_cast<uint32_t>(i), color);
    }
    obj.Set("Combos", combos);
    set_optional_color(obj, env, "SliderTrackOverride", c.slider_track_override);
    set_optional_color(obj, env, "SliderBorder", c.slider_border);
    return obj;
}

Napi::Object hit_sample_to_js(Napi::Env& env, const hit_sample& hs) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("normalSet", hs.normal_set);
    obj.Set("additionSet", hs.addition_set);
    obj.Set("index", hs.index);
    obj.Set("volume", hs.volume);
    obj.Set("filename", hs.filename);
    return obj;
}

Napi::Object hit_object_to_js(Napi::Env& env, const hit_object& ho) {
    Napi::Object obj = Napi::Object::New(env);

    obj.Set("x", ho.x);
    obj.Set("y", ho.y);
    obj.Set("time", ho.time);
    obj.Set("type", ho.type);
    obj.Set("hitSound", ho.hit_sound);
    obj.Set("hitSample", hit_sample_to_js(env, ho.sample));
    obj.Set("curveType", std::string(1, ho.curve_type));

    Napi::Array curve_points = Napi::Array::New(env, ho.curve_points.size());

    for (size_t i = 0; i < ho.curve_points.size(); i++) {
        Napi::Object point = Napi::Object::New(env);
        point.Set("x", ho.curve_points[i].first);
        point.Set("y", ho.curve_points[i].second);
        curve_points.Set(static_cast<uint32_t>(i), point);
    }

    obj.Set("curvePoints", curve_points);
    obj.Set("slides", ho.slides);
    obj.Set("length", ho.length);

    Napi::Array edge_sounds = Napi::Array::New(env, ho.edge_sounds.size());
    for (size_t i = 0; i < ho.edge_sounds.size(); i++) {
        edge_sounds.Set(static_cast<uint32_t>(i), ho.edge_sounds[i]);
    }

    obj.Set("edgeSounds", edge_sounds);
    Napi::Array edge_sets = Napi::Array::New(env, ho.edge_sets.size());

    for (size_t i = 0; i < ho.edge_sets.size(); i++) {
        Napi::Object set = Napi::Object::New(env);
        set.Set("normalSet", ho.edge_sets[i].first);
        set.Set("additionSet", ho.edge_sets[i].second);
        edge_sets.Set(static_cast<uint32_t>(i), set);
    }

    obj.Set("edgeSets", edge_sets);
    obj.Set("endTime", ho.end_time);

    return obj;
}

Napi::Object file_to_js(Napi::Env& env, const osu_file_format& file) {
    Napi::Object result = Napi::Object::New(env);

    result.Set("version", file.version);
    result.Set("General", general_to_js(env, file.general));
    result.Set("Editor", editor_to_js(env, file.editor));
    result.Set("Metadata", metadata_to_js(env, file.metadata));
    result.Set("Difficulty", difficulty_to_js(env, file.difficulty));

    Napi::Object events = Napi::Object::New(env);

    events.Set("background",
               file.background.has_value() ? background_to_js(env, file.background.value()) : Napi::Value(env.Null()));
    events.Set("video", file.video.has_value() ? video_to_js(env, file.video.value()) : Napi::Value(env.Null()));

    Napi::Array breaks = Napi::Array::New(env, file.breaks.size());

    for (size_t i = 0; i < file.breaks.size(); i++) {
        breaks.Set(static_cast<uint32_t>(i), break_to_js(env, file.breaks[i]));
    }

    events.Set("breaks", breaks);
    result.Set("Events", events);

    Napi::Array timing_points = Napi::Array::New(env, file.timing_points.size());

    for (size_t i = 0; i < file.timing_points.size(); i++) {
        timing_points.Set(static_cast<uint32_t>(i), timing_point_to_js(env, file.timing_points[i]));
    }

    result.Set("TimingPoints", timing_points);
    result.Set("Colours", colours_to_js(env, file.colours));

    Napi::Array hit_objects = Napi::Array::New(env, file.hit_objects.size());
    for (size_t i = 0; i < file.hit_objects.size(); i++) {
        hit_objects.Set(static_cast<uint32_t>(i), hit_object_to_js(env, file.hit_objects[i]));
    }

    result.Set("HitObjects", hit_objects);
    return result;
}

Napi::Promise resolve_promise(Napi::Env env, const Napi::Value& value) {
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    deferred.Resolve(value);
    return deferred.Promise();
}

class LambdaAsyncWorker : public Napi::AsyncWorker {
public:
    using execute_fn = std::function<void()>;
    using resolve_fn = std::function<Napi::Value(Napi::Env)>;

    LambdaAsyncWorker(Napi::Env env, std::string task_name, execute_fn exec, resolve_fn resolve)
        : Napi::AsyncWorker(env), deferred(Napi::Promise::Deferred::New(env)), task_name(std::move(task_name)),
          exec(std::move(exec)), resolve(std::move(resolve)) {}

    void Execute() override {
        try {
            exec();
        } catch (const std::exception& e) {
            SetError(e.what());
        }
    }

    void OnOK() override { deferred.Resolve(resolve(Env())); }

    void OnError(const Napi::Error& e) override {
        LOG_LINE("[addon]", task_name.c_str(), "error:", e.Message());
        deferred.Reject(e.Value());
    }

    Napi::Promise GetPromise() { return deferred.Promise(); }

private:
    Napi::Promise::Deferred deferred;
    std::string task_name;
    execute_fn exec;
    resolve_fn resolve;
};

Napi::Promise queue_worker(LambdaAsyncWorker* worker) {
    Napi::Promise promise = worker->GetPromise();
    worker->Queue();
    return promise;
}

Napi::Value get_property(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsBuffer() || !info[1].IsString()) {
        return resolve_promise(env, Napi::String::New(env, ""));
    }

    auto buffer = info[0].As<Napi::Buffer<char>>();
    std::string content(buffer.Data(), buffer.Length());
    std::string key = info[1].As<Napi::String>().Utf8Value();
    auto result = std::make_shared<std::string>();

    return queue_worker(new LambdaAsyncWorker(
        env, "get_property", [content = std::move(content), key = std::move(key), result]() {
            *result = osu_parser::get_property(content, key);
        }, [result](Napi::Env env) { return Napi::String::New(env, *result); }));
}

Napi::Value get_properties(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsBuffer() || !info[1].IsArray()) {
        return resolve_promise(env, Napi::Object::New(env));
    }

    auto buffer = info[0].As<Napi::Buffer<char>>();
    std::string content(buffer.Data(), buffer.Length());

    Napi::Array keys_array = info[1].As<Napi::Array>();
    std::vector<std::string> keys;
    keys.reserve(keys_array.Length());

    for (uint32_t i = 0; i < keys_array.Length(); i++) {
        Napi::Value val = keys_array[i];
        if (val.IsString()) {
            keys.push_back(val.As<Napi::String>().Utf8Value());
        }
    }

    auto result = std::make_shared<std::unordered_map<std::string, std::string>>();

    return queue_worker(new LambdaAsyncWorker(
        env, "get_properties", [content = std::move(content), keys = std::move(keys), result]() {
            *result = osu_parser::get_properties(content, keys);
        }, [result](Napi::Env env) {
            Napi::Object obj = Napi::Object::New(env);
            for (const auto& [k, v] : *result) {
                obj.Set(k, v);
            }
            return obj;
        }));
}

Napi::Value get_section(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsBuffer() || !info[1].IsString()) {
        return resolve_promise(env, Napi::Array::New(env));
    }

    auto buffer = info[0].As<Napi::Buffer<char>>();
    std::string content(buffer.Data(), buffer.Length());
    std::string section = info[1].As<Napi::String>().Utf8Value();
    auto result = std::make_shared<std::vector<std::string>>();

    return queue_worker(new LambdaAsyncWorker(
        env, "get_section", [content = std::move(content), section = std::move(section), result]() {
            *result = osu_parser::get_section(content, section);
        }, [result](Napi::Env env) {
            Napi::Array arr = Napi::Array::New(env, result->size());
            for (size_t i = 0; i < result->size(); i++) {
                arr.Set(static_cast<uint32_t>(i), (*result)[i]);
            }
            return arr;
        }));
}

Napi::Value parse(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsBuffer()) {
        return resolve_promise(env, Napi::Object::New(env));
    }

    auto buffer = info[0].As<Napi::Buffer<char>>();
    std::string content(buffer.Data(), buffer.Length());
    auto result = std::make_shared<osu_file_format>();

    return queue_worker(new LambdaAsyncWorker(
        env, "parse", [content = std::move(content), result]() { *result = osu_parser::parse(content); },
        [result](Napi::Env env) { return file_to_js(env, *result); }));
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("get_property", Napi::Function::New(env, get_property));
    exports.Set("get_properties", Napi::Function::New(env, get_properties));
    exports.Set("get_section", Napi::Function::New(env, get_section));
    exports.Set("parse", Napi::Function::New(env, parse));
    return exports;
}

NODE_API_MODULE(osu_beatmap_parser, Init)
