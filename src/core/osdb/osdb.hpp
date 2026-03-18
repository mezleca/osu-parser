#pragma once

#include <cstdint>
#include <string>
#include <vector>

// === data ===
struct osdb_beatmap {
    int32_t difficulty_id;
    int32_t beatmapset_id;
    std::string artist;
    std::string title;
    std::string difficulty;
    std::string checksum;
    std::string user_comment;
    int32_t mode;
    int32_t difficulty_rating;
};

struct osdb_collection {
    std::string name;
    int32_t online_id;
    std::vector<osdb_beatmap> beatmaps;
    std::vector<std::string> hash_only_beatmaps;
};

struct osdb_data {
    int64_t save_data;
    std::string last_editor;
    int32_t count;
    std::vector<osdb_collection> collections;
};

// === parser ===
struct osdb_parser {
    osdb_data* data;
    std::string location;
    std::string last_error;

    bool parse(std::string location);
    bool write();
};
