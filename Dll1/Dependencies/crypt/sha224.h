#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <sstream>
#include <iomanip>
#include <cstdint>

class SHA224 {
public:
    static std::string hash(const std::string& input) {
        SHA224 sha;
        sha.process(input);
        return sha.finalize();
    }

private:
    static constexpr std::array<uint32_t, 8> H_INIT = {
        0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939,
        0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4
    };

    std::array<uint32_t, 8> hash_values = H_INIT;
    std::vector<uint8_t> buffer;
    uint64_t total_bits = 0;

    static uint32_t rotate_right(uint32_t x, uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }

    static uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (~x & z);
    }

    static uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    static uint32_t sigma0(uint32_t x) {
        return rotate_right(x, 2) ^ rotate_right(x, 13) ^ rotate_right(x, 22);
    }

    static uint32_t sigma1(uint32_t x) {
        return rotate_right(x, 6) ^ rotate_right(x, 11) ^ rotate_right(x, 25);
    }

    static uint32_t gamma0(uint32_t x) {
        return rotate_right(x, 7) ^ rotate_right(x, 18) ^ (x >> 3);
    }

    static uint32_t gamma1(uint32_t x) {
        return rotate_right(x, 17) ^ rotate_right(x, 19) ^ (x >> 10);
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
        static constexpr std::array<uint32_t, 64> K = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
            0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
            0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
            0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
            0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
            0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        for (size_t i = 0; i < buffer.size(); i += 64) {
            std::array<uint32_t, 64> w = {};
            for (size_t j = 0; j < 16; ++j) {
                w[j] = (buffer[i + j * 4] << 24) | (buffer[i + j * 4 + 1] << 16) |
                    (buffer[i + j * 4 + 2] << 8) | (buffer[i + j * 4 + 3]);
            }
            for (size_t j = 16; j < 64; ++j) {
                w[j] = gamma1(w[j - 2]) + w[j - 7] + gamma0(w[j - 15]) + w[j - 16];
            }
            std::array<uint32_t, 8> h = hash_values;
            for (size_t j = 0; j < 64; ++j) {
                uint32_t temp1 = h[7] + sigma1(h[4]) + ch(h[4], h[5], h[6]) + K[j] + w[j];
                uint32_t temp2 = sigma0(h[0]) + maj(h[0], h[1], h[2]);
                h[7] = h[6]; h[6] = h[5]; h[5] = h[4]; h[4] = h[3] + temp1;
                h[3] = h[2]; h[2] = h[1]; h[1] = h[0]; h[0] = temp1 + temp2;
            }
            for (size_t j = 0; j < 8; ++j) {
                hash_values[j] += h[j];
            }
        }
    }

    std::string finalize() {
        std::ostringstream result;
        for (size_t i = 0; i < 7; ++i) {
            result << std::hex << std::setw(8) << std::setfill('0') << hash_values[i];
        }
        return result.str();
    }
};