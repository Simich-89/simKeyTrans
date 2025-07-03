# SimKeyTrans

A simple Windows keyboard translator. \n
If you suffer from an inappropriate keyboard layout when entering text or logins / passwords, \n
then just press: \n
ctrl+alt+1, ctrl+alt+2,...ctrl+alt+9 or ctrl+alt+capslock \n
Yes, this program uses SetWindowsHookExW and your antivirus will not like it, \n
but I wrote it for myself and you can look at its code so \n
you can make sure that there is nothing suspicious there. \n
Just add an exception to the antivirus on the directory and place this program in it. \n
 \n \n
(c) 2025 Simich

## Download .exe or code
You can find precompiled .exe in release section: \n
https://github.com/Simich-89/simKeyTrans/releases/ \n
Also code is there so you could check it or even build it yourself.

## Build Instructions

- Make sure you have a C++ compiler (like MinGW or MSVC) installed.
- Use the provided VS Code build task or run the following command in the terminal:

```
g++ main.cpp resource.res -O2 -mwindows -static -static-libgcc -static-libstdc++ -lurlmon -lole32 -o simKeyTrans.exe  
```

## Run

Double-click `simKeyTrans.exe` or run it from the terminal.

