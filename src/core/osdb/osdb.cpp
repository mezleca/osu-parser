#include "osdb.hpp"

#include "utils/binary.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <zlib.h>

static bool ends_with(const std::string& value, const char* suffix) {
    const size_t suffix_len = std::strlen(suffix);
    if (value.size() < suffix_len) {
        return false;
    }
    return value.compare(value.size() - suffix_len, suffix_len, suffix) == 0;
}

static int osdb_version_to_code(const std::string& version) {
    if (version == "o!dm") {
        return 1;
    }
    if (version == "o!dm2") {
        return 2;
    }
    if (version == "o!dm3") {
        return 3;
    }
    if (version == "o!dm4") {
        return 4;
    }
    if (version == "o!dm5") {
        return 5;
    }
    if (version == "o!dm6") {
        return 6;
    }
    if (version == "o!dm7") {
        return 7;
    }
    if (version == "o!dm8") {
        return 8;
    }
    if (version == "o!dm7min") {
        return 1007;
    }
    if (version == "o!dm8min") {
        return 1008;
    }
    return 0;
}

static bool gzip_decompress(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
    if (input.empty()) {
        output.clear();
        return true;
    }

    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input.data()));
    stream.avail_in = static_cast<uInt>(input.size());

    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        return false;
    }

    const size_t chunk_size = 262144;
    output.clear();
    int status = Z_OK;

    while (status != Z_STREAM_END) {
        const size_t start = output.size();
        output.resize(start + chunk_size);
        stream.next_out = reinterpret_cast<Bytef*>(output.data() + start);
        stream.avail_out = static_cast<uInt>(chunk_size);

        status = inflate(&stream, Z_NO_FLUSH);
        if (status != Z_OK && status != Z_STREAM_END) {
            inflateEnd(&stream);
            return false;
        }

        output.resize(start + (chunk_size - stream.avail_out));
    }

    inflateEnd(&stream);
    return true;
}

static bool gzip_compress(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input.data()));
    stream.avail_in = static_cast<uInt>(input.size());

    if (deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return false;
    }

    const size_t chunk_size = 262144;
    output.clear();
    int status = Z_OK;

    while (status != Z_STREAM_END) {
        const size_t start = output.size();
        output.resize(start + chunk_size);
        stream.next_out = reinterpret_cast<Bytef*>(output.data() + start);
        stream.avail_out = static_cast<uInt>(chunk_size);

        status = deflate(&stream, stream.avail_in ? Z_NO_FLUSH : Z_FINISH);
        if (status != Z_OK && status != Z_STREAM_END) {
            deflateEnd(&stream);
            return false;
        }

        output.resize(start + (chunk_size - stream.avail_out));
    }

    deflateEnd(&stream);
    return true;
}

bool osdb_parser::parse(std::string location) {
    if (data == nullptr) {
        last_error = "parser data is null";
        return false;
    }

    this->location = std::move(location);
    std::vector<uint8_t> buffer;
    if (!osu_binary::read_file_buffer(this->location, buffer)) {
        last_error = "failed to read file";
        *data = osdb_data();
        return false;
    }

    try {
        osu_binary::binary_cursor cursor;
        osu_binary::set_cursor(cursor, buffer);

        const std::string version_string = osu_binary::read_string2(cursor);
        const int version = osdb_version_to_code(version_string);
        if (version == 0) {
            last_error = "invalid osdb version";
            *data = osdb_data();
            return false;
        }

        const bool is_minimal = ends_with(version_string, "min");
        std::vector<uint8_t> decompressed;

        if (version >= 7) {
            std::vector<uint8_t> compressed(buffer.begin() + static_cast<std::ptrdiff_t>(cursor.offset), buffer.end());
            if (!gzip_decompress(compressed, decompressed)) {
                last_error = "failed to decompress osdb data";
                *data = osdb_data();
                return false;
            }

            osu_binary::set_cursor(cursor, decompressed);
            osu_binary::read_string2(cursor);
        }

        data->save_data = osu_binary::read_i64(cursor);
        data->last_editor = osu_binary::read_string2(cursor);
        data->count = osu_binary::read_i32(cursor);
        data->collections.clear();
        data->collections.reserve(static_cast<size_t>(std::max(0, data->count)));

        for (int32_t i = 0; i < data->count; i++) {
            osdb_collection collection;
            collection.name = osu_binary::read_string2(cursor);

            if (version >= 7) {
                collection.online_id = osu_binary::read_i32(cursor);
            } else {
                collection.online_id = 0;
            }

            const int32_t beatmaps_count = osu_binary::read_i32(cursor);
            collection.beatmaps.clear();
            collection.beatmaps.reserve(static_cast<size_t>(std::max(0, beatmaps_count)));

            for (int32_t j = 0; j < beatmaps_count; j++) {
                osdb_beatmap beatmap;
                beatmap.difficulty_id = osu_binary::read_i32(cursor);
                beatmap.beatmapset_id = version >= 2 ? osu_binary::read_i32(cursor) : -1;

                if (!is_minimal) {
                    beatmap.artist = osu_binary::read_string2(cursor);
                    beatmap.title = osu_binary::read_string2(cursor);
                    beatmap.difficulty = osu_binary::read_string2(cursor);
                }

                beatmap.checksum = osu_binary::read_string2(cursor);

                if (version >= 4) {
                    beatmap.user_comment = osu_binary::read_string2(cursor);
                }

                if (version >= 8 || (version >= 5 && !is_minimal)) {
                    beatmap.mode = osu_binary::read_u8(cursor);
                }

                if (version >= 8 || (version >= 6 && !is_minimal)) {
                    beatmap.difficulty_rating = osu_binary::read_f64(cursor);
                }

                collection.beatmaps.push_back(std::move(beatmap));
            }

            if (version >= 3) {
                const int32_t hash_count = osu_binary::read_i32(cursor);
                collection.hash_only_beatmaps.clear();
                collection.hash_only_beatmaps.reserve(static_cast<size_t>(std::max(0, hash_count)));

                for (int32_t j = 0; j < hash_count; j++) {
                    collection.hash_only_beatmaps.push_back(osu_binary::read_string2(cursor));
                }
            }

            data->collections.push_back(std::move(collection));
        }

        const std::string footer = osu_binary::read_string2(cursor);
        if (footer != "By Piotrekol") {
            last_error = "invalid osdb footer";
            *data = osdb_data();
            return false;
        }

        last_error.clear();
        return true;
    } catch (const std::exception& e) {
        last_error = e.what();
        *data = osdb_data();
        return false;
    } catch (...) {
        last_error = "unknown error";
        *data = osdb_data();
        return false;
    }
}

