#include "common.hpp"

#include "beatmap/beatmap.hpp"
#include "parser/parser_base.hpp"
#include "update_helpers.hpp"

#include <string>

namespace osu_bindings {
    using beatmap_instance = parser_base<osu_beatmap, beatmap_parser>;

#define OSU_GENERAL_FIELDS(X)                                                                                          \
    X(get_optional_string, "AudioFilename", audio_filename)                                                            \
    X(get_optional_int32, "AudioLeadIn", audio_lead_in)                                                                \
    X(get_optional_string, "AudioHash", audio_hash)                                                                    \
    X(get_optional_int32, "PreviewTime", preview_time)                                                                 \
    X(get_optional_int32, "Countdown", countdown)                                                                      \
    X(get_optional_string, "SampleSet", sample_set)                                                                    \
    X(get_optional_double, "StackLeniency", stack_leniency)                                                            \
    X(get_optional_int32, "Mode", mode)                                                                                \
    X(get_optional_int32, "LetterboxInBreaks", letterbox_in_breaks)                                                    \
    X(get_optional_int32, "StoryFireInFront", story_fire_in_front)                                                     \
    X(get_optional_int32, "UseSkinSprites", use_skin_sprites)                                                          \
    X(get_optional_int32, "AlwaysShowPlayfield", always_show_playfield)                                                \
    X(get_optional_string, "OverlayPosition", overlay_position)                                                        \
    X(get_optional_string, "SkinPreference", skin_preference)                                                          \
    X(get_optional_int32, "EpilepsyWarning", epilepsy_warning)                                                         \
    X(get_optional_int32, "CountdownOffset", countdown_offset)                                                         \
    X(get_optional_int32, "SpecialStyle", special_style)                                                               \
    X(get_optional_int32, "WidescreenStoryboard", widescreen_storyboard)                                               \
    X(get_optional_int32, "SamplesMatchPlaybackRate", samples_match_playback_rate)

#define OSU_EDITOR_FIELDS(X)                                                                                           \
    X(get_optional_double, "DistanceSpacing", distance_spacing)                                                        \
    X(get_optional_int32, "BeatDivisor", beat_divisor)                                                                 \
    X(get_optional_int32, "GridSize", grid_size)                                                                       \
    X(get_optional_double, "TimelineZoom", timeline_zoom)

#define OSU_METADATA_FIELDS(X)                                                                                         \
    X(get_optional_string, "Title", title)                                                                             \
    X(get_optional_string, "TitleUnicode", title_unicode)                                                              \
    X(get_optional_string, "Artist", artist)                                                                           \
    X(get_optional_string, "ArtistUnicode", artist_unicode)                                                            \
    X(get_optional_string, "Creator", creator)                                                                         \
    X(get_optional_string, "Version", version)                                                                         \
    X(get_optional_string, "Source", source)                                                                           \
    X(get_optional_string, "Tags", tags)                                                                               \
    X(get_optional_int32, "BeatmapID", beatmap_id)                                                                     \
    X(get_optional_int32, "BeatmapSetID", beatmap_set_id)

#define OSU_DIFFICULTY_FIELDS(X)                                                                                       \
    X(get_optional_double, "HPDrainRate", hp_drain_rate)                                                               \
    X(get_optional_double, "CircleSize", circle_size)                                                                  \
    X(get_optional_double, "OverallDifficulty", overall_difficulty)                                                    \
    X(get_optional_double, "ApproachRate", approach_rate)                                                              \
    X(get_optional_double, "SliderMultiplier", slider_multiplier)                                                      \
    X(get_optional_double, "SliderTickRate", slider_tick_rate)

    bool parse_version_value(const Napi::Value& value, int32_t& out, std::string& err) {
        if (value.IsNumber()) {
            out = value.As<Napi::Number>().Int32Value();
            return true;
        }

        if (!value.IsString()) {
            err = "invalid type for version";
            return false;
        }

        std::string text = value.As<Napi::String>();
        const std::string whitespace = " \t\n\r\f\v";
        const size_t start = text.find_first_not_of(whitespace);
        if (start == std::string::npos) {
            err = "invalid version format";
            return false;
        }
        const size_t end = text.find_last_not_of(whitespace);
        text = text.substr(start, end - start + 1);

        if (!text.empty() && (text[0] == 'v' || text[0] == 'V')) {
            text.erase(text.begin());
        }

        try {
            out = std::stoi(text);
        } catch (...) {
            err = "invalid version format";
            return false;
        }

        return true;
    }

