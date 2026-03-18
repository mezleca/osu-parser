#include "beatmap.hpp"

#include <algorithm>
#include <charconv>
#include <functional>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unordered_set>

static void debug_print(const std::string& message) {
    std::cerr << "[beatmap] " << message << "\n";
}

static std::string debug_preview(std::string_view content, size_t length) {
    size_t size = std::min(content.size(), length);
    std::string preview(content.substr(0, size));
    for (char& c : preview) {
        if (c == '\r') {
            c = 'R';
        } else if (c == '\n') {
            c = 'N';
        } else if (static_cast<unsigned char>(c) < 32) {
            c = '.';
        }
    }
    return preview;
}

static const std::unordered_set<std::string>& get_video_extensions() {
    static const std::unordered_set<std::string> extensions = {".mp4", ".avi", ".flv", ".mov",
                                                               ".wmv", ".m4v", ".mpg", ".mpeg"};
    return extensions;
}

static const std::unordered_set<std::string>& get_image_extensions() {
    static const std::unordered_set<std::string> extensions = {".jpg", ".jpeg", ".png", ".bmp", ".gif"};
    return extensions;
}

static std::string_view trim_view(std::string_view s) {
    size_t start = s.find_first_not_of(" \t\r\n");

    if (start == std::string_view::npos) {
        return "";
    }

    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::vector<std::string_view> split_view(std::string_view s, char delim) {
    std::vector<std::string_view> result;
    size_t start = 0;
    size_t end = s.find(delim);

    while (end != std::string_view::npos) {
        result.push_back(s.substr(start, end - start));
        start = end + 1;
        end = s.find(delim, start);
    }

    result.push_back(s.substr(start));
    return result;
}

static std::string normalize_path(const std::string& path) {
#ifdef _WIN32
    std::string normalized = path;
    std::replace(normalized.begin(), normalized.end(), '/', '\\');
    return normalized;
#else
    return path;
#endif
}

static std::string_view get_extension(std::string_view filename) {
    size_t dot = filename.find_last_of('.');
    return (dot != std::string_view::npos) ? filename.substr(dot) : "";
}

static int parse_int(std::string_view s, int def = 0) {
    int result = def;
    auto t = trim_view(s);
    std::from_chars(t.data(), t.data() + t.size(), result);
    return result;
}

static double parse_double(std::string_view s, double def = 0.0) {
    auto t = trim_view(s);
    if (t.empty()) {
        return def;
    }
    try {
        return std::stod(std::string(t));
    } catch (...) {
        return def;
    }
}

static std::string remove_quotes(std::string_view s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return std::string(s.substr(1, s.size() - 2));
    }
    return std::string(s);
}