bool osdb_parser::write() {
    if (data == nullptr || location.empty()) {
        last_error = data == nullptr ? "parser data is null" : "location is empty";
        return false;
    }

    if (data->collections.empty()) {
        last_error = "no collections to write";
        return false;
    }

    const std::string version_string = "o!dm8min";
    const int version = osdb_version_to_code(version_string);
    const bool is_minimal = ends_with(version_string, "min");

    std::vector<uint8_t> content;
    if (version >= 7) {
        osu_binary::write_string2(content, version_string);
    }

    const int64_t save_time = data->save_data != 0
                                  ? data->save_data
                                  : static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                             std::chrono::system_clock::now().time_since_epoch())
                                                             .count());

    osu_binary::write_i64(content, save_time);
    osu_binary::write_string2(content, data->last_editor);
    osu_binary::write_i32(content, static_cast<int32_t>(data->collections.size()));

    for (const auto& collection : data->collections) {
        osu_binary::write_string2(content, collection.name);

        if (version >= 7) {
            osu_binary::write_i32(content, collection.online_id);
        }

        osu_binary::write_i32(content, static_cast<int32_t>(collection.beatmaps.size()));

        for (const auto& beatmap : collection.beatmaps) {
            osu_binary::write_i32(content, beatmap.difficulty_id);
            if (version >= 2) {
                osu_binary::write_i32(content, beatmap.beatmapset_id);
            }

            if (!is_minimal) {
                osu_binary::write_string2(content, beatmap.artist);
                osu_binary::write_string2(content, beatmap.title);
                osu_binary::write_string2(content, beatmap.difficulty);
            }

            osu_binary::write_string2(content, beatmap.checksum);

            if (version >= 4) {
                osu_binary::write_string2(content, beatmap.user_comment);
            }

            if (version >= 8 || (version >= 5 && !is_minimal)) {
                osu_binary::write_u8(content, static_cast<uint8_t>(beatmap.mode));
            }

            if (version >= 8 || (version >= 6 && !is_minimal)) {
                osu_binary::write_f64(content, beatmap.difficulty_rating);
            }
        }

        if (version >= 3) {
            osu_binary::write_i32(content, static_cast<int32_t>(collection.hash_only_beatmaps.size()));
            for (const auto& hash : collection.hash_only_beatmaps) {
                osu_binary::write_string2(content, hash);
            }
        }
    }

    osu_binary::write_string2(content, "By Piotrekol");

    std::vector<uint8_t> buffer;
    osu_binary::write_string2(buffer, version_string);

    if (version >= 7) {
        std::vector<uint8_t> compressed;
        if (!gzip_compress(content, compressed)) {
            last_error = "failed to compress osdb data";
            return false;
        }
        buffer.insert(buffer.end(), compressed.begin(), compressed.end());
    } else {
        buffer.insert(buffer.end(), content.begin(), content.end());
    }

    if (!osu_binary::write_file_buffer(location, buffer)) {
        last_error = "failed to write file";
        return false;
    }

    last_error.clear();
    return true;
}
