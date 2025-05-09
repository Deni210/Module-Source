#pragma once

#include <lualib.h>

#include "Libraries/Cache.h"
#include "Libraries/Closure.h"
#include "Libraries/Crypt.h"
#include "Libraries/Drawing.h"
#include "Libraries/Debug.h"
#include "Libraries/Filesystem.h"
#include "Libraries/Input.h"
#include "Libraries/Instances.h"
#include "Libraries/Metatable.h"
#include "Libraries/Miscellaneous.h"
#include "Libraries/Scripts.h"
#include "Libraries/Console.h"
#include "Libraries/Websocket.h"

std::vector<const char*> dangerousFunctions =
{
	"OpenVideosFolder", "OpenScreenshotsFolder",
	"GetRobuxBalance", "PerformPurchase", "PromptBundlePurchase", "PromptNativePurchase",
	"PromptProductPurchase", "PromptPurchase", "PromptThirdPartyPurchase", "Publish",
	"GetMessageId", "OpenBrowserWindow", "RequestInternal", "ExecuteJavaScript",
	"openvideosfolder", "openscreenshotsfolder", "getrobuxbalance", "performpurchase",
	"promptbundlepurchase", "promptnativepurchase", "promptproductpurchase",
	"promptpurchase", "promptthirdpartypurchase", "publish", "getmessageid",
	"openbrowserwindow", "requestinternal", "executejavascript", "openVideosFolder",
	"openScreenshotsFolder", "getRobuxBalance", "performPurchase", "promptBundlePurchase",
	"promptNativePurchase", "promptProductPurchase", "promptPurchase",
	"promptThirdPartyPurchase", "publish", "getMessageId", "openBrowserWindow",
	"requestInternal", "executeJavaScript",
	"ToggleRecording", "TakeScreenshot", "HttpRequestAsync", "GetLast",
	"SendCommand", "GetAsync", "GetAsyncFullUrl", "RequestAsync", "MakeRequest",
	"togglerecording", "takescreenshot", "httprequestasync", "getlast",
	"sendcommand", "getasync", "getasyncfullurl", "requestasync", "makerequest",
	"toggleRecording", "takeScreenshot", "httpRequestAsync", "getLast",
	"sendCommand", "getAsync", "getAsyncFullUrl", "requestAsync", "makeRequest", "AddCoreScriptLocal",
	"SaveScriptProfilingData", "GetUserSubscriptionDetailsInternalAsync",
	"GetUserSubscriptionStatusAsync", "PerformBulkPurchase", "PerformCancelSubscription",
	"PerformPurchaseV2", "PerformSubscriptionPurchase", "PerformSubscriptionPurchaseV2",
	"PrepareCollectiblesPurchase", "PromptBulkPurchase", "PromptCancelSubscription",
	"PromptCollectiblesPurchase", "PromptGamePassPurchase", "PromptNativePurchaseWithLocalPlayer",
	"PromptPremiumPurchase", "PromptRobloxPurchase", "PromptSubscriptionPurchase",
	"ReportAbuse", "ReportAbuseV3", "ReturnToJavaScript", "OpenNativeOverlay",
	"OpenWeChatAuthWindow", "EmitHybridEvent", "OpenUrl", "PostAsync", "PostAsyncFullUrl",
	"RequestLimitedAsync", "Run", "Load", "CaptureScreenshot", "CreatePostAsync",
	"DeleteCapture", "DeleteCapturesAsync", "GetCaptureFilePathAsync", "SaveCaptureToExternalStorage",
	"SaveCapturesToExternalStorageAsync", "GetCaptureUploadDataAsync", "RetrieveCaptures",
	"SaveScreenshotCapture", "Call", "GetProtocolMethodRequestMessageId",
	"GetProtocolMethodResponseMessageId", "PublishProtocolMethodRequest",
	"PublishProtocolMethodResponse", "Subscribe", "SubscribeToProtocolMethodRequest",
	"SubscribeToProtocolMethodResponse", "GetDeviceIntegrityToken", "GetDeviceIntegrityTokenYield",
	"NoPromptCreateOutfit", "NoPromptDeleteOutfit", "NoPromptRenameOutfit", "NoPromptSaveAvatar",
	"NoPromptSaveAvatarThumbnailCustomization", "NoPromptSetFavorite", "NoPromptUpdateOutfit",
	"PerformCreateOutfitWithDescription", "PerformRenameOutfit", "PerformSaveAvatarWithDescription",
	"PerformSetFavorite", "PerformUpdateOutfit", "PromptCreateOutfit", "PromptDeleteOutfit",
	"PromptRenameOutfit", "PromptSaveAvatar", "PromptSetFavorite", "PromptUpdateOutfit"
};

