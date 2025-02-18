#include "console.h"

#ifdef WIN_OS

namespace console_utils {
    void blockQuickChange() {
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);

        if (hStdin == nullptr) {
            loggerError << "GetStdHandler() return nullptr";
        }
        else {
            // Prev console mode stdin
            DWORD prevConsoleModeStdin;

            if (GetConsoleMode(hStdin, &prevConsoleModeStdin)) {
                SetConsoleMode(hStdin, prevConsoleModeStdin & (~ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS));
            }
        }
    }

    int blockScroll() {
        // Get handle to the console window
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

        // Retrieve screen buffer info
        CONSOLE_SCREEN_BUFFER_INFO scrBufferInfo;
        GetConsoleScreenBufferInfo(hOut, &scrBufferInfo);

        // Current window size
        short winWidth = scrBufferInfo.srWindow.Right - scrBufferInfo.srWindow.Left + 1;
        short winHeight = scrBufferInfo.srWindow.Bottom - scrBufferInfo.srWindow.Top + 1;

        // Current screen buffer size
        short scrBufferWidth = scrBufferInfo.dwSize.X;
        short scrBufferHeight = scrBufferInfo.dwSize.Y;

        // To remove the scrollbar, make sure the window height matches the screen buffer height
        COORD newSize;
        newSize.X = scrBufferWidth;
        newSize.Y = winHeight;

        // Set the new screen buffer dimensions
        int Status = SetConsoleScreenBufferSize(hOut, newSize);
        if (Status == 0)
        {
            loggerError << (LogMsg() << "SetConsoleScreenBufferSize() failed! Reason: " << GetLastError());
        }

        return Status;
    }

    int blockResize() {
        HWND hWindow;
        //HMENU hMenu;

        if ((hWindow = GetConsoleWindow()) == nullptr)
        {
            return 0;
        }

        SetWindowLong(hWindow, -16, GetWindowLong(hWindow, -16) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

        /*hMenu = GetSystemMenu(hWindow, FALSE);
        RemoveMenu(hMenu, SC_SIZE, MF_BYCOMMAND | MF_REMOVE);
        RemoveMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND | MF_REMOVE);
        DrawMenuBar(hWindow);*/

        return 1;
    }

    bool consoleExists() {
        return (GetConsoleCP != FALSE);
    }
}

#endif