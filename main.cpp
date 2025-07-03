#include <windows.h>
#include <string>
#include <vector>
#include <ctime>
#include <shellapi.h>
#include <shlobj.h>
#include <objbase.h>
#include <shobjidl.h>
#include <initguid.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

#define APP_VERSION "0.1.1"
#define WM_TRAYICON (WM_USER + 1)
#define IDI_APP_ICON 101

DEFINE_GUID(IID_IShellLinkW, 0x000214F9, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
DEFINE_GUID(IID_IPersistFile, 0x0000010b, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

NOTIFYICONDATA nid = {0};
HMENU hTrayMenu = NULL;

HHOOK g_hHook = NULL;
bool ctrlPressed = false;
bool altPressed = false;
bool progTyping = false;
std::time_t lastT = 0;
std::vector<std::pair<UINT, bool>> lastInput;
std::wstring shortcutPath = L"";

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (progTyping) return CallNextHookEx(g_hHook, nCode, wParam, lParam);

    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        // Ignore injected (SendInput) events
        if (p->flags & LLKHF_INJECTED) return CallNextHookEx(g_hHook, nCode, wParam, lParam);
        // Track modifier keys
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (p->vkCode == VK_CONTROL || p->vkCode == VK_LCONTROL || p->vkCode == VK_RCONTROL) ctrlPressed = true;
            if (p->vkCode == VK_MENU || p->vkCode == VK_LMENU || p->vkCode == VK_RMENU) altPressed = true;
        }
        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            //printf("vkCode: 0x%02X \n", p->vkCode);
            // Check for Ctrl+Alt+int(1-5)
            if (ctrlPressed && altPressed && (p->vkCode == 0x31 || p->vkCode == 0x32 || p->vkCode == 0x33 
                                           || p->vkCode == 0x34 || p->vkCode == 0x35 || p->vkCode == 0x14 )) { // 0x31 is '1'

                progTyping=true;
                keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
                keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
                keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                //Backspace Word
                for (size_t i = 0; i < lastInput.size(); i++) {
                    keybd_event(VK_BACK, 0, 0, 0); keybd_event(VK_BACK, 0, KEYEVENTF_KEYUP, 0);
                }

                //anticaps
                if (p->vkCode == 0x14) {
                    keybd_event(0x14, 0, 0, 0); keybd_event(0x14, 0, KEYEVENTF_KEYUP, 0);
                } else {
                    //Switch language to index
                    int index = p->vkCode - 0x31;
                    int count = GetKeyboardLayoutList(0, NULL);
                    if (count >= 0) {
                        std::vector<HKL> layouts(count);
                        GetKeyboardLayoutList(count, layouts.data());
                        if (index >= 0 && index < count) {
                            HKL hkl = layouts[index];
                            HWND hwnd = GetForegroundWindow();
                            PostMessage(hwnd, WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)hkl);
                        }
                    }
                }
                
                //Retype Word
                for (size_t i = 0; i < lastInput.size(); ++i) {
                    printf("  [%zu] vkCode: 0x%02X (%d), shift: %s\n",i, lastInput[i].first, lastInput[i].first, lastInput[i].second ? "true" : "false");
                    if (lastInput[i].second == true) keybd_event(VK_SHIFT, 0, 0, 0);
                    keybd_event(lastInput[i].first, 0, 0, 0);
                    keybd_event(lastInput[i].first, 0, KEYEVENTF_KEYUP, 0);
                    if (lastInput[i].second == true) keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                }

                //lastInput.clear();
                lastT = static_cast<int>(std::time(nullptr));
                
                progTyping=false;
                return CallNextHookEx(g_hHook, nCode, wParam, lParam);
            }

            // Only process alphabetic (A-Z) and digit (0-9) keys
            //if ((p->vkCode >= 0x41 && p->vkCode <= 0x5A) || (p->vkCode >= 0x30 && p->vkCode <= 0x39)) {
            if (
                // A-Z
                (p->vkCode >= 0x41 && p->vkCode <= 0x5A) ||
                // 0-9 (main)
                (p->vkCode >= 0x30 && p->vkCode <= 0x39) ||
                // Numpad 0-9
                (p->vkCode >= VK_NUMPAD0 && p->vkCode <= VK_NUMPAD9) ||
                // Symbols: - = [ ] \ ; ' , . / (main keyboard)
                p->vkCode == VK_OEM_MINUS    || // -
                p->vkCode == VK_OEM_PLUS     || // =
                p->vkCode == VK_OEM_4        || // [
                p->vkCode == VK_OEM_6        || // ]
                p->vkCode == VK_OEM_5        || // '\'
                p->vkCode == VK_OEM_1        || // ;
                p->vkCode == VK_OEM_7        || // '
                p->vkCode == VK_OEM_COMMA    || // ,
                p->vkCode == VK_OEM_PERIOD   || // .
                p->vkCode == VK_OEM_2        || // /
                // Numpad symbols: / * - + .
                p->vkCode == VK_DIVIDE       || // Numpad /
                p->vkCode == VK_MULTIPLY     || // Numpad *
                p->vkCode == VK_SUBTRACT     || // Numpad -
                p->vkCode == VK_ADD          || // Numpad +
                p->vkCode == VK_DECIMAL         // Numpad .
            ) {

                int nowT = static_cast<int>(std::time(nullptr));
                if (nowT - lastT > 2) {
                    lastInput.clear(); 
                    printf("clear\n"); 
                }
                lastT = nowT;
                bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
                lastInput.push_back({p->vkCode, shiftPressed});
            }

            if (ctrlPressed) ctrlPressed=false;
            if (altPressed) altPressed=false;
        }
    }
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