namespace Scheduler {
	static std::vector<std::string> coderun_queue;

	int hookcaller(lua_State* L) {
		//SetFunction("scheduler::hookcaller");
		if (!coderun_queue.empty()) {
			std::string Script = coderun_queue.front();
			coderun_queue.erase(coderun_queue.begin());

			if (!Script.empty())
				Execution::Execute(L, Script);
		}

		return 0;
	}
}

int require(lua_State* L) {
	SetFunction("general::require");
	int type = lua_type(L, 1);
	if (type != LUA_TUSERDATA && type != LUA_TLIGHTUSERDATA) {
		lua_pushnil(L);
		return 1;
	}

	uintptr_t script = *(uintptr_t*)lua_touserdata(L, 1);

	if (!CheckMemory(script) || !CheckMemory(*(uintptr_t*)(script + Offsets::Instance::ClassDescriptor))) {
		lua_pushnil(L);
		return 1;
	}

	const char* className = *(const char**)(*(uintptr_t*)(script + Offsets::Instance::ClassDescriptor) + Offsets::Instance::ClassName);

	if (strcmp(className, "ModuleScript") == 0) {
		BYTE oldValue = *(BYTE*)(script + 0x70);
		*(BYTE*)(script + 0x70) = 0;

		lua_getglobal(L, "oldrequire");
		lua_pushvalue(L, 1);
		lua_call(L, 1, 1);

		*(BYTE*)(script + 0x70) = oldValue;

		return 1;
	}

	lua_getglobal(L, "oldrequire");
	lua_pushvalue(L, 1);
	lua_call(L, 1, 1);

	return 1;
}

