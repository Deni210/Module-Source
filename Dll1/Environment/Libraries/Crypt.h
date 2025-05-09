#pragma once

#include <base64.h>
#include <md5.h>
#include <sha1.h>
#include <sha224.h>
#include <sha256.h>
#include <sha384.h>
#include <sha512.h>
#include <sha3_224.h>
#include <sha3_256.h>
#include <sha3_384.h>
#include <sha3_512.h>
#include <map>
#include <AES.h>

inline std::string random_string(std::size_t length) {
	auto randchar = []() -> char {
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const std::size_t max_index = (sizeof(charset) - 1);
		return charset[rand() % max_index];
		};

	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
};

namespace Crypt {

	int base64encode(lua_State* L) {
		SetFunction("crypt::base64encode");
		luaL_checktype(L, 1, LUA_TSTRING);

		const char* str = luaL_checklstring(L, 1, nullptr);
		bool urlEncoding = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : false;

		std::string encoded = base64_encode(str, urlEncoding);
		lua_pushlstring(L, encoded.data(), encoded.size());
		return 1;
	}

	int base64decode(lua_State* L) {
		SetFunction("crypt::base64decode");
		luaL_checktype(L, 1, LUA_TSTRING);

		const char* str = luaL_checklstring(L, 1, nullptr);
		bool urlEncoding = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : false;

		std::string decoded = base64_decode(str, urlEncoding);
		lua_pushlstring(L, decoded.data(), decoded.size());
		return 1;
	}

	int generatekey(lua_State* L) {
		SetFunction("crypt::generatekey");
		std::string generatedString = random_string(32);
		std::string encodedKey = base64_encode(generatedString, false);

		lua_pushlstring(L, encodedKey.data(), encodedKey.size());
		return 1;
	}

	int generatebytes(lua_State* L) {
		SetFunction("crypt::generatebytes");
		luaL_checktype(L, 1, LUA_TNUMBER);

		size_t length = luaL_checkinteger(L, 1);

		std::string generatedString = random_string(length);
		std::string encodedBytes = base64_encode(generatedString, false);

		lua_pushlstring(L, encodedBytes.data(), encodedBytes.size());
		return 1;
	}

	enum CryptHashModes
	{
		H_SHA1,
		H_SHA224,
		H_SHA256,
		H_SHA384,
		H_SHA512,
		H_SHA3_224,
		H_SHA3_256,
		H_SHA3_384,
		H_SHA3_512,
		H_MD5
	};

	std::map<std::string, CryptHashModes> CryptHashTranslationMap = {
		{ "sha1", H_SHA1 },
		{ "sha224", H_SHA224 },
		{ "sha256", H_SHA256 },
		{ "sha384", H_SHA384 },
		{ "sha512", H_SHA512 },
		{ "sha3-224", H_SHA3_224 },
		{ "sha3-256", H_SHA3_256 }, // incorrect
		{ "sha3-384", H_SHA3_384 }, // incorrect
		{ "sha3-512", H_SHA3_512 }, // incorrect
		{ "md5", H_MD5 }
	};

	enum CryptModes
	{
		//AES
		AES_CBC,
		AES_CFB,
		AES_CTR,
		AES_OFB,
		AES_GCM,
		AES_EAX,

		//Blowfish
		BF_CBC,
		BF_CFB,
		BF_OFB
	};

	std::map<std::string, CryptModes> CryptTranslationMap =
	{
		{ "cbc", AES_CBC },

		{ "cfb", AES_CFB },

		{ "ctr", AES_CTR },

		{ "ofb", AES_OFB },

		{ "gcm", AES_GCM },

		{ "eax", AES_EAX },

		{ "blowfish-cbc", BF_CBC },
		{ "bf-cbc", BF_CBC },

		{ "blowfish-cfb", BF_CFB },
		{ "bf_cfb", BF_CFB },

		{ "blowfish-ofb", BF_OFB },
		{ "bf_ofb", BF_OFB },
	};

	int hash(lua_State* L) {
		SetFunction("crypt::hash");
		luaL_checktype(L, 1, LUA_TSTRING);

		std::string Message = lua_tostring(L, 1);
		std::string HashAlg = luaL_optstring(L, 2, "");

		if (Message.empty())
			luaL_argerror(L, 1, "String cannot be empty");

		if (HashAlg.empty())
			luaL_argerror(L, 2, "String cannot be empty");

		std::transform(HashAlg.begin(), HashAlg.end(), HashAlg.begin(), ::tolower);

		if (CryptHashTranslationMap.find(HashAlg) == CryptHashTranslationMap.end())
			luaL_argerror(L, 2, "Invalid algoritm");

		CryptHashModes hashingMode = CryptHashTranslationMap[HashAlg];
		std::string hashed;
		switch (hashingMode) {
		case H_SHA1: {
			hashed = SHA1::hash(Message);

			lua_pushlstring(L, hashed.data(), hashed.size());
			return 1;
		}
		case H_SHA224: {
			hashed = SHA224::hash(Message);

			lua_pushlstring(L, hashed.data(), hashed.size());
			return 1;
		}
		case H_SHA256: {
			hashed = SHA256::hash(Message);

			lua_pushlstring(L, hashed.data(), hashed.size());
			return 1;
		}
		case H_SHA384: {
			hashed = sha384(Message);

			lua_pushlstring(L, hashed.data(), hashed.size());

			return 1;
		}
		case H_SHA512: {
			hashed = sha512(Message);

			lua_pushlstring(L, hashed.data(), hashed.size());
			return 1;
		}
		case H_SHA3_224: {
			hashed = sha3_224::sha3_224_hash(Message);

			lua_pushlstring(L, hashed.data(), hashed.size());
			return 1;
		}
		case H_SHA3_256: {
			hashed = sha3_256::sha3_256_hash(Message);

			lua_pushlstring(L, hashed.data(), hashed.size());
			return 1;
		}
		case H_SHA3_384: {
			hashed = sha3_384::sha3_384_hash(Message);

			lua_pushlstring(L, hashed.data(), hashed.size());
			return 1;
		}
		case H_SHA3_512: {
			hashed = sha3_512::sha3_512_hash(Message);

			lua_pushlstring(L, hashed.data(), hashed.size());
			return 1;
		}
		case H_MD5: {
			hashed = md5(Message);

			lua_pushlstring(L, hashed.data(), hashed.size());
			return 1;
		}
		default: {
			luaL_error(L, "Invalid algorithm");
			return 0;
		};
		}

		return 0;
	};