    bool parse_int_array(const Napi::Value& value, std::vector<int>& out, std::string& err) {
        if (!value.IsArray()) {
            err = "expected array";
            return false;
        }

        Napi::Array arr = value.As<Napi::Array>();

        out.clear();
        out.reserve(arr.Length());

        for (uint32_t i = 0; i < arr.Length(); i++) {
            Napi::Value item = arr.Get(i);
            if (!item.IsNumber()) {
                err = "expected number in array";
                return false;
            }
            out.push_back(item.As<Napi::Number>().Int32Value());
        }

        return true;
    }

    bool parse_color(const Napi::Value& value, std::array<int, 3>& out, std::string& err) {
        if (!value.IsArray()) {
            err = "expected color array";
            return false;
        }

        Napi::Array arr = value.As<Napi::Array>();

        if (arr.Length() != 3) {
            err = "color array must have 3 items";
            return false;
        }

        for (uint32_t i = 0; i < 3; i++) {
            Napi::Value item = arr.Get(i);

            if (!item.IsNumber()) {
                err = "color values must be numbers";
                return false;
            }

            out[i] = item.As<Napi::Number>().Int32Value();
        }

        return true;
    }

    bool update_general_section(general_section& s, const Napi::Object& patch, std::string& err) {
#define APPLY_FIELD(fn, key, member)                                                                                   \
    if (!fn(patch, key, s.member, err)) {                                                                              \
        return false;                                                                                                  \
    }
        OSU_GENERAL_FIELDS(APPLY_FIELD)
#undef APPLY_FIELD
        return true;
    }

    bool update_editor_section(editor_section& s, const Napi::Object& patch, std::string& err) {
        if (patch.Has("Bookmarks")) {
            if (!parse_int_array(patch.Get("Bookmarks"), s.bookmarks, err)) {
                return false;
            }
        }
#define APPLY_FIELD(fn, key, member)                                                                                   \
    if (!fn(patch, key, s.member, err)) {                                                                              \
        return false;                                                                                                  \
    }
        OSU_EDITOR_FIELDS(APPLY_FIELD)
#undef APPLY_FIELD
        return true;
    }

    bool update_metadata_section(metadata_section& s, const Napi::Object& patch, std::string& err) {
#define APPLY_FIELD(fn, key, member)                                                                                   \
    if (!fn(patch, key, s.member, err)) {                                                                              \
        return false;                                                                                                  \
    }
        OSU_METADATA_FIELDS(APPLY_FIELD)
#undef APPLY_FIELD
        return true;
    }

    bool update_difficulty_section(difficulty_section& s, const Napi::Object& patch, std::string& err) {
#define APPLY_FIELD(fn, key, member)                                                                                   \
    if (!fn(patch, key, s.member, err)) {                                                                              \
        return false;                                                                                                  \
    }
        OSU_DIFFICULTY_FIELDS(APPLY_FIELD)
#undef APPLY_FIELD
        return true;
    }

    bool update_background(event_background& b, const Napi::Object& patch, std::string& err) {
        return get_optional_string(patch, "filename", b.filename, err) &&
               get_optional_int32(patch, "xOffset", b.x_offset, err) &&
               get_optional_int32(patch, "yOffset", b.y_offset, err);
    }

    bool update_video(event_video& v, const Napi::Object& patch, std::string& err) {
        return get_optional_string(patch, "filename", v.filename, err) &&
               get_optional_int32(patch, "startTime", v.start_time, err) &&
               get_optional_int32(patch, "xOffset", v.x_offset, err) &&
               get_optional_int32(patch, "yOffset", v.y_offset, err);
    }

    bool parse_event_break(const Napi::Value& value, event_break& out, std::string& err) {
        if (!is_object(value)) {
            err = "break must be an object";
            return false;
        }

        Napi::Object obj = value.As<Napi::Object>();

        return get_optional_int32(obj, "startTime", out.start_time, err) &&
               get_optional_int32(obj, "endTime", out.end_time, err);
    }

