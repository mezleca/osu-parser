#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// === enums ===
enum OSU_SECTIONS { General = 0, Editor, Metadata, Difficulty, Events, TimingPoints, Colours, HitObjects };

// === sections ===
struct general_section {
    std::string audio_filename;
    int32_t audio_lead_in = 0;
    std::string audio_hash; // deprecated
    int32_t preview_time = -1;
    int32_t countdown = 1; // 0=none, 1=normal, 2=half, 3=double
    std::string sample_set = "Normal";
    double stack_leniency = 0.7;
    int32_t mode = 0; // 0=std, 1=taiko, 2=ctb, 3=mania
    int32_t letterbox_in_breaks = 0;
    int32_t story_fire_in_front = 1; // deprecated
    int32_t use_skin_sprites = 0;
    int32_t always_show_playfield = 0; // deprecated
    std::string overlay_position = "NoChange";
    std::string skin_preference;
    int32_t epilepsy_warning = 0;
    int32_t countdown_offset = 0;
    int32_t special_style = 0;
    int32_t widescreen_storyboard = 0;
    int32_t samples_match_playback_rate = 0;
};

struct editor_section {
    std::vector<int> bookmarks;
    double distance_spacing = 1.0;
    int32_t beat_divisor = 4;
    int32_t grid_size = 4;
    double timeline_zoom = 1.0;
};

struct metadata_section {
    std::string title;
    std::string title_unicode;
    std::string artist;
    std::string artist_unicode;
    std::string creator;
    std::string version;
    std::string source;
    std::string tags;
    int32_t beatmap_id = -1;
    int32_t beatmap_set_id = -1;
};

struct difficulty_section {
    double hp_drain_rate = 5.0;
    double circle_size = 5.0;
    double overall_difficulty = 5.0;
    double approach_rate = 5.0;
    double slider_multiplier = 1.4;
    double slider_tick_rate = 1.0;
};

struct event_background {
    std::string filename;
    int32_t x_offset = 0;
    int32_t y_offset = 0;
};

struct event_video {
    std::string filename;
    int32_t start_time = 0;
    int32_t x_offset = 0;
    int32_t y_offset = 0;
};

struct event_break {
    int32_t start_time = 0;
    int32_t end_time = 0;
};

struct timing_point {
    int32_t time = 0;
    double beat_length = 0.0;
    int32_t meter = 4;
    int32_t sample_set = 0; // 0=default, 1=normal, 2=soft, 3=drum
    int32_t sample_index = 0;
    int32_t volume = 100;
    int32_t uninherited = 1; // 1=red line (BPM), 0=green line (SV)
    int32_t effects = 0;     // bit 0=kiai, bit 3=omit first barline
};

struct colour_section {
    std::vector<std::array<int, 3>> combos;
    std::optional<std::array<int, 3>> slider_track_override;
    std::optional<std::array<int, 3>> slider_border;
};

struct hit_sample {
    int32_t normal_set = 0;
    int32_t addition_set = 0;
    int32_t index = 0;
    int32_t volume = 0;
    std::string filename;
};

struct hit_object {
    int32_t x = 0; // 0-512 (playfield coords)
    int32_t y = 0; // 0-384
    int32_t time = 0;
    int32_t type = 0;
    int32_t hit_sound = 0;
    hit_sample sample;

    char curve_type = 'L'; // B=bezier, C=catmull, L=linear, P=perfect circle
    std::vector<std::pair<int, int>> curve_points;
    int32_t slides = 1;
    double length = 0.0;
    std::vector<int> edge_sounds;
    std::vector<std::pair<int, int>> edge_sets;

    int32_t end_time = 0;
};

// === data ===
struct osu_beatmap {
    int32_t version = 14;
    general_section general;
    editor_section editor;
    metadata_section metadata;
    difficulty_section difficulty;
    std::optional<event_background> background;
    std::optional<event_video> video;
    std::vector<event_break> breaks;
    std::vector<timing_point> timing_points;
    colour_section colours;
    std::vector<hit_object> hit_objects;
};

// === helpers ===
inline const std::unordered_map<std::string, std::string>& key_to_section() {
    static const std::unordered_map<std::string, std::string> map = {
        {"AudioFilename", "[General]"},
        {"AudioLeadIn", "[General]"},
        {"AudioHash", "[General]"},
        {"PreviewTime", "[General]"},
        {"Countdown", "[General]"},
        {"SampleSet", "[General]"},
        {"StackLeniency", "[General]"},
        {"Mode", "[General]"},
        {"LetterboxInBreaks", "[General]"},
        {"StoryFireInFront", "[General]"},
        {"UseSkinSprites", "[General]"},
        {"AlwaysShowPlayfield", "[General]"},
        {"OverlayPosition", "[General]"},
        {"SkinPreference", "[General]"},
        {"EpilepsyWarning", "[General]"},
        {"CountdownOffset", "[General]"},
        {"SpecialStyle", "[General]"},
        {"WidescreenStoryboard", "[General]"},
        {"SamplesMatchPlaybackRate", "[General]"},
        {"Bookmarks", "[Editor]"},
        {"DistanceSpacing", "[Editor]"},
        {"BeatDivisor", "[Editor]"},
        {"GridSize", "[Editor]"},
        {"TimelineZoom", "[Editor]"},
        {"Title", "[Metadata]"},
        {"TitleUnicode", "[Metadata]"},
        {"Artist", "[Metadata]"},
        {"ArtistUnicode", "[Metadata]"},
        {"Creator", "[Metadata]"},
        {"Version", "[Metadata]"},
        {"Source", "[Metadata]"},
        {"Tags", "[Metadata]"},
        {"BeatmapID", "[Metadata]"},
        {"BeatmapSetID", "[Metadata]"},
        {"HPDrainRate", "[Difficulty]"},
        {"CircleSize", "[Difficulty]"},
        {"OverallDifficulty", "[Difficulty]"},
        {"ApproachRate", "[Difficulty]"},
        {"SliderMultiplier", "[Difficulty]"},
        {"SliderTickRate", "[Difficulty]"},
        {"Background", "[Events]"},
        {"Video", "[Events]"},
        {"Storyboard", "[Events]"},
    };

    return map;
}

inline const std::unordered_set<std::string>& get_special_keys() {
    static const std::unordered_set<std::string> set{"Background", "Video", "Storyboard"};
    return set;
}

// === parser ===
struct beatmap_parser {
    osu_beatmap* data;
    std::string location;
    std::vector<uint8_t> buffer;

    bool parse(std::string location);
    bool write();
};
