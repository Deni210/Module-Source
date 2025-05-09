#pragma once

#pragma comment(lib, "shlwapi.lib")

#include "../Execution/Execution.hpp"

#include <lstate.h>
#include <lz4.h>
#include <lualib.h>
#include <lapi.h>
#include <filesystem>
#include <shlwapi.h>
#include <fstream>
#include <future>

std::vector<std::string> disallowedExtensions =
{
	".exe", ".scr", ".bat", ".com", ".csh", ".msi", ".vb", ".vbs", ".vbe", ".ws", ".wsf", ".wsh", ".ps1"
};

bool equals_ignore_case(const std::string& a, const std::string& b)
{
	return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) { return tolower(a) == tolower(b); });
}

namespace Filesystem {
	std::filesystem::path localAppdata = getenv("LOCALAPPDATA");
	std::filesystem::path realLibrary = localAppdata / "RealLibrary";
	std::filesystem::path workspace = realLibrary / "Workspace";

	int readfile(lua_State* L) {
		SetFunction("filesystem::readfile");
		luaL_checktype(L, 1, LUA_TSTRING);

		std::string path = lua_tostring(L, 1);
		std::replace(path.begin(), path.end(), '/', '\\');
		if (path.find("..") != std::string::npos) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "attempt to escape directory");
			lua_call(L, 1, 0);
			return 0;
		}

		std::filesystem::path workspacePath = workspace / path;
		const std::string extension = PathFindExtension(path.data());

		FILE* file = fopen(workspacePath.string().c_str(), "rb");
		if (!file) {
			luaL_error(L, "file does not exist!");
			return 0;
		}

		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);
		rewind(file);

		std::string content(fileSize, '\0');
		size_t bytesread = fread(&content[0], 1, fileSize, file);
		fclose(file);

		lua_pushstring(L, content.data());

		return 1;
	}

	int listfiles(lua_State* L) {
		SetFunction("filesystem::listfiles");
		luaL_checktype(L, 1, LUA_TSTRING);

		std::string path = lua_tostring(L, 1);
		std::replace(path.begin(), path.end(), '/', '\\');
		if (path.find("..") != std::string::npos) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "attempt to escape directory");
			lua_call(L, 1, 0);
			return 0;
		}

		std::filesystem::path workspacePath = workspace / path;
		if (!std::filesystem::exists(workspacePath)) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "directory doesn't exist!");
			lua_call(L, 1, 0);
			return 0;
		}

		int index = 0;
		lua_newtable(L);

		for (auto& file : std::filesystem::directory_iterator(workspacePath)) {
			auto filePath = file.path().string().substr(workspace.string().length() + 1);

			lua_pushinteger(L, ++index);
			lua_pushstring(L, filePath.data());
			lua_settable(L, -3);
		}

		return 1;
	}

	int writefile(lua_State* L) {
		luaL_checktype(L, 1, LUA_TSTRING);
		luaL_checktype(L, 2, LUA_TSTRING);

		std::string path = lua_tostring(L, 1);
		std::string data = lua_tostring(L, 2);

		std::replace(path.begin(), path.end(), '/', '\\');

		if (path.find("..") != std::string::npos) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "Attempt to escape directory");
			lua_call(L, 1, 0);
			return 0;
		}

		std::filesystem::path workspacePath = workspace / path;

		const std::string extension = PathFindExtension(path.c_str());
		for (const std::string& forbidden : disallowedExtensions) {
			if (equals_ignore_case(extension, forbidden)) {
				lua_getglobal(L, "warn");
				lua_pushstring(L, "Forbidden extension!");
				lua_call(L, 1, 0);
				return 0;
			}
		}

		const size_t maxPathLength = 260;
		if (workspacePath.string().length() > maxPathLength) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "Path length exceeds system limit!");
			lua_call(L, 1, 0);
			return 0;
		}

		std::ofstream file(workspacePath, std::ios::binary);
		if (!file) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "Failed to open file for writing!");
			lua_call(L, 1, 0);
			return 0;
		}

		const size_t chunkSize = 1024;
		size_t totalWritten = 0;
		while (totalWritten < data.size()) {
			size_t toWrite = std::min(chunkSize, data.size() - totalWritten);
			file.write(data.data() + totalWritten, toWrite);
			if (!file) {
				lua_getglobal(L, "warn");
				lua_pushstring(L, "Error writing to file!");
				lua_call(L, 1, 0);
				return 0;
			}
			totalWritten += toWrite;
		}

		return 0;
	}

	int makefolder(lua_State* L) {
		SetFunction("filesystem::makefolder");
		luaL_checktype(L, 1, LUA_TSTRING);

		std::string path = lua_tostring(L, 1);
		std::replace(path.begin(), path.end(), '/', '\\');
		if (path.find("..") != std::string::npos) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "attempt to escape directory");
			lua_call(L, 1, 0);
			return 0;
		}

		std::filesystem::path workspacePath = workspace / path;
		std::filesystem::create_directory(workspacePath);
		
		return 0;
	}

	int appendfile(lua_State* L) {
		SetFunction("filesystem::appendfile");
		luaL_checktype(L, 1, LUA_TSTRING);
		luaL_checktype(L, 2, LUA_TSTRING);

		std::string path = lua_tostring(L, 1);
		std::string data = lua_tostring(L, 2);

		std::replace(path.begin(), path.end(), '/', '\\');

		if (path.find("..") != std::string::npos) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "attempt to escape directory");
			lua_call(L, 1, 0);
			return 0;
		}

		std::filesystem::path workspacePath = workspace / path;

		std::string extension = PathFindExtension(path.c_str());

		for (const std::string& forbidden : disallowedExtensions) {
			if (equals_ignore_case(extension, forbidden)) {
				lua_getglobal(L, "warn");
				lua_pushstring(L, "forbidden extension!");
				lua_call(L, 1, 0);
				return 0;
			}
		}

		std::ofstream outFile(workspacePath, std::ios::app | std::ios::binary);
		if (!outFile.is_open()) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "failed to open file for appending");
			lua_call(L, 1, 0);
			return 0;
		}

		outFile.write(data.data(), data.size());

		if (!outFile) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "error while writing to file");
			lua_call(L, 1, 0);
		}

		outFile.close();
		return 0;
	}

	int isfile(lua_State* L) {
		SetFunction("filesystem::isfile");
		luaL_checktype(L, 1, LUA_TSTRING);

		std::string path = lua_tostring(L, 1);
		std::replace(path.begin(), path.end(), '/', '\\');
		if (path.find("..") != std::string::npos) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "attempt to escape directory");
			lua_call(L, 1, 0);
			return 0;
		}

		std::filesystem::path workspacePath = workspace / path;

		lua_pushboolean(L, std::filesystem::is_regular_file(workspacePath));

		return 1;
	}

	int isfolder(lua_State* L) {
		SetFunction("filesystem::isfolder");
		luaL_checktype(L, 1, LUA_TSTRING);

		std::string path = lua_tostring(L, 1);
		std::replace(path.begin(), path.end(), '/', '\\');
		if (path.find("..") != std::string::npos) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "attempt to escape directory");
			lua_call(L, 1, 0);
			return 0;
		}

		std::filesystem::path workspacePath = workspace / path;

		lua_pushboolean(L, std::filesystem::is_directory(workspacePath));

		return 1;
	}

	int delfolder(lua_State* L) {
		SetFunction("filesystem::delfolder");
		luaL_checktype(L, 1, LUA_TSTRING);

		std::string path = lua_tostring(L, 1);
		std::replace(path.begin(), path.end(), '/', '\\');
		if (path.find("..") != std::string::npos) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "attempt to escape directory");
			lua_call(L, 1, 0);
			return 0;
		}

		std::filesystem::path workspacePath = workspace / path;
		if (!std::filesystem::remove_all(workspacePath)) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "folder does not exist!");
			lua_call(L, 1, 0);
			return 0;
		}

		return 0;
	}

	int delfile(lua_State* L) {
		SetFunction("filesystem::delfile");
		luaL_checktype(L, 1, LUA_TSTRING);

		std::string path = lua_tostring(L, 1);
		std::replace(path.begin(), path.end(), '/', '\\');
		if (path.find("..") != std::string::npos) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "attempt to escape directory");
			lua_call(L, 1, 0);
			return 0;
		}

		std::filesystem::path workspacePath = workspace / path;
		if (!std::filesystem::remove(workspacePath)) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "file does not exist!");
			lua_call(L, 1, 0);
			return 0;
		}

		return 0;
	}

	int loadfile(lua_State* L) {
		SetFunction("filesystem::loadfile");
		luaL_checktype(L, 1, LUA_TSTRING);

		std::string path = lua_tostring(L, 1);
		const std::string chunkname = luaL_optstring(L, 2, "=");
		std::replace(path.begin(), path.end(), '/', '\\');
		if (path.find("..") != std::string::npos) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "attempt to escape directory");
			lua_call(L, 1, 0);
			return 0;
		}

		std::filesystem::path workspacePath = workspace / path;

		FILE* file = fopen(workspacePath.string().c_str(), "rb");
		if (!file) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "file does not exist!");
			lua_call(L, 1, 0);
			return 0;
		}

		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);
		rewind(file);

		std::string content(fileSize, '\0');
		size_t bytesread = fread(&content[0], 1, fileSize, file);
		fclose(file);

		std::string script = Execution::CompileScript(content);
		if (script[0] == '\0' || script.empty()) {
			lua_pushnil(L);
			lua_pushstring(L, "Failed to compile script");
			return 2;
		}

		int result = RBX::LuaVM__Load(L, &script, chunkname.data(), 0);
		if (result != LUA_OK) {
			std::string Error = luaL_checklstring(L, -1, nullptr);
			lua_pop(L, 1);

			lua_pushnil(L);
			lua_pushstring(L, Error.data());

			return 2;
		}

		Closure* closure = clvalue(luaA_toobject(L, -1));

		Execution::SetProtoCapabilities(closure->l.p);

		return 1;
	}

	int dofile(lua_State* L) {
		SetFunction("filesystem::dofile");
		luaL_checktype(L, 1, LUA_TSTRING);

		std::string path = lua_tostring(L, 1);
		std::replace(path.begin(), path.end(), '/', '\\');
		if (path.find("..") != std::string::npos) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "attempt to escape directory");
			lua_call(L, 1, 0);
			return 0;
		}

		std::filesystem::path workspacePath = workspace / path;

		FILE* file = fopen(workspacePath.string().c_str(), "rb");
		if (!file) {
			lua_getglobal(L, "warn");
			lua_pushstring(L, "file does not exist!");
			lua_call(L, 1, 0);
			return 0;
		}

		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);
		rewind(file);

		std::string content(fileSize, '\0');
		size_t bytesread = fread(&content[0], 1, fileSize, file);
		fclose(file);

		std::string script = Execution::CompileScript(content);
		if (script[0] == '\0' || script.empty()) {
			lua_pushnil(L);
			lua_pushstring(L, "Failed to compile script");
			return 2;
		}

		int result = RBX::LuaVM__Load(L, &script, "=", 0);
		if (result != LUA_OK) {
			std::string Error = luaL_checklstring(L, -1, nullptr);
			lua_pop(L, 1);

			lua_pushnil(L);
			lua_pushstring(L, Error.data());

			return 2;
		}

		Closure* closure = clvalue(luaA_toobject(L, -1));

		Execution::SetProtoCapabilities(closure->l.p);

		RBX::Task__Defer(L);

		return 0;
	}

	int getcustomasset(lua_State* L) {
		SetFunction("filesystem::getcustomasset");

		luaL_checktype(L, 1, LUA_TSTRING);
		const std::string FileName = lua_tostring(L, 1);

		const auto FilePath = workspace / FileName;
		if (!std::filesystem::exists(FilePath)) {
			luaL_error(L, "File not found: %s", FileName.c_str());
			return 0;
		}

		const auto SoundDir = std::filesystem::current_path() / "content" / "sounds";
		std::filesystem::create_directories(SoundDir);

		const auto AssetPath = SoundDir / FilePath.filename();

		const auto ContentSize = std::filesystem::file_size(FilePath);
		std::string Result;
		Result.resize(ContentSize);

		std::ifstream In(FilePath, std::ios::binary);
		In.read(Result.data(), ContentSize);
		In.close();

		std::ofstream Out(AssetPath, std::ios::binary);
		Out.write(Result.data(), Result.size());
		Out.close();

		const std::string SoundId = std::format("rbxasset://sounds/{}", AssetPath.filename().string());
		lua_pushstring(L, SoundId.c_str());
		return 1;
	}

}