    bool parse_timing_point(const Napi::Value& value, timing_point& out, std::string& err) {
        if (!is_object(value)) {
            err = "timing point must be an object";
            return false;
        }

        Napi::Object obj = value.As<Napi::Object>();

        return get_optional_int32(obj, "time", out.time, err) &&
               get_optional_double(obj, "beatLength", out.beat_length, err) &&
               get_optional_int32(obj, "meter", out.meter, err) &&
               get_optional_int32(obj, "sampleSet", out.sample_set, err) &&
               get_optional_int32(obj, "sampleIndex", out.sample_index, err) &&
               get_optional_int32(obj, "volume", out.volume, err) &&
               get_optional_int32(obj, "uninherited", out.uninherited, err) &&
               get_optional_int32(obj, "effects", out.effects, err);
    }

    bool parse_hit_sample(const Napi::Value& value, hit_sample& out, std::string& err) {
        if (!is_object(value)) {
            err = "hitSample must be an object";
            return false;
        }

        Napi::Object obj = value.As<Napi::Object>();

        return get_optional_int32(obj, "normalSet", out.normal_set, err) &&
               get_optional_int32(obj, "additionSet", out.addition_set, err) &&
               get_optional_int32(obj, "index", out.index, err) && get_optional_int32(obj, "volume", out.volume, err) &&
               get_optional_string(obj, "filename", out.filename, err);
    }

    bool parse_curve_points(const Napi::Value& value, std::vector<std::pair<int, int>>& out, std::string& err) {
        if (!value.IsArray()) {
            err = "curvePoints must be an array";
            return false;
        }

        Napi::Array arr = value.As<Napi::Array>();

        out.clear();
        out.reserve(arr.Length());

        for (uint32_t i = 0; i < arr.Length(); i++) {
            Napi::Value item = arr.Get(i);

            if (!is_object(item)) {
                err = "curve point must be an object";
                return false;
            }

            Napi::Object obj = item.As<Napi::Object>();

            int32_t x = 0;
            int32_t y = 0;

            if (!get_optional_int32(obj, "x", x, err) || !get_optional_int32(obj, "y", y, err)) {
                return false;
            }

            out.emplace_back(x, y);
        }
        return true;
    }

    bool parse_edge_sets(const Napi::Value& value, std::vector<std::pair<int, int>>& out, std::string& err) {
        if (!value.IsArray()) {
            err = "edgeSets must be an array";
            return false;
        }

        Napi::Array arr = value.As<Napi::Array>();

        out.clear();
        out.reserve(arr.Length());

        for (uint32_t i = 0; i < arr.Length(); i++) {
            Napi::Value item = arr.Get(i);

            if (!is_object(item)) {
                err = "edge set must be an object";
                return false;
            }

            Napi::Object obj = item.As<Napi::Object>();

            int32_t normal_set = 0;
            int32_t addition_set = 0;

            if (!get_optional_int32(obj, "normalSet", normal_set, err) ||
                !get_optional_int32(obj, "additionSet", addition_set, err)) {
                return false;
            }

            out.emplace_back(normal_set, addition_set);
        }
        return true;
    }

    bool parse_edge_sounds(const Napi::Value& value, std::vector<int>& out, std::string& err) {
        return parse_int_array(value, out, err);
    }

