#ifndef AES_MBED_H
#define AES_MBED_H

#include <string>
#include <mbedtls/aes.h> // Install with vcpkg cg

class AES {
private:
    mbedtls_aes_context ctx;
    static const size_t BLOCK_SIZE = 32; // AES-256 (CG's generatekey is 256 bits)

public:
    AES();
    ~AES();
    std::pair<std::string, std::string> encrypt(const std::string& data,
        const std::string& key,
        const std::string& iv = "",
        const std::string& mode = "CBC");
    std::string decrypt(const std::string& data,
        const std::string& key,
        const std::string& iv,
        const std::string& mode = "CBC");
};

#endif