	int encrypt(lua_State* L) {
		SetFunction("crypt::encrypt");
		luaL_checktype(L, 1, LUA_TSTRING);
		luaL_checktype(L, 2, LUA_TSTRING);

		std::string MessageText = luaL_optstring(L, 1, "");
		std::string keyText = luaL_optstring(L, 2, "");
		std::string IVText = luaL_optstring(L, 3, "");
		std::string AlgText = luaL_optstring(L, 4, "CBC");

		if (MessageText.empty())
			luaL_argerror(L, 1, "String cannot be empty");

		if (keyText.empty())
			luaL_argerror(L, 2, "String cannot be empty");

		std::transform(AlgText.begin(), AlgText.end(), AlgText.begin(), ::tolower);

		if (CryptTranslationMap.find(AlgText) == CryptTranslationMap.end())
			luaL_error(L, "Invalid algorithm");

		std::string decodedKey = base64_decode(keyText);
		if (decodedKey.size() != 32) {
			luaL_argerror(L, 2, "Key must be 32 bytes after decoding");
		}

		CryptModes Mode = CryptTranslationMap[AlgText];
		AES aes;
		std::pair<std::string, std::string> result;

		try {
			switch (Mode) {
			case AES_CBC:
				result = aes.encrypt(MessageText, decodedKey, IVText, "CBC");
				break;
			case AES_CFB:
				result = aes.encrypt(MessageText, decodedKey, IVText, "CFB");
				break;
			case AES_CTR:
				result = aes.encrypt(MessageText, decodedKey, IVText, "CTR");
				break;
			case AES_OFB:
				result = aes.encrypt(MessageText, decodedKey, IVText, "OFB");
				break;
			case AES_EAX:
				luaL_error(L, "EAX mode not supported");
				return 0;
			case BF_CBC:
			case BF_CFB:
			case BF_OFB:
				luaL_error(L, "Blowfish modes not supported");
				return 0;
			default:
				luaL_argerror(L, 4, "Invalid algorithm");
			}

			lua_pushstring(L, base64_encode(result.first).c_str());
			lua_pushstring(L, base64_encode(result.second).c_str());
			return 2;
		}
		catch (const std::exception& e) {
			luaL_error(L, e.what());
			return 0;
		}
	}

	int decrypt(lua_State* L) {
		SetFunction("crypt::decrypt");
		luaL_checktype(L, 1, LUA_TSTRING);
		luaL_checktype(L, 2, LUA_TSTRING);
		luaL_checktype(L, 3, LUA_TSTRING);

		std::string encryptedText = luaL_optstring(L, 1, "");
		std::string keyText = luaL_optstring(L, 2, "");
		std::string IVText = luaL_optstring(L, 3, "");
		std::string AlgText = luaL_optstring(L, 4, "CBC");

		if (encryptedText.empty())
			luaL_argerror(L, 1, "String cannot be empty");
		if (keyText.empty())
			luaL_argerror(L, 2, "String cannot be empty");
		if (IVText.empty())
			luaL_argerror(L, 3, "String cannot be empty");

		std::transform(AlgText.begin(), AlgText.end(), AlgText.begin(), ::tolower);
		if (CryptTranslationMap.find(AlgText) == CryptTranslationMap.end())
			luaL_error(L, "Invalid algorithm");

		std::string decodedKey = base64_decode(keyText);
		std::string decodedEncryptedText = base64_decode(encryptedText);
		std::string decodedIV = base64_decode(IVText);

		if (decodedKey.size() != 32) {
			luaL_argerror(L, 2, "Key must be 32 bytes after decoding");
		}

		CryptModes Mode = CryptTranslationMap[AlgText];
		AES aes;
		std::string decryptedText;

		try {
			switch (Mode) {
			case AES_CBC:
				decryptedText = aes.decrypt(decodedEncryptedText, decodedKey, decodedIV, "CBC");
				break;
			case AES_CFB:
				decryptedText = aes.decrypt(decodedEncryptedText, decodedKey, decodedIV, "CFB");
				break;
			case AES_CTR:
				decryptedText = aes.decrypt(decodedEncryptedText, decodedKey, decodedIV, "CTR");
				break;
			case AES_OFB:
				decryptedText = aes.decrypt(decodedEncryptedText, decodedKey, decodedIV, "OFB");
				break;
			case AES_EAX:
				luaL_error(L, "EAX mode not supported");
				return 0;
			case BF_CBC:
			case BF_CFB:
			case BF_OFB:
				luaL_error(L, "Blowfish modes not supported");
				return 0;
			default:
				luaL_argerror(L, 4, "Invalid algorithm");
			}

			lua_pushstring(L, decryptedText.c_str());
			return 1;
		}
		catch (const std::exception& e) {
			luaL_error(L, e.what());
			return 0;
		}
	}
}