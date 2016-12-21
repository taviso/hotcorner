#define WIN32_LEAN_AND_MEAN
#include <stdlib.h>
#include <windows.h>

#pragma comment(lib, "user32")

// In GNOME 3 whenever you move the mouse to the top left corner, GNOME
// switches to the activities view, it looks like this:
//
//  https://www.gnome.org/wp-content/uploads/2016/03/window-selection-3.20-420x236.png
//
// Whenever I'm using Windows, I always forget that this doesn't work. Bleh.
//
// I searched around for existing solutions, and wasn't happy with anything I could find.
//
// The options seem to be
//
//  * Some ridiculous AutoHotKey/AutoIT monstrosity (?!?).
//  * Massive Delphi application with 100MB of resources.
//  * Some naive program that polls GetCursorPos() in a busy loop.
//
// None of these are what I want, I just want that GNOME 3 thing with absolute
// minimal overhead.
// 
// This is a **very** minimal hotcorner app, written in C. Maybe its not the
// optimal way to do this, but it works for me.
//
// Zero state is stored anywhere, no registry keys or configuration files.
//
// - If you want to configure something, edit the code.
// - If you want to uninstall it, just delete it.
//
// Instructions
// ------------------
//
// Change any of the parameters below, compile, then install.
//
// Building:
//
//  > nmake
//
// Installing:
//
// > copy hotcorners.exe "%USERPROFILE%\Start Menu\Programs\Startup"
//
// Uninstalling:
//
// > del "%USERPROFILE%\Start Menu\Programs\Startup\hotcorners.exe"
//
//
// If you don't have cl or nmake, they come with Visual Studio (or the Windows SDK, I think).
//
// Tavis Ormandy <taviso@cmpxchg8b.com> December, 2016
//
// https://github.com/taviso/hotcorner
//

// If the mouse enters this rectangle, activate the hot corner function.
static const RECT kHotCorner = {
    .top    = -20,
    .left   = -20,
    .right  = +20,
    .bottom = +20,
};

// Input to inject when corner activated (Win+Tab by default).
static const INPUT kCornerInput[] = {
    { .type = INPUT_KEYBOARD, .ki = { .wVk = VK_LWIN }},
    { .type = INPUT_KEYBOARD, .ki = { .wVk = VK_TAB }},
    { .type = INPUT_KEYBOARD, .ki = { .wVk = VK_TAB, .dwFlags = KEYEVENTF_KEYUP }},
    { .type = INPUT_KEYBOARD, .ki = { .wVk = VK_LWIN, .dwFlags = KEYEVENTF_KEYUP }},
};

// How long cursor has to linger in the kHotCorner RECT to trigger input.
static const DWORD kHotDelay = 300;

static HANDLE CornerThread = INVALID_HANDLE_VALUE;

// This thread runs when the cursor enters the hot corner, and waits to see if the cursor stays in the corner.
// If the mouse leaves while we're waiting, the thread is just terminated.
static DWORD WINAPI CornerHotFunc(LPVOID lpParameter)
{
    POINT Point;

    Sleep(kHotDelay);
    
    // Verify the corner is still hot
    if (GetCursorPos(&Point) == FALSE) {
        return 1;
    }

    // Check co-ordinates.
    if (PtInRect(&kHotCorner, Point)) {
        if (SendInput(_countof(kCornerInput), kCornerInput, sizeof(INPUT)) != _countof(kCornerInput)) {
            return 1;
        }
    }

    return 0;
}

static LRESULT CALLBACK MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    MSLLHOOKSTRUCT *evt = (MSLLHOOKSTRUCT *)(lParam);

    if (wParam != WM_MOUSEMOVE)
        goto finish;

    // Check if the cursor is hot or cold.
    if (!PtInRect(&kHotCorner, evt->pt)) {

        // The corner is cold, and was cold before.
        if (CornerThread == INVALID_HANDLE_VALUE)
            goto finish;

        // The corner is cold, but was previously hot.
        TerminateThread(CornerThread, 0);

        CloseHandle(CornerThread);

        // Reset state.
        CornerThread = INVALID_HANDLE_VALUE;

        goto finish;
    }

    // The corner is hot, check if it was already hot.
    if (CornerThread != INVALID_HANDLE_VALUE) {
        goto finish;
    }

    // Check if a mouse putton is pressed, maybe a drag operation?
    if (GetKeyState(VK_LBUTTON) < 0 || GetKeyState(VK_RBUTTON) < 0) {
        goto finish;
    }

    // The corner is hot, and was previously cold. Here we start a thread to
    // monitor if the mouse lingers.
    CornerThread = CreateThread(NULL, 0, CornerHotFunc, NULL, 0, NULL);

finish:
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main(int argc, char **argv)
{
    MSG Msg;
    HHOOK Hook;


    if (!(Hook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookCallback, NULL, 0)))
        return 1;

    while (GetMessage(&Msg, NULL, 0, 0)) {
        DispatchMessage(&Msg);
    }

    UnhookWindowsHookEx(Hook);

    return Msg.wParam;
}