void ShowAbout(HWND hwnd) {
    MessageBoxW(hwnd,
        L"simKeyTrans\n\n\
A simple Windows keyboard translator.\n\
If you suffer from an inappropriate keyboard layout when entering text or logins / passwords,\
then just press:\
ctrl+alt+1, ctrl+alt+2,...ctrl+alt+9 or ctrl+alt+capslock\n\
Yes, this program uses SetWindowsHookExW and your antivirus will not like it, \
but I wrote it for myself and you can look at its code so \
you can make sure that there is nothing suspicious there.\
Just add an exception to the antivirus on the directory and place this program in it.\
\n\n(c) 2025 Simich",
        L"About simKeyTrans",
        MB_OK | MB_ICONINFORMATION);
}

void CheckForUpdates(HWND hwnd) {
    // URL to a plain text file with the latest version number
    IStream* pStream = nullptr;
    std::string latest;
    if (SUCCEEDED(URLOpenBlockingStreamW(0, L"https://raw.githubusercontent.com/Simich-89/simKeyTrans/main/version.txt", &pStream, 0, 0)) && pStream) {
        char buffer[10];
        ULONG bytesRead = 0;
        while (SUCCEEDED(pStream->Read(buffer, sizeof(buffer) - 1, &bytesRead)) && bytesRead > 0) {
            buffer[bytesRead] = 0;
            latest += buffer;
        }
        pStream->Release();
        
        if (strcmp(latest.c_str(), APP_VERSION) != 0) {
            MessageBoxW(hwnd, L"A new version is available!\nVisit the GitHub releases page.", L"Update Available", MB_OK | MB_ICONINFORMATION);
            ShellExecuteW(hwnd, L"open", L"https://github.com/Simich-89/simKeyTrans/releases/", NULL, NULL, SW_SHOWNORMAL);
        } else {
            MessageBoxW(hwnd, L"You are using the latest version.", L"No Update", MB_OK | MB_ICONINFORMATION);
        }
    } else {
        MessageBoxW(hwnd, L"Could not check for updates.", L"Error", MB_OK | MB_ICONERROR);
    }
}

void manageStartup(bool add) {
    if (!shortcutPath.empty()) {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);

        if (add) {
            CoInitialize(NULL);
            IShellLinkW* pShellLink = nullptr;
            if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&pShellLink))) {
                pShellLink->SetPath(exePath);
                pShellLink->SetDescription(L"SimKeyTrans");
                IPersistFile* pPersistFile = nullptr;
                if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile))) {
                    pPersistFile->Save(shortcutPath.c_str(), TRUE);
                    pPersistFile->Release();
                }
                pShellLink->Release();
                ModifyMenuW(hTrayMenu, 3, MF_BYCOMMAND | MF_STRING, 4, L"Remove from Startup");
            }
            CoUninitialize();
        } else {
            // Remove the shortcut
            if (DeleteFileW(shortcutPath.c_str()) == 0) {
                MessageBoxW(NULL, L"Failed to remove startup shortcut.", L"Error", MB_OK | MB_ICONERROR);
            } else {
                ModifyMenuW(hTrayMenu, 4, MF_BYCOMMAND | MF_STRING, 3, L"Add to Startup");
            }
        }
    } else {
        MessageBoxW(NULL, L"Shortcut path is not set.", L"Error", MB_OK | MB_ICONERROR);
    }
}

// In TrayWndProc, handle the About menu item:
LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hTrayMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
        }
        break;
    case WM_COMMAND:
        if      (LOWORD(wParam) == 1) ShowAbout(hwnd);
        else if (LOWORD(wParam) == 2) CheckForUpdates(hwnd);
        else if (LOWORD(wParam) == 3) manageStartup(true); // Add to Startup
        else if (LOWORD(wParam) == 4) manageStartup(false); // Remove from Startup
        else if (LOWORD(wParam) == 9) { // Exit menu item
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
        }
        break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

    //add Console for debugging    
    //AllocConsole();
    //freopen("CONOUT$", "w", stdout);

    //process running ?    
    HANDLE hMutex = CreateMutexW(NULL, FALSE, L"SimKeyTrans");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(NULL, L"SimKeyTrans is already running.", L"Info", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    // Register a window class for the tray icon
    WNDCLASSW wc = {};
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SimKeyTransTrayClass";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(0, L"SimKeyTransTrayClass", L"SimKeyTrans", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    // Add tray icon
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    Shell_NotifyIcon(NIM_ADD, &nid);

    // Create tray menu
    hTrayMenu = CreatePopupMenu();
    AppendMenuW(hTrayMenu, MF_STRING, 1, L"About");
    AppendMenuW(hTrayMenu, MF_STRING, 2, L"Check for Updates");
    //Check for startup shortcut
    wchar_t startupPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, startupPath))) {
        shortcutPath = std::wstring(startupPath) + L"\\SimKeyTrans.lnk";
        if (GetFileAttributesW(shortcutPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
            AppendMenuW(hTrayMenu, MF_STRING, 3, L"Add to Startup");
        } else {
            AppendMenuW(hTrayMenu, MF_STRING, 4, L"Remove from Startup");
        }
    }
    AppendMenuW(hTrayMenu, MF_STRING, 9, L"Exit");

    MSG msg;
    g_hHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (!g_hHook) {
        MessageBoxW(NULL, L"Failed to install keyboard hook.", L"Error", MB_ICONERROR);
        return 1;
    }
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(g_hHook);
    return 0;
}