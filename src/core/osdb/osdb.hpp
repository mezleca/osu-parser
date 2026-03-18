#pragma once

#include <cstdint>
#include <string>
#include <vector>

// === data ===
struct osdb_beatmap {
    int32_t difficulty_id = 0;
    int32_t beatmapset_id = -1;
    std::string artist;
    std::string title;
    std::string difficulty;
    std::string checksum;
    std::string user_comment;
    int32_t mode = 0;
    double difficulty_rating = 0.0;
};

struct osdb_collection {
    std::string name;
    int32_t online_id = 0;
    std::vector<osdb_beatmap> beatmaps;
    std::vector<std::string> hash_only_beatmaps;
};

struct osdb_data {
    std::string version_string;
    int64_t save_data = 0;
    std::string last_editor;
    int32_t count = 0;
    std::vector<osdb_collection> collections;
};

// === parser ===
struct osdb_parser {
    // data is non-owning and managed by parser_base
    osdb_data* data;
    std::string location;
    std::string last_error;

    bool parse(std::string location);
    bool write();
};
