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

Have fun.

-- Discord: Bytecode (`goodbytecode`)
