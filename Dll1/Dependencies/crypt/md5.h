#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

std::string md5(const std::string& input) {
    typedef unsigned int uint32;
    typedef unsigned char uint8;

    const uint32 s[64] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };

    const uint32 K[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    auto leftRotate = [](uint32 x, uint32 c) -> uint32 {
        return (x << c) | (x >> (32 - c));
        };

    uint32 a0 = 0x67452301;
    uint32 b0 = 0xefcdab89;
    uint32 c0 = 0x98badcfe;
    uint32 d0 = 0x10325476;

    uint64_t original_len = input.length();
    uint64_t bit_len = original_len * 8;

    uint64_t padded_len = ((((original_len + 8) / 64) + 1) * 64) - 8;
    uint8* msg = new uint8[padded_len + 8];

    for (uint64_t i = 0; i < original_len; ++i) {
        msg[i] = input[i];
    }

    msg[original_len] = 0x80;

    for (uint64_t i = original_len + 1; i < padded_len; ++i) {
        msg[i] = 0;
    }

    for (int i = 0; i < 8; ++i) {
        msg[padded_len + i] = (bit_len >> (i * 8)) & 0xff;
    }

    for (uint64_t chunk_start = 0; chunk_start < padded_len; chunk_start += 64) {
        uint32 M[16];
        for (int i = 0; i < 16; ++i) {
            M[i] = (msg[chunk_start + i * 4]) |
                (msg[chunk_start + i * 4 + 1] << 8) |
                (msg[chunk_start + i * 4 + 2] << 16) |
                (msg[chunk_start + i * 4 + 3] << 24);
        }

        uint32 A = a0;
        uint32 B = b0;
        uint32 C = c0;
        uint32 D = d0;

        for (int i = 0; i < 64; ++i) {
            uint32 F, g;

            if (i < 16) {
                F = (B & C) | ((~B) & D);
                g = i;
            }
            else if (i < 32) {
                F = (D & B) | ((~D) & C);
                g = (5 * i + 1) % 16;
            }
            else if (i < 48) {
                F = B ^ C ^ D;
                g = (3 * i + 5) % 16;
            }
            else {
                F = C ^ (B | (~D));
                g = (7 * i) % 16;
            }

            uint32 temp = D;
            D = C;
            C = B;
            B = B + leftRotate((A + F + K[i] + M[g]), s[i]);
            A = temp;
        }

        a0 += A;
        b0 += B;
        c0 += C;
        d0 += D;
    }

    delete[] msg;

    std::stringstream ss;

    uint8 bytes[16];
    for (int i = 0; i < 4; ++i) {
        bytes[i] = (a0 >> (i * 8)) & 0xff;
        bytes[i + 4] = (b0 >> (i * 8)) & 0xff;
        bytes[i + 8] = (c0 >> (i * 8)) & 0xff;
        bytes[i + 12] = (d0 >> (i * 8)) & 0xff;
    }

    for (int i = 0; i < 16; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
    }

    return ss.str();
}