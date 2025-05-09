#include "AES.h"
#include "base64.h"
#include <random>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <windows.h>

AES::AES() {
    mbedtls_aes_init(&ctx);
}

AES::~AES() {
    mbedtls_aes_free(&ctx);
}

std::pair<std::string, std::string> AES::encrypt(const std::string& data,
    const std::string& key,
    const std::string& iv,
    const std::string& mode) {

    std::vector<unsigned char> ivBytes(BLOCK_SIZE);

    if (iv.empty()) {
        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&ctr_drbg);

        mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, nullptr, 0);
        mbedtls_ctr_drbg_random(&ctr_drbg, ivBytes.data(), BLOCK_SIZE);

        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
    }
    else {
        std::string decodedIv = base64_decode(iv);
        if (decodedIv.size() != BLOCK_SIZE)  return {};
        memcpy(ivBytes.data(), decodedIv.data(), BLOCK_SIZE);
    }

    std::string ivStr(ivBytes.begin(), ivBytes.end());

    std::vector<unsigned char> paddedData(data.begin(), data.end());
    size_t padding = BLOCK_SIZE - (data.length() % BLOCK_SIZE);
    for (size_t i = 0; i < padding; i++) {
        paddedData.push_back(static_cast<unsigned char>(padding));
    }

    std::vector<unsigned char> encrypted(paddedData.size());
    std::string modeUpper = mode;
    std::transform(modeUpper.begin(), modeUpper.end(), modeUpper.begin(), ::toupper);

    mbedtls_aes_setkey_enc(&ctx, reinterpret_cast<const unsigned char*>(key.data()), 256);

    if (modeUpper == "ECB") {
        for (size_t i = 0; i < paddedData.size(); i += BLOCK_SIZE) {
            mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT,
                &paddedData[i], &encrypted[i]);
        }
    }
    else if (modeUpper == "CBC") {
        mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, paddedData.size(),
            ivBytes.data(), paddedData.data(), encrypted.data());
    }
    else if (modeUpper == "CTR") {
        size_t nc_off = 0;
        unsigned char stream_block[16];
        mbedtls_aes_crypt_ctr(&ctx, paddedData.size(), &nc_off, ivBytes.data(),
            stream_block, paddedData.data(), encrypted.data());
    }
    else if (modeUpper == "CFB") {
        mbedtls_aes_crypt_cfb128(&ctx, MBEDTLS_AES_ENCRYPT, paddedData.size(), 0,
            ivBytes.data(), paddedData.data(), encrypted.data());
    }
    else if (modeUpper == "OFB") {
        mbedtls_aes_crypt_ofb(&ctx, paddedData.size(), 0, ivBytes.data(),
            paddedData.data(), encrypted.data());
    }
    else {
        return {};
    }

    std::string encryptedStr(encrypted.begin(), encrypted.end());

    return { encryptedStr, ivStr };
}

std::string AES::decrypt(const std::string& data,
    const std::string& key,
    const std::string& iv,
    const std::string& mode) {

    std::string decodedData = base64_decode(data);
    std::string decodedIv = base64_decode(iv);
    if (decodedIv.size() != BLOCK_SIZE) return "";
    if (decodedData.size() % BLOCK_SIZE != 0) return "";

    std::vector<unsigned char> encrypted(decodedData.begin(), decodedData.end());
    std::vector<unsigned char> ivBytes(decodedIv.begin(), decodedIv.end());
    std::vector<unsigned char> decrypted(encrypted.size());

    std::string modeUpper = mode;
    std::transform(modeUpper.begin(), modeUpper.end(), modeUpper.begin(), ::toupper);

    mbedtls_aes_setkey_dec(&ctx, reinterpret_cast<const unsigned char*>(key.data()), 256);

    if (modeUpper == "ECB") {
        for (size_t i = 0; i < encrypted.size(); i += BLOCK_SIZE) {
            mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_DECRYPT,
                &encrypted[i], &decrypted[i]);
        }
    }
    else if (modeUpper == "CBC") {
        mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, encrypted.size(),
            ivBytes.data(), encrypted.data(), decrypted.data());
    }
    else if (modeUpper == "CTR") {
        size_t nc_off = 0;
        unsigned char stream_block[16];
        mbedtls_aes_crypt_ctr(&ctx, encrypted.size(), &nc_off, ivBytes.data(),
            stream_block, encrypted.data(), decrypted.data());
    }
    else if (modeUpper == "CFB") {
        mbedtls_aes_crypt_cfb128(&ctx, MBEDTLS_AES_DECRYPT, encrypted.size(), 0,
            ivBytes.data(), encrypted.data(), decrypted.data());
    }
    else if (modeUpper == "OFB") {
        mbedtls_aes_crypt_ofb(&ctx, encrypted.size(), 0, ivBytes.data(),
            encrypted.data(), decrypted.data());
    }
    else {
        return "";
    }

    size_t padding = decrypted.back();
    if (padding <= BLOCK_SIZE) {
        decrypted.resize(decrypted.size() - padding);
    }

    return std::string(decrypted.begin(), decrypted.end());
}