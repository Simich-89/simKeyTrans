# SimKeyTrans

A simple Windows keyboard translator.\n\
If you suffer from an inappropriate keyboard layout when entering text or logins / passwords,\
then just press:\
ctrl+alt+1, ctrl+alt+2,...ctrl+alt+9 or ctrl+alt+capslockn\n\
Yes, this program uses SetWindowsHookExW and your antivirus will not like it, \
but I wrote it for myself and you can look at its code so \
you can make sure that there is nothing suspicious there.\
Just add an exception to the antivirus on the directory and place this program in it.\
\n\n(c) 2025 Simich",

## Build Instructions

- Make sure you have a C++ compiler (like MinGW or MSVC) installed.
- Use the provided VS Code build task or run the following command in the terminal:

```
g++ main.cpp -O2 -mwindows -static -static-libgcc -static-libstdc++ -o simKeyTrans.exe
```

## Run

Double-click `simKeyTrans.exe` or run it from the terminal.