static const struct luaL_Reg executorlibrary[] = {
	// Cache
	{"cache_invalidate", Cache::invalidate},
	{"cache_iscached", Cache::iscached},
	{"cache_replace", Cache::replace},
	{"cloneref", Cache::cloneref},
	{"compareinstances", Cache::compareinstances},

	// Closure
	{"checkcaller", ClosureLib::checkcaller},
	{"clonefunction", ClosureLib::clonefunction},
	{"getcallingscript", ClosureLib::getcallingscript},
	{"getscriptclosure", Scripts::getscriptclosure},
	{"iscclosure", ClosureLib::iscclosure},
	{"islclosure", ClosureLib::islclosure},
	{"isexecutorclosure", ClosureLib::isexecutorclosure},
	{"checkclosure", ClosureLib::isexecutorclosure},
	{"isourclosure", ClosureLib::isexecutorclosure},
	{"isnewcclosure", ClosureLib::isnewcclosure},
	{"loadstring", ClosureLib::loadstring},
	{"newcclosure", ClosureLib::newcclosure},
	{"hookfunction", ClosureLib::hookfunction},
    {"hookmetamethod", ClosureLib::hookmetamethod},

	{"replaceclosure", ClosureLib::hookfunction},
	{"hookfunc", ClosureLib::hookfunction},

	// Crypt
	{"base64encode", Crypt::base64encode},
	{"base64_encode", Crypt::base64encode},
	{"base64decode", Crypt::base64decode},
	{"base64_decode", Crypt::base64decode},
	//{"crypt_encrypt", Crypt::encrypt},
	//{"crypt_decrypt", Crypt::decrypt},
	{"crypt_generatebytes", Crypt::generatebytes},
	{"crypt_generatekey", Crypt::generatekey},
	{"crypt_hash", Crypt::hash},

	// Debug
	{"getconstant", Debug::getconstant},
	{"getconstants", Debug::getconstants},
	{"getinfo", Debug::getinfo},
	{"getproto", Debug::getproto},
	{"getprotos", Debug::getprotos},
	{"getstack", Debug::getstack},
	{"getupvalue", Debug::getupvalue},
	{"getupvalues", Debug::getupvalues},
	{"setconstant", Debug::setconstant},
	{"setstack", Debug::setstack},
	{"setupvalue", Debug::_setupvalue},

	// Filesystem
	{"readfile", Filesystem::readfile},
	{"listfiles", Filesystem::listfiles},
	{"writefile", Filesystem::writefile},
	{"makefolder", Filesystem::makefolder},
	{"appendfile", Filesystem::appendfile},
	{"isfile", Filesystem::isfile},
	{"isfolder", Filesystem::isfolder},
	{"delfolder", Filesystem::delfolder},
	{"delfile", Filesystem::delfile},
	{"loadfile", Filesystem::loadfile},
	{"dofile", Filesystem::dofile},
	{"getcustomasset", Filesystem::getcustomasset},

	// Input
	{"isrbxactive", Input::isrbxactive},
	{"isgameactive", Input::isrbxactive},
	{"iswindowactive", Input::isrbxactive},
	{"mouse1click", Input::mouse1click},
	{"mouse1press", Input::mouse1press},
	{"mouse1release", Input::mouse1release},
	{"mouse2click", Input::mouse2click},
	{"mouse2press", Input::mouse2press},
	{"mouse2release", Input::mouse2release},
	{"mousemoveabs", Input::mousemoveabs},
	{"mousemoverel", Input::mousemoverel},
	{"mousescroll", Input::mousescroll},
	{"iskeydown", Input::iskeydown},
	{"keypress", Input::keypress},
	{"keyrelease", Input::keyrelease},

	// Instances
	{"getcallbackvalue", Instances::getcallbackvalue},
	{"gethui", Instances::gethui},
	{"getinstances", Instances::getinstances},
	{"getnilinstances", Instances::getnilinstances},
	{"getscripts", Instances::getscripts},
	{"getrunningscripts", Instances::getrunningscripts},
	{"getloadedmodules", Instances::getloadedmodules},
	{"isscriptable", Instances::isscriptable},
	{"setscriptable", Instances::setscriptable},
	{"sethiddenproperty", Instances::sethiddenproperty},
	{"gethiddenproperty", Instances::gethiddenproperty}, // needs fixing
	{"fireclickdetector", Instances::fireclickdetector},
	{"firetouchinterest", Instances::firetouchinterest},
	{"fireproximityprompt", Instances::fireproximityprompt},
	//{"getconnections", Instances::getconnections},
	{"firesignal", Instances::firesignal},
	//{"replicatesignal", Instances::replicatesignal},

	// Metatable
	{"getrawmetatable", Metatable::getrawmetatable},
	{"getnamecallmethod", Metatable::getnamecallmethod},
	{"isreadonly", Metatable::isreadonly},
	{"setrawmetatable", Metatable::setrawmetatable},
	{"setreadonly", Metatable::setreadonly},
	{"makereadonly", Metatable::makereadonly},
	{"makewriteable", Metatable::makewriteable},

	// Miscellaneous
	{"identifyexecutor", Miscellaneous::identifyexecutor},
	{"getexecutorname", Miscellaneous::identifyexecutor},
	{"lz4compress", Miscellaneous::lz4compress},
	{"lz4decompress", Miscellaneous::lz4decompress},
	{"messagebox", Miscellaneous::messagebox},
	//{"queue_on_teleport", Miscellaneous::queueonteleport},
	//{"queueonteleport", Miscellaneous::queueonteleport},
	{"request", Miscellaneous::request},
	{"HttpGet", Miscellaneous::httpget},
	{"HttpRequest", Miscellaneous::httpget},
	{"GetObjects", Miscellaneous::getobjects},
	{"http_request", Miscellaneous::request},
	{"setclipboard", Miscellaneous::setclipboard},
	{"toclipboard", Miscellaneous::setclipboard},
	{"setrbxclipboard", Miscellaneous::setclipboard},
	{"setfpscap", Miscellaneous::setfpscap},
	{"getfpscap", Miscellaneous::getfpscap},
	{"checkparallel", Miscellaneous::checkparallel},
	{"isparallel", Miscellaneous::checkparallel},
	{"info", Miscellaneous::info},

	// Scripts
	{"getgc", Scripts::getgc}, 
	{"getgenv", Scripts::getgenv},
	{"getreg", Scripts::getreg},
	{"getsenv", Scripts::getsenv},
	{"getrenv", Scripts::getrenv},
	{"getscriptbytecode", Scripts::getscriptbytecode},
	{"getscripthash", Scripts::getscripthash}, 
	{"getfunctionhash", Scripts::getfunctionhash},
	{"getfunctionbytecode", Scripts::getfunctionhash},
	{"getscriptclosure", Scripts::getscriptclosure},
	{"dumpstring", Scripts::getscriptbytecode},
	{"getthreadidentity", Scripts::getthreadidentity},
	{"getthreadcontext", Scripts::getthreadidentity},
	{"getidentity", Scripts::getthreadidentity},
	{"setthreadidentity", Scripts::setthreadidentity},
	{"setthreadcontext", Scripts::setthreadidentity},
	{"setidentity", Scripts::setthreadidentity},
	{"decompile", Scripts::decompile},

	// Console
	{"consolecreate", ConsoleLib::rconsolecreate}, {"rconsolecreate", ConsoleLib::rconsolecreate},
    {"consoleprint", ConsoleLib::rconsoleprint}, {"rconsoleprint", ConsoleLib::rconsoleprint},
    {"consoleclear", ConsoleLib::rconsoleclear}, {"rconsoleclear", ConsoleLib::rconsoleclear},
    {"consolesettitle", ConsoleLib::rconsolesettitle}, {"rconsolesettitle", ConsoleLib::rconsolesettitle},
    {"consolename", ConsoleLib::rconsolesettitle}, {"rconsolename", ConsoleLib::rconsolesettitle},
    {"consoledestroy", ConsoleLib::rconsoledestroy}, {"rconsoledestroy", ConsoleLib::rconsoledestroy},
    {"rconsoleinput", ConsoleLib::rconsoleinput}, {"consoleinput", ConsoleLib::rconsoleinput},
    {"rconsoletopmost", ConsoleLib::rconsoletopmost}, {"consoletopmost", ConsoleLib::rconsoletopmost},
    {"rconsoleinfo", ConsoleLib::rconsoleinfo}, {"consoleinfo", ConsoleLib::rconsoleinfo},
    {"rconsolewarn", ConsoleLib::rconsolewarn}, {"consolewarn", ConsoleLib::rconsolewarn},
    {"rconsoleerr", ConsoleLib::rconsoleerr}, {"consoleerr", ConsoleLib::rconsoleerr},
    {"rconsolehidden", ConsoleLib::rconsolehidden}, {"consoleerr", ConsoleLib::rconsolehidden},
    {"rconsolehide", ConsoleLib::rconsolehide}, {"consolehide", ConsoleLib::rconsolehide},
    {"rconsoleshow", ConsoleLib::rconsoleshow}, {"consoleshow", ConsoleLib::rconsoleshow},
    {"rconsoletoggle", ConsoleLib::rconsoletoggle}, {"consoletoggle", ConsoleLib::rconsoletoggle},

	// Websocket
	{"websocket_connect", Websocket::connect},

	{"require", require},

	{"HookCaller", Scheduler::hookcaller},

	{NULL, NULL},
};

