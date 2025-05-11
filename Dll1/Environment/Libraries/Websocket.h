
#pragma once

#include <lstate.h>
#include <lualib.h>
#include <lapi.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <asio.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "../../Dependencies/easywsclient/easywsclient.hpp"

using easywsclient::WebSocket;
namespace asio = boost::asio;

namespace Websocket {
    class exploit_websocket {
    public:
        lua_State* th = nullptr;
        bool connected = false;
        WebSocket::pointer webSocket = nullptr;
        std::thread pollThread;
        std::atomic<bool> running = false;

        int onMessageRef;
        int onCloseRef;
        int threadRef;

        asio::io_context io_context;
        asio::ip::tcp::socket socket;
        SSL_CTX* ssl_ctx = nullptr;
        SSL* ssl = nullptr;

        void pollMessages() {
            while (running) {
                if (!webSocket || webSocket->getReadyState() != WebSocket::OPEN) {
                    fireClose();
                    break;
                }

                webSocket->poll(10);
                webSocket->dispatch([this](const std::string& message) {
                    fireMessage(message);
                });

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        void fireMessage(const std::string& message) {
            if (!connected || !th) {
                return;
            }

            lua_getref(th, onMessageRef);
            lua_getfield(th, -1, "Fire");
            lua_getref(th, onMessageRef);
            lua_pushlstring(th, message.c_str(), message.size());

            if (lua_pcall(th, 2, 0, 0) != LUA_OK) {
                lua_settop(th, 0);
                return;
            }

            lua_settop(th, 0);
        }

        void fireClose() {
            if (!connected || !th) {
                return;
            }

            connected = false;
            running = false;

            lua_getref(th, onCloseRef);
            lua_getfield(th, -1, "Fire");
            lua_getref(th, onCloseRef);
            if (lua_pcall(th, 1, 0, 0) != LUA_OK) {
                luaL_error(th, lua_tostring(th, -1));
                return;
            }
            lua_settop(th, 0);

            lua_unref(th, onMessageRef);
            lua_unref(th, onCloseRef);
            lua_unref(th, threadRef);
        }

        int handleIndex(lua_State* ls) {
            if (!ls || !connected) return 0;

            luaL_checktype(ls, 1, LUA_TUSERDATA);
            std::string idx = luaL_checkstring(ls, 2);

            if (idx == "OnMessage") {
                lua_getref(ls, this->onMessageRef);
                lua_getfield(ls, -1, "Event");
                return 1;
            }
            else if (idx == "OnClose") {
                lua_getref(ls, this->onCloseRef);
                lua_getfield(ls, -1, "Event");
                return 1;
            }
            else if (idx == "Send") {
                lua_pushvalue(ls, -10003);
                lua_pushcclosure(ls,
                    [](lua_State* L) -> int {
                        if (!L) return 0;

                        luaL_checktype(L, 1, LUA_TUSERDATA);
                        std::string data = luaL_checkstring(L, 2);

                        exploit_websocket* ws = reinterpret_cast<exploit_websocket*>(lua_touserdata(L, -10003));
                        if (ws && ws->webSocket && ws->connected) {
                            ws->webSocket->send(data);
                        }
                        return 0;
                    }, "websocketinstance_send", 1);
                return 1;
            }
            else if (idx == "Close") {
                lua_pushvalue(ls, -10003);
                lua_pushcclosure(ls,
                    [](lua_State* L) -> int {
                        if (!L) return 0;

                        exploit_websocket* ws = reinterpret_cast<exploit_websocket*>(lua_touserdata(L, -10003));
                        if (ws && ws->webSocket) {
                            ws->webSocket->close();
                            ws->fireClose();
                        }
                        return 0;
                    }, "websocketinstance_close", 1);
                return 1;
            }

            return 0;
        }

        bool reconnect(const std::string& url) {
            constexpr int maxRetries = 5;
            for (int i = 0; i < maxRetries; ++i) {
                if (url.find("wss://") == 0) {
                    if (setupTLSConnection(url)) {
                        return true;
                    }
                }
                else {
                    webSocket = WebSocket::from_url(url);
                    if (webSocket && webSocket->getReadyState() == WebSocket::OPEN) {
                        return true;
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            return false;
        }

        bool setupTLSConnection(const std::string& url) {
            SSL_library_init();
            OpenSSL_add_all_algorithms();
            SSL_load_error_strings();

            ssl_ctx = SSL_CTX_new(SSLv23_client_method());
            if (!ssl_ctx) {
                return false;
            }

            try {
                asio::ip::tcp::resolver resolver(io_context);
                auto endpoints = resolver.resolve("wss://localhost", "443");
                socket.connect(*endpoints.begin());

                ssl = SSL_new(ssl_ctx);
                SSL_set_fd(ssl, socket.native_handle());
                if (SSL_connect(ssl) == -1) {
                    ERR_print_errors_fp(stderr);
                    return false;
                }

                webSocket = WebSocket::from_url(url, [&socket, &ssl](const std::string& msg) {
                    SSL_write(ssl, msg.c_str(), msg.size());
                });

                if (webSocket && webSocket->getReadyState() == WebSocket::OPEN) {
                    return true;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error setting up TLS connection: " << e.what() << std::endl;
                return false;
            }

            return false;
        }
    };

    int connect(lua_State* ls) {
        SetFunction("websocket::connect");
        luaL_checktype(ls, 1, LUA_TSTRING);
        std::string url = luaL_checkstring(ls, 1);

        exploit_websocket* ws = (exploit_websocket*)lua_newuserdata(ls, sizeof(exploit_websocket));
        new (ws) exploit_websocket{};

        ws->th = lua_newthread(ls);
        ws->threadRef = lua_ref(ls, -1);
        lua_pop(ls, 1);

        if (!ws->reconnect(url)) {
            luaL_error(ls, "Failed to connect to WebSocket after multiple retries");
            return 0;
        }

        lua_getglobal(ls, "Instance");
        lua_getfield(ls, -1, "new");
        lua_pushstring(ls, "BindableEvent");
        lua_pcall(ls, 1, 1, 0);
        ws->onMessageRef = lua_ref(ls, -1);
        lua_pop(ls, 2);

        lua_getglobal(ls, "Instance");
        lua_getfield(ls, -1, "new");
        lua_pushstring(ls, "BindableEvent");
        lua_pcall(ls, 1, 1, 0);
        ws->onCloseRef = lua_ref(ls, -1);
        lua_pop(ls, 2);

        ws->connected = true;
        ws->running = true;
        ws->pollThread = std::thread(&exploit_websocket::pollMessages, ws);

        lua_newtable(ls);
        lua_pushstring(ls, "WebSocket");
        lua_setfield(ls, -2, "__type");

        lua_pushvalue(ls, -2);
        lua_pushcclosure(ls,
            [](lua_State* L) -> int {
                exploit_websocket* ws = reinterpret_cast<exploit_websocket*>(lua_touserdata(L, lua_upvalueindex(1)));
                return ws->handleIndex(L);
            },
            "__index", 1);
        lua_setfield(ls, -2, "__index");
        lua_setmetatable(ls, -2);

        return 1;
    }
}
