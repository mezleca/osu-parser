#include "definitions.hpp"
#include "osu/parser.hpp"
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <string_view>
#include <vector>

using namespace emscripten;

val optional_color_to_val(const std::optional<std::array<int, 3>>& color) {
    if (!color.has_value()) {
        return val::null();
    }
    val arr = val::array();
    arr.set(0, color->at(0));
    arr.set(1, color->at(1));
    arr.set(2, color->at(2));
    return arr;
}

val general_to_val(const general_section& s) {
    val obj = val::object();
    obj.set("AudioFilename", s.audio_filename);
    obj.set("AudioLeadIn", s.audio_lead_in);
    obj.set("AudioHash", s.audio_hash);
    obj.set("PreviewTime", s.preview_time);
    obj.set("Countdown", s.countdown);
    obj.set("SampleSet", s.sample_set);
    obj.set("StackLeniency", s.stack_leniency);
    obj.set("Mode", s.mode);
    obj.set("LetterboxInBreaks", s.letterbox_in_breaks);
    obj.set("StoryFireInFront", s.story_fire_in_front);
    obj.set("UseSkinSprites", s.use_skin_sprites);
    obj.set("AlwaysShowPlayfield", s.always_show_playfield);
    obj.set("OverlayPosition", s.overlay_position);
    obj.set("SkinPreference", s.skin_preference);
    obj.set("EpilepsyWarning", s.epilepsy_warning);
    obj.set("CountdownOffset", s.countdown_offset);
    obj.set("SpecialStyle", s.special_style);
    obj.set("WidescreenStoryboard", s.widescreen_storyboard);
    obj.set("SamplesMatchPlaybackRate", s.samples_match_playback_rate);
    return obj;
}

val editor_to_val(const editor_section& s) {
    val obj = val::object();
    val bookmarks = val::array();

    for (size_t i = 0; i < s.bookmarks.size(); i++) {
        bookmarks.set(i, s.bookmarks[i]);
    }

    obj.set("Bookmarks", bookmarks);
    obj.set("DistanceSpacing", s.distance_spacing);
    obj.set("BeatDivisor", s.beat_divisor);
    obj.set("GridSize", s.grid_size);
    obj.set("TimelineZoom", s.timeline_zoom);
    return obj;
}

val metadata_to_val(const metadata_section& s) {
    val obj = val::object();
    obj.set("Title", s.title);
    obj.set("TitleUnicode", s.title_unicode);
    obj.set("Artist", s.artist);
    obj.set("ArtistUnicode", s.artist_unicode);
    obj.set("Creator", s.creator);
    obj.set("Version", s.version);
    obj.set("Source", s.source);
    obj.set("Tags", s.tags);
    obj.set("BeatmapID", s.beatmap_id);
    obj.set("BeatmapSetID", s.beatmap_set_id);
    return obj;
}

val difficulty_to_val(const difficulty_section& s) {
    val obj = val::object();
    obj.set("HPDrainRate", s.hp_drain_rate);
    obj.set("CircleSize", s.circle_size);
    obj.set("OverallDifficulty", s.overall_difficulty);
    obj.set("ApproachRate", s.approach_rate);
    obj.set("SliderMultiplier", s.slider_multiplier);
    obj.set("SliderTickRate", s.slider_tick_rate);
    return obj;
}

val timing_point_to_val(const timing_point& tp) {
    val obj = val::object();
    obj.set("time", tp.time);
    obj.set("beatLength", tp.beat_length);
    obj.set("meter", tp.meter);
    obj.set("sampleSet", tp.sample_set);
    obj.set("sampleIndex", tp.sample_index);
    obj.set("volume", tp.volume);
    obj.set("uninherited", tp.uninherited);
    obj.set("effects", tp.effects);
    return obj;
}

val colours_to_val(const colour_section& c) {
    val obj = val::object();
    val combos = val::array();

    for (size_t i = 0; i < c.combos.size(); i++) {
        val color = val::array();
        color.set(0, c.combos[i][0]);
        color.set(1, c.combos[i][1]);
        color.set(2, c.combos[i][2]);
        combos.set(i, color);
    }

    obj.set("Combos", combos);
    obj.set("SliderTrackOverride", optional_color_to_val(c.slider_track_override));
    obj.set("SliderBorder", optional_color_to_val(c.slider_border));

    return obj;
}

val hit_sample_to_val(const hit_sample& hs) {
    val obj = val::object();
    obj.set("normalSet", hs.normal_set);
    obj.set("additionSet", hs.addition_set);
    obj.set("index", hs.index);
    obj.set("volume", hs.volume);
    obj.set("filename", hs.filename);
    return obj;
}