static std::string to_lower(std::string_view s) {
    std::string result(s);
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

static std::pair<std::string_view, std::string_view> split_key_value(std::string_view line) {
    size_t delim = line.find(':');

    if (delim == std::string_view::npos) {
        return {"", ""};
    }

    return {trim_view(line.substr(0, delim)), trim_view(line.substr(delim + 1))};
}

static void iterate_lines(std::string_view content, const std::function<void(std::string_view)>& callback) {
    size_t start = 0;
    size_t end = content.find('\n');

    while (end != std::string_view::npos) {
        callback(trim_view(content.substr(start, end - start)));
        start = end + 1;
        end = content.find('\n', start);
    }

    if (start < content.size()) {
        callback(trim_view(content.substr(start)));
    }
}

static std::vector<std::string_view> get_section(std::string_view content, std::string_view section_name) {
    std::vector<std::string_view> result;
    std::string target = "[" + std::string(section_name) + "]";
    bool in_section = false;
    bool done = false;

    iterate_lines(content, [&](std::string_view line) {
        if (done) {
            return;
        }
        if (line.empty()) {
            return;
        }

        if (line[0] == '[') {
            if (in_section) {
                done = true;
                return;
            }
            in_section = (line == target);
            return;
        }

        if (in_section && line[0] != '/') {
            result.push_back(line);
        }
    });

    return result;
}

static general_section parse_general(const std::vector<std::string_view>& lines) {
    general_section s;

    for (const auto& line : lines) {
        auto [key, val] = split_key_value(line);

        if (key == "AudioFilename")
            s.audio_filename = std::string(val);
        else if (key == "AudioLeadIn")
            s.audio_lead_in = parse_int(val, 0);
        else if (key == "AudioHash")
            s.audio_hash = std::string(val);
        else if (key == "PreviewTime")
            s.preview_time = parse_int(val, -1);
        else if (key == "Countdown")
            s.countdown = parse_int(val, 1);
        else if (key == "SampleSet")
            s.sample_set = std::string(val);
        else if (key == "StackLeniency")
            s.stack_leniency = parse_double(val, 0.7);
        else if (key == "Mode")
            s.mode = parse_int(val, 0);
        else if (key == "LetterboxInBreaks")
            s.letterbox_in_breaks = parse_int(val, 0);
        else if (key == "StoryFireInFront")
            s.story_fire_in_front = parse_int(val, 1);
        else if (key == "UseSkinSprites")
            s.use_skin_sprites = parse_int(val, 0);
        else if (key == "AlwaysShowPlayfield")
            s.always_show_playfield = parse_int(val, 0);
        else if (key == "OverlayPosition")
            s.overlay_position = std::string(val);
        else if (key == "SkinPreference")
            s.skin_preference = std::string(val);
        else if (key == "EpilepsyWarning")
            s.epilepsy_warning = parse_int(val, 0);
        else if (key == "CountdownOffset")
            s.countdown_offset = parse_int(val, 0);
        else if (key == "SpecialStyle")
            s.special_style = parse_int(val, 0);
        else if (key == "WidescreenStoryboard")
            s.widescreen_storyboard = parse_int(val, 0);
        else if (key == "SamplesMatchPlaybackRate")
            s.samples_match_playback_rate = parse_int(val, 0);
    }

    return s;
}

static editor_section parse_editor(const std::vector<std::string_view>& lines) {
    editor_section s;

    for (const auto& line : lines) {
        auto [key, val] = split_key_value(line);

        if (key == "Bookmarks") {
            for (const auto& p : split_view(val, ',')) {
                auto t = trim_view(p);
                if (!t.empty()) {
                    s.bookmarks.push_back(parse_int(t, 0));
                }
            }
        } else if (key == "DistanceSpacing")
            s.distance_spacing = parse_double(val, 1.0);
        else if (key == "BeatDivisor")
            s.beat_divisor = parse_int(val, 4);
        else if (key == "GridSize")
            s.grid_size = parse_int(val, 4);
        else if (key == "TimelineZoom")
            s.timeline_zoom = parse_double(val, 1.0);
    }

    return s;
}

static metadata_section parse_metadata(const std::vector<std::string_view>& lines) {
    metadata_section s;

    for (const auto& line : lines) {
        auto [key, val] = split_key_value(line);

        if (key == "Title")
            s.title = std::string(val);
        else if (key == "TitleUnicode")
            s.title_unicode = std::string(val);
        else if (key == "Artist")
            s.artist = std::string(val);
        else if (key == "ArtistUnicode")
            s.artist_unicode = std::string(val);
        else if (key == "Creator")
            s.creator = std::string(val);
        else if (key == "Version")
            s.version = std::string(val);
        else if (key == "Source")
            s.source = std::string(val);
        else if (key == "Tags")
            s.tags = std::string(val);
        else if (key == "BeatmapID")
            s.beatmap_id = parse_int(val, -1);
        else if (key == "BeatmapSetID")
            s.beatmap_set_id = parse_int(val, -1);
    }

    return s;
}

static difficulty_section parse_difficulty(const std::vector<std::string_view>& lines) {
    difficulty_section s;

    for (const auto& line : lines) {
        auto [key, val] = split_key_value(line);

        if (key == "HPDrainRate")
            s.hp_drain_rate = parse_double(val, 5.0);
        else if (key == "CircleSize")
            s.circle_size = parse_double(val, 5.0);
        else if (key == "OverallDifficulty")
            s.overall_difficulty = parse_double(val, 5.0);
        else if (key == "ApproachRate")
            s.approach_rate = parse_double(val, 5.0);
        else if (key == "SliderMultiplier")
            s.slider_multiplier = parse_double(val, 1.4);
        else if (key == "SliderTickRate")
            s.slider_tick_rate = parse_double(val, 1.0);
    }

    return s;
}

void parse_events(const std::vector<std::string_view>& lines, std::optional<event_background>& bg,
                  std::optional<event_video>& vid, std::vector<event_break>& breaks) {
    for (const auto& line : lines) {
        auto parts = split_view(line, ',');
        if (parts.empty()) {
            continue;
        }

        auto type = trim_view(parts[0]);

        if (type == "0" && parts.size() >= 3 && trim_view(parts[1]) == "0") {
            std::string filename = remove_quotes(trim_view(parts[2]));
            std::string ext = to_lower(get_extension(filename));

            if (get_image_extensions().count(ext)) {
                event_background b;
                b.filename = normalize_path(filename);
                if (parts.size() >= 4)
                    b.x_offset = parse_int(parts[3], 0);
                if (parts.size() >= 5)
                    b.y_offset = parse_int(parts[4], 0);
                bg = b;
            }
        } else if ((type == "1" || type == "Video") && parts.size() >= 3) {
            std::string filename = remove_quotes(trim_view(parts[2]));
            std::string ext = to_lower(get_extension(filename));

            if (get_video_extensions().count(ext)) {
                event_video v;
                v.start_time = parse_int(parts[1], 0);
                v.filename = normalize_path(filename);
                if (parts.size() >= 4)
                    v.x_offset = parse_int(parts[3], 0);
                if (parts.size() >= 5)
                    v.y_offset = parse_int(parts[4], 0);
                vid = v;
            }
        } else if ((type == "2" || type == "Break") && parts.size() >= 3) {
            event_break b;
            b.start_time = parse_int(parts[1], 0);
            b.end_time = parse_int(parts[2], 0);
            breaks.push_back(b);
        }
    }
}

static std::vector<timing_point> parse_timing_points(const std::vector<std::string_view>& lines) {
    std::vector<timing_point> points;
    points.reserve(lines.size());

    for (const auto& line : lines) {
        auto parts = split_view(line, ',');
        if (parts.size() < 2) {
            continue;
        }

        timing_point tp;
        tp.time = parse_int(parts[0], 0);
        tp.beat_length = parse_double(parts[1], 0.0);
        if (parts.size() > 2)
            tp.meter = parse_int(parts[2], 4);
        if (parts.size() > 3)
            tp.sample_set = parse_int(parts[3], 0);
        if (parts.size() > 4)
            tp.sample_index = parse_int(parts[4], 0);
        if (parts.size() > 5)
            tp.volume = parse_int(parts[5], 100);
        if (parts.size() > 6)
            tp.uninherited = parse_int(parts[6], 1);
        if (parts.size() > 7)
            tp.effects = parse_int(parts[7], 0);

        points.push_back(tp);
    }

    return points;
}

static colour_section parse_colours(const std::vector<std::string_view>& lines) {
    colour_section s;

    for (const auto& line : lines) {
        auto [key, val] = split_key_value(line);
        auto parts = split_view(val, ',');

        if (parts.size() < 3) {
            continue;
        }

        std::array<int, 3> color = {parse_int(parts[0], 0), parse_int(parts[1], 0), parse_int(parts[2], 0)};

        if (key.find("Combo") != std::string_view::npos) {
            s.combos.push_back(color);
        } else if (key == "SliderTrackOverride") {
            s.slider_track_override = color;
        } else if (key == "SliderBorder") {
            s.slider_border = color;
        }
    }

    return s;
}

static hit_sample parse_hit_sample(std::string_view str) {
    hit_sample hs;
    auto parts = split_view(str, ':');

    if (!parts.empty())
        hs.normal_set = parse_int(parts[0], 0);
    if (parts.size() > 1)
        hs.addition_set = parse_int(parts[1], 0);
    if (parts.size() > 2)
        hs.index = parse_int(parts[2], 0);
    if (parts.size() > 3)
        hs.volume = parse_int(parts[3], 0);
    if (parts.size() > 4)
        hs.filename = std::string(trim_view(parts[4]));

    return hs;
}

static hit_sample default_hit_sample() {
    return parse_hit_sample("0:0:0:0:");
}

static std::vector<hit_object> parse_hit_objects(const std::vector<std::string_view>& lines) {
    std::vector<hit_object> objects;
    objects.reserve(lines.size());

    for (const auto& line : lines) {
        auto parts = split_view(line, ',');
        if (parts.size() < 5) {
            continue;
        }

        hit_object ho;
        ho.x = parse_int(parts[0], 0);
        ho.y = parse_int(parts[1], 0);
        ho.time = parse_int(parts[2], 0);
        ho.type = parse_int(parts[3], 0);
        ho.hit_sound = parse_int(parts[4], 0);

        bool is_circle = (ho.type & 1) != 0;
        bool is_slider = (ho.type & 2) != 0;
        bool is_spinner = (ho.type & 8) != 0;
        bool is_hold = (ho.type & 128) != 0;

        if (is_slider && parts.size() >= 8) {
            auto curve_parts = split_view(parts[5], '|');

            if (!curve_parts.empty()) {
                auto type_str = trim_view(curve_parts[0]);
                if (!type_str.empty()) {
                    ho.curve_type = type_str[0];
                }

                for (size_t i = 1; i < curve_parts.size(); i++) {
                    auto point = split_view(curve_parts[i], ':');
                    if (point.size() >= 2) {
                        ho.curve_points.push_back({parse_int(point[0], 0), parse_int(point[1], 0)});
                    }
                }
            }

            ho.slides = parse_int(parts[6], 1);
            ho.length = parse_double(parts[7], 0.0);
            int edge_count = ho.slides + 1;

            if (parts.size() > 8) {
                for (const auto& s : split_view(parts[8], '|')) {
                    ho.edge_sounds.push_back(parse_int(s, 0));
                }
            } else {
                ho.edge_sounds.assign(edge_count, 0);
            }

            if (parts.size() > 9) {
                for (const auto& ep : split_view(parts[9], '|')) {
                    auto set = split_view(ep, ':');
                    if (set.size() >= 2) {
                        ho.edge_sets.push_back({parse_int(set[0], 0), parse_int(set[1], 0)});
                    }
                }
            } else {
                ho.edge_sets.assign(edge_count, {0, 0});
            }

            if (parts.size() > 10) {
                ho.sample = parse_hit_sample(parts[10]);
            } else {
                ho.sample = default_hit_sample();
            }
        } else if (is_spinner && parts.size() >= 6) {
            ho.end_time = parse_int(parts[5], 0);
            if (parts.size() > 6) {
                ho.sample = parse_hit_sample(parts[6]);
            } else {
                ho.sample = default_hit_sample();
            }
        } else if (is_hold && parts.size() >= 6) {
            auto hold_parts = split_view(parts[5], ':');
            if (!hold_parts.empty()) {
                ho.end_time = parse_int(hold_parts[0], 0);
            }
            if (hold_parts.size() > 1) {
                std::string sample_str;
                for (size_t i = 1; i < hold_parts.size(); i++) {
                    if (i > 1)
                        sample_str += ":";
                    sample_str += std::string(hold_parts[i]);
                }
                ho.sample = parse_hit_sample(sample_str);
            } else {
                ho.sample = default_hit_sample();
            }
        } else if (is_circle && parts.size() >= 6) {
            ho.sample = parse_hit_sample(parts[5]);
        } else if (is_circle) {
            ho.sample = default_hit_sample();
        }

        objects.push_back(ho);
    }

    return objects;
}

static int parse_version(std::string_view content) {
    size_t end = content.find('\n');
    auto first_line = trim_view((end != std::string_view::npos) ? content.substr(0, end) : content);

    size_t v_pos = first_line.find('v');
    if (v_pos != std::string_view::npos) {
        return parse_int(first_line.substr(v_pos + 1), 14);
    }

    return 14;
}

static bool read_file_text(const std::string& location, std::string& out) {
    std::ifstream file(location, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    const std::ifstream::pos_type size = file.tellg();
    if (size < 0) {
        return false;
    }

    out.resize(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    if (!file.read(out.data(), static_cast<std::streamsize>(out.size()))) {
        out.clear();
        return false;
    }

    return true;
}

bool beatmap_parser::parse(std::string location) {
    if (data == nullptr) {
        last_error = "parser data is null";
        return false;
    }

    this->location = std::move(location);
    std::string content;
    if (!read_file_text(this->location, content)) {
        last_error = "failed to read file";
        return false;
    }

    debug_print("file: " + this->location);
    debug_print("size: " + std::to_string(content.size()));
    debug_print("preview: " + debug_preview(content, 80));

    const int parsed_version = parse_version(content);
    debug_print("parsed version: " + std::to_string(parsed_version));

    *data = osu_beatmap();
    data->version = parsed_version;
    data->general = parse_general(get_section(content, "General"));
    data->editor = parse_editor(get_section(content, "Editor"));
    data->metadata = parse_metadata(get_section(content, "Metadata"));
    data->difficulty = parse_difficulty(get_section(content, "Difficulty"));

    auto events = get_section(content, "Events");
    parse_events(events, data->background, data->video, data->breaks);

    data->timing_points = parse_timing_points(get_section(content, "TimingPoints"));
    data->colours = parse_colours(get_section(content, "Colours"));
    data->hit_objects = parse_hit_objects(get_section(content, "HitObjects"));

    data->version = parsed_version;
    last_error.clear();
    return true;
}

bool beatmap_parser::write() {
    if (data == nullptr || location.empty()) {
        last_error = data == nullptr ? "parser data is null" : "location is empty";
        return false;
    }

    last_error = "write not implemented";
    return false;
}
