#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace osu_binary {
    struct binary_cursor {
        const std::vector<uint8_t>* buffer = nullptr;
        size_t offset = 0;
    };

    inline void set_cursor(binary_cursor& cursor, const std::vector<uint8_t>& data) {
        cursor.buffer = &data;
        cursor.offset = 0;
    }

    inline void ensure_range(const binary_cursor& cursor, size_t bytes) {
        if (!cursor.buffer) {
            throw std::runtime_error("binary read out of range");
        }

        if (cursor.offset > cursor.buffer->size()) {
            throw std::runtime_error("binary read out of range");
        }

        const size_t remaining = cursor.buffer->size() - cursor.offset;
        if (bytes > remaining) {
            throw std::runtime_error("binary read out of range");
        }
    }

    template <typename T> inline T read_integral(binary_cursor& cursor) {
        T value = 0;
        ensure_range(cursor, sizeof(T));
        std::memcpy(&value, cursor.buffer->data() + cursor.offset, sizeof(T));
        cursor.offset += sizeof(T);
        return value;
    }

    inline uint8_t read_u8(binary_cursor& cursor) {
        return read_integral<uint8_t>(cursor);
    }

    inline int8_t read_i8(binary_cursor& cursor) {
        return read_integral<int8_t>(cursor);
    }

    inline uint16_t read_u16(binary_cursor& cursor) {
        return read_integral<uint16_t>(cursor);
    }

    inline int16_t read_i16(binary_cursor& cursor) {
        return read_integral<int16_t>(cursor);
    }

    inline uint32_t read_u32(binary_cursor& cursor) {
        return read_integral<uint32_t>(cursor);
    }

    inline int32_t read_i32(binary_cursor& cursor) {
        return read_integral<int32_t>(cursor);
    }

    inline uint64_t read_u64(binary_cursor& cursor) {
        return read_integral<uint64_t>(cursor);
    }

    inline int64_t read_i64(binary_cursor& cursor) {
        return read_integral<int64_t>(cursor);
    }

    inline float read_f32(binary_cursor& cursor) {
        float value = 0.0f;
        ensure_range(cursor, sizeof(value));
        std::memcpy(&value, cursor.buffer->data() + cursor.offset, sizeof(value));
        cursor.offset += sizeof(value);
        return value;
    }

    inline double read_f64(binary_cursor& cursor) {
        double value = 0.0;
        ensure_range(cursor, sizeof(value));
        std::memcpy(&value, cursor.buffer->data() + cursor.offset, sizeof(value));
        cursor.offset += sizeof(value);
        return value;
    }

    inline bool read_bool(binary_cursor& cursor) {
        return read_u8(cursor) != 0;
    }

    inline uint32_t read_uleb128(binary_cursor& cursor) {
        uint32_t result = 0;
        int shift = 0;
        bool has_more = true;

        // ULEB128 for uint32_t must fit within 5 bytes.
        for (int i = 0; i < 5; i++) {
            uint8_t byte = read_u8(cursor);
            result |= static_cast<uint32_t>(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) {
                has_more = false;
                break;
            }
            shift += 7;
        }

        if (has_more) {
            throw std::runtime_error("uleb128 overflow");
        }

        return result;
    }

    inline std::string read_string(binary_cursor& cursor) {
        uint8_t marker = read_u8(cursor);
        if (marker == 0x00) {
            return "";
        }

        if (marker != 0x0B) {
            throw std::runtime_error("invalid string marker");
        }

        uint32_t length = read_uleb128(cursor);
        ensure_range(cursor, length);
        std::string value(reinterpret_cast<const char*>(cursor.buffer->data() + cursor.offset), length);
        cursor.offset += length;
        return value;
    }

    inline std::string read_string2(binary_cursor& cursor) {
        uint32_t length = read_uleb128(cursor);
        ensure_range(cursor, length);
        std::string value(reinterpret_cast<const char*>(cursor.buffer->data() + cursor.offset), length);
        cursor.offset += length;
        return value;
    }

    inline void skip(binary_cursor& cursor, size_t bytes) {
        ensure_range(cursor, bytes);
        cursor.offset += bytes;
    }

    template <typename T> inline void append_integral(std::vector<uint8_t>& out, T value) {
        const size_t start = out.size();
        out.resize(start + sizeof(T));
        std::memcpy(out.data() + start, &value, sizeof(T));
    }

    inline void write_u8(std::vector<uint8_t>& out, uint8_t value) {
        out.push_back(value);
    }

    inline void write_i8(std::vector<uint8_t>& out, int8_t value) {
        append_integral(out, value);
    }

    inline void write_u16(std::vector<uint8_t>& out, uint16_t value) {
        append_integral(out, value);
    }

    inline void write_i16(std::vector<uint8_t>& out, int16_t value) {
        append_integral(out, value);
    }

    inline void write_u32(std::vector<uint8_t>& out, uint32_t value) {
        append_integral(out, value);
    }

    inline void write_i32(std::vector<uint8_t>& out, int32_t value) {
        append_integral(out, value);
    }

    inline void write_u64(std::vector<uint8_t>& out, uint64_t value) {
        append_integral(out, value);
    }

    inline void write_i64(std::vector<uint8_t>& out, int64_t value) {
        append_integral(out, value);
    }

    inline void write_f32(std::vector<uint8_t>& out, float value) {
        append_integral(out, value);
    }

    inline void write_f64(std::vector<uint8_t>& out, double value) {
        append_integral(out, value);
    }

    inline void write_bool(std::vector<uint8_t>& out, bool value) {
        out.push_back(value ? 1 : 0);
    }

    inline void write_uleb128(std::vector<uint8_t>& out, uint32_t value) {
        do {
            uint8_t byte = static_cast<uint8_t>(value & 0x7F);
            value >>= 7;
            if (value != 0) {
                byte |= 0x80;
            }
            out.push_back(byte);
        } while (value != 0);
    }

    inline void write_string(std::vector<uint8_t>& out, const std::string& value) {
        if (value.empty()) {
            out.push_back(0x00);
            return;
        }

        out.push_back(0x0B);
        write_uleb128(out, static_cast<uint32_t>(value.size()));
        out.insert(out.end(), value.begin(), value.end());
    }

    inline void write_string2(std::vector<uint8_t>& out, const std::string& value) {
        write_uleb128(out, static_cast<uint32_t>(value.size()));
        out.insert(out.end(), value.begin(), value.end());
    }

    inline bool read_file_buffer(const std::string& location, std::vector<uint8_t>& out) {
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
        if (!file.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(out.size()))) {
            out.clear();
            return false;
        }

        return true;
    }

    inline bool write_file_buffer(const std::string& location, const std::vector<uint8_t>& buffer) {
        std::ofstream file(location, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            return false;
        }

        if (!buffer.empty()) {
            file.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        }

        return file.good();
    }
}
