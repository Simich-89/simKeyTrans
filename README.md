# SimKeyTrans

A simple Windows keyboard translator.
If you suffer from an inappropriate keyboard layout when entering text or logins / passwords,
then just press:
ctrl+alt+1, ctrl+alt+2,...ctrl+alt+9 or ctrl+alt+capslockn
Yes, this program uses SetWindowsHookExW and your antivirus will not like it,
but I wrote it for myself and you can look at its code so
you can make sure that there is nothing suspicious there.
Just add an exception to the antivirus on the directory and place this program in it.
(c) 2025 Simich",

## Download .exe or code
You can find precompiled .exe in release section:
https://github.com/Simich-89/simKeyTrans/releases/
Also code is there so you could check it or even build it yourself.

## Build Instructions

- Make sure you have a C++ compiler (like MinGW or MSVC) installed.
- Use the provided VS Code build task or run the following command in the terminal:

```
g++ main.cpp resource.res -O2 -mwindows -static -static-libgcc -static-libstdc++ -lurlmon -lole32 -o simKeyTrans.exe  
```

## Run

Double-click `simKeyTrans.exe` or run it from the terminal.

