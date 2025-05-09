# 88% sUNC open source module

# Why release this?
It was recently announced that The Hunt's anti-cheat is being rolled out for everyone. That pretty much kills off a lot of executors and tools.

I'm releasing this to help people who are running into issues with the new Luau library. Hopefully, this gives you something useful to work with.

This source was originally made for an executor project I was working on. I left the project after having some problems with the owner. I told them not to use my code anymore, but they went ahead anyway — so I'm making it public.

# How to Use
In `dllmain.cpp`, there’s a simple communication method using the file system. You can keep it or change it to something else if you want.
If you stick with the file method:
  * When the DLL is injected, it creates a file called `Injected.txt` in `%localappdata%` to let you know it worked.
  * It then watches `%localappdata%/execute.txt` for changes.
  * If that file changes, it reads the contents and runs it.

# Contributions Welcome
If you find any bugs and you know how to fix them, feel free to open a pull request and i'll make sure to merge it if everything checks out.
functions that don't pass sUNC:
- gethiddenproperty – returns an empty string for SharedString
- getrunningscripts – doesn’t return scripts inside Actors
- getloadedmodules – same issue as above, misses scripts in Actors
- WebSocket.Connect – no TLS support
- getcustomasset – doesn't load .mp3 files
- getconnections – causes a crash

Have fun.

-- Discord: Bytecode (`goodbytecode`)