    bool parse_hit_object(const Napi::Value& value, hit_object& out, std::string& err) {
        if (!is_object(value)) {
            err = "hit object must be an object";
            return false;
        }

        Napi::Object obj = value.As<Napi::Object>();

        if (!get_optional_int32(obj, "x", out.x, err) || !get_optional_int32(obj, "y", out.y, err) ||
            !get_optional_int32(obj, "time", out.time, err) || !get_optional_int32(obj, "type", out.type, err) ||
            !get_optional_int32(obj, "hitSound", out.hit_sound, err)) {
            return false;
        }

        if (obj.Has("hitSample")) {
            if (!parse_hit_sample(obj.Get("hitSample"), out.sample, err)) {
                return false;
            }
        }

        if (obj.Has("curveType")) {
            Napi::Value value_curve = obj.Get("curveType");
            if (!value_curve.IsString()) {
                err = "curveType must be a string";
                return false;
            }
            std::string curve = value_curve.As<Napi::String>();
            if (!curve.empty()) {
                out.curve_type = curve[0];
            }
        }

        if (obj.Has("curvePoints")) {
            if (!parse_curve_points(obj.Get("curvePoints"), out.curve_points, err)) {
                return false;
            }
        }

        if (!get_optional_int32(obj, "slides", out.slides, err) ||
            !get_optional_double(obj, "length", out.length, err)) {
            return false;
        }

        if (obj.Has("edgeSounds")) {
            if (!parse_edge_sounds(obj.Get("edgeSounds"), out.edge_sounds, err)) {
                return false;
            }
        }

        if (obj.Has("edgeSets")) {
            if (!parse_edge_sets(obj.Get("edgeSets"), out.edge_sets, err)) {
                return false;
            }
        }

        return get_optional_int32(obj, "endTime", out.end_time, err);
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

    void set_optional_color(Napi::Object& obj, Napi::Env& env, const char* key,
                            const std::optional<std::array<int, 3>>& color) {
        if (color.has_value()) {
            Napi::Array arr = Napi::Array::New(env, 3);
            arr.Set(static_cast<uint32_t>(0), color->at(0));
            arr.Set(static_cast<uint32_t>(1), color->at(1));
            arr.Set(static_cast<uint32_t>(2), color->at(2));
            obj.Set(key, arr);
        } else {
            obj.Set(key, env.Null());
        }
    }

    Napi::Object colours_to_js(Napi::Env& env, const colour_section& c) {
        Napi::Object obj = Napi::Object::New(env);
        Napi::Array combos = Napi::Array::New(env, c.combos.size());

        for (size_t i = 0; i < c.combos.size(); i++) {
            Napi::Array color = Napi::Array::New(env, 3);
            color.Set(static_cast<uint32_t>(0), c.combos[i][0]);
            color.Set(static_cast<uint32_t>(1), c.combos[i][1]);
            color.Set(static_cast<uint32_t>(2), c.combos[i][2]);
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

    Napi::Object beatmap_to_js(Napi::Env& env, const osu_beatmap& file) {
        Napi::Object result = Napi::Object::New(env);

        result.Set("version", Napi::String::New(env, "v" + std::to_string(file.version)));
        result.Set("General", general_to_js(env, file.general));
        result.Set("Editor", editor_to_js(env, file.editor));
        result.Set("Metadata", metadata_to_js(env, file.metadata));
        result.Set("Difficulty", difficulty_to_js(env, file.difficulty));

        Napi::Object events = Napi::Object::New(env);

        if (file.background.has_value()) {
            events.Set("background", background_to_js(env, *file.background));
        } else {
            events.Set("background", env.Null());
        }

        if (file.video.has_value()) {
            events.Set("video", video_to_js(env, *file.video));
        } else {
            events.Set("video", env.Null());
        }

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

    Napi::Value create_beatmap_parser(const Napi::CallbackInfo& info) {
        return create_instance<beatmap_instance>(info);
    }

    Napi::Value free_beatmap_parser(const Napi::CallbackInfo& info) {
        return free_instance<beatmap_instance>(info);
    }

    Napi::Value beatmap_parser_parse(const Napi::CallbackInfo& info) {
        return parse_instance_async<beatmap_instance>(info, "beatmap_parse");
    }

    Napi::Value beatmap_parser_write(const Napi::CallbackInfo& info) {
        return write_instance_async<beatmap_instance>(info, "beatmap_write");
    }

    Napi::Value beatmap_parser_last_error(const Napi::CallbackInfo& info) {
        return last_error_instance<beatmap_instance>(info);
    }

    Napi::Value beatmap_parser_get(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        beatmap_instance* instance = get_ptr<beatmap_instance>(info, 0);

        if (instance == nullptr) {
            return env.Null();
        }

        return instance->with_lock([&](osu_beatmap& data, beatmap_parser&) { return beatmap_to_js(env, data); });
    }

    Napi::Value beatmap_get_by_key(Napi::Env& env, const osu_beatmap& data, const std::string& key) {
        if (key == "version") {
            return Napi::String::New(env, "v" + std::to_string(data.version));
        }

        if (key == "General")
            return general_to_js(env, data.general);
        if (key == "Editor")
            return editor_to_js(env, data.editor);
        if (key == "Metadata")
            return metadata_to_js(env, data.metadata);
        if (key == "Difficulty")
            return difficulty_to_js(env, data.difficulty);
        if (key == "Events") {
            Napi::Object events = Napi::Object::New(env);
            events.Set("background",
                       data.background.has_value() ? background_to_js(env, *data.background) : env.Null());
            events.Set("video", data.video.has_value() ? video_to_js(env, *data.video) : env.Null());
            Napi::Array breaks = Napi::Array::New(env, data.breaks.size());
            for (size_t i = 0; i < data.breaks.size(); i++) {
                breaks.Set(static_cast<uint32_t>(i), break_to_js(env, data.breaks[i]));
            }
            events.Set("breaks", breaks);
            return events;
        }
        if (key == "TimingPoints") {
            Napi::Array arr = Napi::Array::New(env, data.timing_points.size());
            for (size_t i = 0; i < data.timing_points.size(); i++) {
                arr.Set(static_cast<uint32_t>(i), timing_point_to_js(env, data.timing_points[i]));
            }
            return arr;
        }
        if (key == "Colours")
            return colours_to_js(env, data.colours);
        if (key == "HitObjects") {
            Napi::Array arr = Napi::Array::New(env, data.hit_objects.size());
            for (size_t i = 0; i < data.hit_objects.size(); i++) {
                arr.Set(static_cast<uint32_t>(i), hit_object_to_js(env, data.hit_objects[i]));
            }
            return arr;
        }
        return env.Undefined();
    }

    Napi::Value beatmap_parser_update(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        beatmap_instance* instance = get_ptr<beatmap_instance>(info, 0);

        if (instance == nullptr) {
            Napi::Error::New(env, "invalid parser handle").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        if (info.Length() < 2 || !is_object(info[1])) {
            Napi::TypeError::New(env, "update patch must be an object").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        std::string err;
        bool ok = instance->with_lock([&](osu_beatmap& data, beatmap_parser& parser) {
            Napi::Object patch = info[1].As<Napi::Object>();
            osu_beatmap temp = data;

            if (patch.Has("version")) {
                if (!parse_version_value(patch.Get("version"), temp.version, err)) {
                    parser.last_error = err;
                    return false;
                }
            }

            if (patch.Has("General")) {
                Napi::Value value = patch.Get("General");
                if (!is_object(value)) {
                    parser.last_error = "General must be an object";
                    return false;
                }
                if (!update_general_section(temp.general, value.As<Napi::Object>(), err)) {
                    parser.last_error = err;
                    return false;
                }
            }

            if (patch.Has("Editor")) {
                Napi::Value value = patch.Get("Editor");
                if (!is_object(value)) {
                    parser.last_error = "Editor must be an object";
                    return false;
                }
                if (!update_editor_section(temp.editor, value.As<Napi::Object>(), err)) {
                    parser.last_error = err;
                    return false;
                }
            }

            if (patch.Has("Metadata")) {
                Napi::Value value = patch.Get("Metadata");
                if (!is_object(value)) {
                    parser.last_error = "Metadata must be an object";
                    return false;
                }
                if (!update_metadata_section(temp.metadata, value.As<Napi::Object>(), err)) {
                    parser.last_error = err;
                    return false;
                }
            }

            if (patch.Has("Difficulty")) {
                Napi::Value value = patch.Get("Difficulty");
                if (!is_object(value)) {
                    parser.last_error = "Difficulty must be an object";
                    return false;
                }
                if (!update_difficulty_section(temp.difficulty, value.As<Napi::Object>(), err)) {
                    parser.last_error = err;
                    return false;
                }
            }

            if (patch.Has("Events")) {
                Napi::Value value = patch.Get("Events");
                if (!is_object(value)) {
                    parser.last_error = "Events must be an object";
                    return false;
                }
                Napi::Object events = value.As<Napi::Object>();

                if (events.Has("background")) {
                    Napi::Value bg_value = events.Get("background");
                    if (bg_value.IsNull()) {
                        temp.background.reset();
                    } else if (is_object(bg_value)) {
                        if (!temp.background.has_value()) {
                            temp.background = event_background{};
                        }
                        if (!update_background(*temp.background, bg_value.As<Napi::Object>(), err)) {
                            parser.last_error = err;
                            return false;
                        }
                    } else {
                        parser.last_error = "Events.background must be an object or null";
                        return false;
                    }
                }

                if (events.Has("video")) {
                    Napi::Value video_value = events.Get("video");
                    if (video_value.IsNull()) {
                        temp.video.reset();
                    } else if (is_object(video_value)) {
                        if (!temp.video.has_value()) {
                            temp.video = event_video{};
                        }
                        if (!update_video(*temp.video, video_value.As<Napi::Object>(), err)) {
                            parser.last_error = err;
                            return false;
                        }
                    } else {
                        parser.last_error = "Events.video must be an object or null";
                        return false;
                    }
                }

                if (!update_array_field(events, "breaks", temp.breaks, parse_event_break, err)) {
                    parser.last_error = err;
                    return false;
                }
            }

            if (!update_array_field(patch, "TimingPoints", temp.timing_points, parse_timing_point, err)) {
                parser.last_error = err;
                return false;
            }

            if (patch.Has("Colours")) {
                Napi::Value value = patch.Get("Colours");
                if (!is_object(value)) {
                    parser.last_error = "Colours must be an object";
                    return false;
                }
                Napi::Object colours = value.As<Napi::Object>();

                if (!update_array_field(colours, "Combos", temp.colours.combos, parse_color, err)) {
                    parser.last_error = err;
                    return false;
                }

                if (colours.Has("SliderTrackOverride")) {
                    Napi::Value slider_track = colours.Get("SliderTrackOverride");
                    if (slider_track.IsNull()) {
                        temp.colours.slider_track_override.reset();
                    } else {
                        std::array<int, 3> color{};
                        if (!parse_color(slider_track, color, err)) {
                            parser.last_error = err;
                            return false;
                        }
                        temp.colours.slider_track_override = color;
                    }
                }

                if (colours.Has("SliderBorder")) {
                    Napi::Value slider_border = colours.Get("SliderBorder");
                    if (slider_border.IsNull()) {
                        temp.colours.slider_border.reset();
                    } else {
                        std::array<int, 3> color{};
                        if (!parse_color(slider_border, color, err)) {
                            parser.last_error = err;
                            return false;
                        }
                        temp.colours.slider_border = color;
                    }
                }
            }

            if (!update_array_field(patch, "HitObjects", temp.hit_objects, parse_hit_object, err)) {
                parser.last_error = err;
                return false;
            }

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

    Napi::Value beatmap_parser_get_by_name(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        beatmap_instance* instance = get_ptr<beatmap_instance>(info, 0);

        if (instance == nullptr || info.Length() < 2 || !info[1].IsString()) {
            return env.Undefined();
        }

        std::string key = info[1].As<Napi::String>();
        return instance->with_lock(
            [&](osu_beatmap& data, beatmap_parser&) { return beatmap_get_by_key(env, data, key); });
    }

    void register_beatmap(Napi::Env env, Napi::Object exports) {
        exports.Set("create_beatmap_parser", Napi::Function::New(env, create_beatmap_parser));
        exports.Set("free_beatmap_parser", Napi::Function::New(env, free_beatmap_parser));
        exports.Set("beatmap_parser_parse", Napi::Function::New(env, beatmap_parser_parse));
        exports.Set("beatmap_parser_write", Napi::Function::New(env, beatmap_parser_write));
        exports.Set("beatmap_parser_last_error", Napi::Function::New(env, beatmap_parser_last_error));
        exports.Set("beatmap_parser_get", Napi::Function::New(env, beatmap_parser_get));
        exports.Set("beatmap_parser_update", Napi::Function::New(env, beatmap_parser_update));
        exports.Set("beatmap_parser_get_by_name", Napi::Function::New(env, beatmap_parser_get_by_name));
    }
}