val hit_object_to_val(const hit_object& ho) {
    val obj = val::object();
    obj.set("x", ho.x);
    obj.set("y", ho.y);
    obj.set("time", ho.time);
    obj.set("type", ho.type);
    obj.set("hitSound", ho.hit_sound);
    obj.set("hitSample", hit_sample_to_val(ho.sample));
    obj.set("curveType", std::string(1, ho.curve_type));

    val curve_points = val::array();

    for (size_t i = 0; i < ho.curve_points.size(); i++) {
        val point = val::object();
        point.set("x", ho.curve_points[i].first);
        point.set("y", ho.curve_points[i].second);
        curve_points.set(i, point);
    }

    obj.set("curvePoints", curve_points);
    obj.set("slides", ho.slides);
    obj.set("length", ho.length);

    val edge_sounds = val::array();

    for (size_t i = 0; i < ho.edge_sounds.size(); i++) {
        edge_sounds.set(i, ho.edge_sounds[i]);
    }

    obj.set("edgeSounds", edge_sounds);
    val edge_sets = val::array();

    for (size_t i = 0; i < ho.edge_sets.size(); i++) {
        val set = val::object();
        set.set("normalSet", ho.edge_sets[i].first);
        set.set("additionSet", ho.edge_sets[i].second);
        edge_sets.set(i, set);
    }

    obj.set("edgeSets", edge_sets);
    obj.set("endTime", ho.end_time);

    return obj;
}

val file_to_val(const osu_file_format& file) {
    val result = val::object();

    result.set("version", file.version);
    result.set("General", general_to_val(file.general));
    result.set("Editor", editor_to_val(file.editor));
    result.set("Metadata", metadata_to_val(file.metadata));
    result.set("Difficulty", difficulty_to_val(file.difficulty));

    val events = val::object();

    if (file.background.has_value()) {
        val bg = val::object();
        bg.set("filename", file.background->filename);
        bg.set("xOffset", file.background->x_offset);
        bg.set("yOffset", file.background->y_offset);
        events.set("background", bg);
    } else {
        events.set("background", val::null());
    }

    if (file.video.has_value()) {
        val vid = val::object();
        vid.set("filename", file.video->filename);
        vid.set("startTime", file.video->start_time);
        vid.set("xOffset", file.video->x_offset);
        vid.set("yOffset", file.video->y_offset);
        events.set("video", vid);
    } else {
        events.set("video", val::null());
    }

    val breaks = val::array();
    for (size_t i = 0; i < file.breaks.size(); i++) {
        val b = val::object();
        b.set("startTime", file.breaks[i].start_time);
        b.set("endTime", file.breaks[i].end_time);
        breaks.set(i, b);
    }
    events.set("breaks", breaks);
    result.set("Events", events);

    val timing_points = val::array();

    for (size_t i = 0; i < file.timing_points.size(); i++) {
        timing_points.set(i, timing_point_to_val(file.timing_points[i]));
    }

    result.set("TimingPoints", timing_points);
    result.set("Colours", colours_to_val(file.colours));

    val hit_objects = val::array();

    for (size_t i = 0; i < file.hit_objects.size(); i++) {
        hit_objects.set(i, hit_object_to_val(file.hit_objects[i]));
    }

    result.set("HitObjects", hit_objects);
    return result;
}

std::string get_property_wasm(uintptr_t data_ptr, int length, std::string key) {
    try {
        if (length <= 0) {
            return "";
        }
        std::string_view content(reinterpret_cast<const char*>(data_ptr), static_cast<size_t>(length));
        return osu_parser::get_property(content, key);
    } catch (...) {
        return "";
    }
}

val get_properties_wasm(uintptr_t data_ptr, int length, val keys) {
    try {
        if (length <= 0) {
            return val::object();
        }
        std::string_view content(reinterpret_cast<const char*>(data_ptr), static_cast<size_t>(length));
        std::vector<std::string> key_list = vecFromJSArray<std::string>(keys);
        auto results = osu_parser::get_properties(content, key_list);

        val obj = val::object();
        for (const auto& key : key_list) {
            obj.set(key, results[key]);
        }
        return obj;
    } catch (...) {
        return val::object();
    }
}

val get_section_wasm(uintptr_t data_ptr, int length, std::string section) {
    try {
        if (length <= 0) {
            return val::array();
        }

        std::string_view content(reinterpret_cast<const char*>(data_ptr), static_cast<size_t>(length));
        auto lines = osu_parser::get_section(content, section);

        val arr = val::array();
        for (size_t i = 0; i < lines.size(); i++) {
            arr.set(i, lines[i]);
        }
        return arr;
    } catch (...) {
        return val::array();
    }
}

val parse_wasm(uintptr_t data_ptr, int length) {
    try {
        if (length <= 0) {
            return val::object();
        }
        std::string_view content(reinterpret_cast<const char*>(data_ptr), static_cast<size_t>(length));
        osu_file_format file = osu_parser::parse(content);
        return file_to_val(file);
    } catch (...) {
        return val::object();
    }
}

EMSCRIPTEN_BINDINGS(osu_parser) {
    function("get_property", &get_property_wasm);
    function("get_properties", &get_properties_wasm);
    function("get_section", &get_section_wasm);
    function("parse", &parse_wasm);
}

int main() {
    return 0;
}