lua_CFunction oldNameCall;
lua_CFunction oldIndex;

int nameCallHook(lua_State* L) {
	if (!L)
		return 0;

	if (L->userdata->Identity >= 8 && L->userdata->Script.expired()) {
		const char* data = L->namecall->data;

		if (!data || !CheckMemory((uintptr_t)data) || !*data) return 0;

		if (strcmp(data, "HttpGet") == 0 || strcmp(data, "HttpGetAsync") == 0) {
			return Miscellaneous::httpget(L);
		}

		if (strcmp(data, "GetObjects") == 0 || strcmp(data, "GetObjectsAsync") == 0) {
			return Miscellaneous::getobjects(L);
		}

		for (const std::string& func : dangerousFunctions) {
			if (std::string(data) == func) {
				luaL_error(L, "Function has been disabled for security reasons.");
				return 0;
			}
		}
	}

	if (!oldNameCall)
		return 0;

	return oldNameCall(L);
}

int indexHook(lua_State* L) {
	if (!L)
		return 0;

	if (L->userdata->Identity >= 8 && L->userdata->Script.expired()) {
		const char* data = lua_tostring(L, 2);

		if (!data || !CheckMemory((uintptr_t)data) || !*data) return 0;

		if (strcmp(data, "HttpGet") == 0 || strcmp(data, "HttpGetAsync") == 0) {
			lua_getglobal(L, "HttpGet");
			return 1;
		}

		if (strcmp(data, "GetObjects") == 0 || strcmp(data, "GetObjectsAsync") == 0) {
			lua_getglobal(L, "GetObjects");
			return 1;
		}


		for (const std::string& func : dangerousFunctions) {
			if (std::string(data) == func) {
				luaL_error(L, "Function has been disabled for security reasons.");
				return 0;
			}
		}
	}

	if (!oldIndex)
		return 0;

	return oldIndex(L);
}

