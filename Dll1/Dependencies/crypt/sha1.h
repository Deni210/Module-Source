#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <sstream>
#include <iomanip>
#include <cstdint>

class SHA1 {
public:
    static std::string hash(const std::string& input) {
        SHA1 sha;
        sha.process(input);
        return sha.finalize();
    }

private:
    static constexpr std::array<uint32_t, 5> H_INIT = {
        0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0
    };

    std::array<uint32_t, 5> hash_values = H_INIT;
    std::vector<uint8_t> buffer;
    uint64_t total_bits = 0;

    static uint32_t rotate_left(uint32_t x, uint32_t n) {
        return (x << n) | (x >> (32 - n));
    }

    static uint32_t f(uint32_t t, uint32_t b, uint32_t c, uint32_t d) {
        if (t < 20) return (b & c) | (~b & d);
        if (t < 40) return b ^ c ^ d;
        if (t < 60) return (b & c) | (b & d) | (c & d);
        return b ^ c ^ d;
    }

    static uint32_t k(uint32_t t) {
        if (t < 20) return 0x5A827999;
        if (t < 40) return 0x6ED9EBA1;
        if (t < 60) return 0x8F1BBCDC;
        return 0xCA62C1D6;
    }

    void process(const std::string& input) {
        buffer.assign(input.begin(), input.end());
        total_bits = buffer.size() * 8;
        buffer.push_back(0x80);
        while ((buffer.size() + 8) % 64 != 0) {
            buffer.push_back(0);
        }
        for (int i = 7; i >= 0; --i) {
            buffer.push_back((total_bits >> (i * 8)) & 0xFF);
        }
        process_blocks();
    }

    void process_blocks() {
        for (size_t i = 0; i < buffer.size(); i += 64) {
            std::array<uint32_t, 80> w = {};
            for (size_t j = 0; j < 16; ++j) {
                w[j] = (buffer[i + j * 4] << 24) | (buffer[i + j * 4 + 1] << 16) |
                    (buffer[i + j * 4 + 2] << 8) | (buffer[i + j * 4 + 3]);
            }
            for (size_t j = 16; j < 80; ++j) {
                w[j] = rotate_left(w[j - 3] ^ w[j - 8] ^ w[j - 14] ^ w[j - 16], 1);
            }
            std::array<uint32_t, 5> h = hash_values;
            for (size_t j = 0; j < 80; ++j) {
                uint32_t temp = rotate_left(h[0], 5) + f(j, h[1], h[2], h[3]) + h[4] + k(j) + w[j];
                h[4] = h[3]; h[3] = h[2]; h[2] = rotate_left(h[1], 30); h[1] = h[0]; h[0] = temp;
            }
            for (size_t j = 0; j < 5; ++j) {
                hash_values[j] += h[j];
            }
        }
    }

    std::string finalize() {
        std::ostringstream result;
        for (auto value : hash_values) {
            result << std::hex << std::setw(8) << std::setfill('0') << value;
        }
        return result.str();
    }
};