void InitializeHooks(lua_State* L) {
	int originalCount = lua_gettop(L);

	lua_getglobal(L, "game");
	luaL_getmetafield(L, -1, "__index");

	Closure* index_closure = clvalue(luaA_toobject(L, -1));
	oldIndex = index_closure->c.f;
	index_closure->c.f = indexHook;

	lua_pop(L, 1);

	luaL_getmetafield(L, -1, "__namecall");

	Closure* namecall_closure = clvalue(luaA_toobject(L, -1));
	oldNameCall = namecall_closure->c.f;
	namecall_closure->c.f = nameCallHook;

	lua_settop(L, originalCount);
}

namespace Environment {
	void Initialize(lua_State* L) {
		lua_newtable(L);
		lua_setglobal(L, "_G");

		lua_newtable(L);
		lua_setglobal(L, "shared");

		lua_newtable(L);
		lua_setglobal(L, "crypt");

		lua_newtable(L);
		lua_setglobal(L, "cache");

		lua_newtable(L);
		lua_setglobal(L, "http");

		lua_getglobal(L, "require");
		lua_setglobal(L, "oldrequire");

		SetFunction("dr");

		lua_pushvalue(L, LUA_GLOBALSINDEX);

		SetFunction("psh");

		const luaL_Reg* i = executorlibrary;

		for (; i->name; i++)
		{
			lua_pushcfunction(L, i->func, i->name);
			lua_setfield(L, -2, i->name);
		}


		SetFunction("reg");
		lua_pop(L, 1);

		SetFunction("lib");

		lua_getglobal(L, "game");
		lua_getfield(L, -1, "GetService");
		lua_pushvalue(L, -2);
		lua_pushstring(L, "CoreGui");
		lua_call(L, 2, 1);

		lua_getglobal(L, "cloneref");
		lua_insert(L, -2);
		lua_call(L, 1, 1);

		SetFunction("hiddenui");

		lua_setglobal(L, "__hiddenui");

		lua_newtable(L);
		lua_setglobal(L, "Drawing");

		lua_newtable(L);
		lua_setglobal(L, "cleardrawcache");

		lua_newtable(L);
		lua_setglobal(L, "setrenderproperty");

		lua_newtable(L);
		lua_setglobal(L, "getrenderproperty");

		lua_newtable(L);
		lua_setglobal(L, "isrenderobj");

		SetFunction("init hooks now");

		InitializeHooks(L);
	}
}