#define WIN32_LEAN_AND_MEAN
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#include "hotcorner.h"

#pragma comment(lib, "USER32")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

// This is a **very** minimal hotcorner app, written in C. Maybe its not the
// optimal way to do this, but it works for me.
//
// Zero state is stored anywhere, no registry keys or configuration files.
//
// - If you want to configure something, edit the code.
// - If you want to uninstall it, just delete it.
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

// What modifier keys you want pressed/unpressed to trigger the hotkey function.
// By default, CTRL+ALT, but it's safe to add or change these.
static const MODSTATE kHotKeyModifier[] = {
    { VK_LCONTROL,  true },
    { VK_LMENU,     true },
    { VK_LSHIFT,    false },
};

// Add, remove, or change hotkeys here. 
static FARPROC kHotKeyCallbacks[256] = {
    ['A'] = ToggleHotCorners,
    ['C'] = ExitCorners,
};

// How long cursor has to linger in the kHotCorner RECT to trigger input.
static const DWORD kHotDelay = 300;

static HANDLE CornerThread = INVALID_HANDLE_VALUE;
static BYTE KeyState[256];
static HHOOK MouseHook, KeyHook;

// This thread runs when the cursor enters the hot corner, and waits to see if the cursor stays in the corner.
// If the mouse leaves while we're waiting, the thread is just terminated.
static DWORD WINAPI CornerHotFunc(LPVOID lpParameter)
{
    POINT Point;

    Sleep(kHotDelay);

    // Check if a mouse putton is pressed, maybe a drag operation?
    if (KeyState[VK_LBUTTON] || KeyState[VK_RBUTTON] || KeyState[VK_MBUTTON])  {
        return 0;
    }

    // Check if any modifier keys are pressed.
    if (KeyState[VK_SHIFT]   || KeyState[VK_RSHIFT]     || KeyState[VK_LSHIFT]
     || KeyState[VK_CONTROL] || KeyState[VK_RCONTROL]   || KeyState[VK_LCONTROL]
     || KeyState[VK_MENU]    || KeyState[VK_LMENU]      || KeyState[VK_RMENU]
     || KeyState[VK_LWIN]    || KeyState[VK_RWIN]) {
        return 0;
    }

    // Verify the corner is still hot
    if (GetCursorPos(&Point) == false) {
        return 1;
    }

    // Check co-ordinates.
    if (PtInRect(&kHotCorner, Point)) {
        #pragma warning(suppress : 4090)
        if (SendInput(_countof(kCornerInput), kCornerInput, sizeof(INPUT)) != _countof(kCornerInput)) {
            return 1;
        }
    }

    return 0;
}

static LRESULT CALLBACK MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    MSLLHOOKSTRUCT *evt = (MSLLHOOKSTRUCT *) lParam;

    // Track button state.
    switch (wParam) {
        case WM_LBUTTONDOWN:
            KeyState[VK_LBUTTON] = true;
            break;
        case WM_LBUTTONUP:
            KeyState[VK_LBUTTON] = false;
            break;
        case WM_RBUTTONDOWN:
            KeyState[VK_RBUTTON] = true;
            break;
        case WM_RBUTTONUP:
            KeyState[VK_RBUTTON] = false;
            break;
        case WM_MOUSEWHEEL:
            KeyState[VK_MBUTTON] = HIWORD(wParam) >= 0 || LOWORD(wParam) == MK_MBUTTON;
            break;
        case WM_XBUTTONDOWN:
            if (HIWORD(evt->mouseData) == XBUTTON1)
                KeyState[VK_XBUTTON1] = true;
            else
                KeyState[VK_XBUTTON2] = true;
            break;
        case WM_XBUTTONUP:
            if (HIWORD(evt->mouseData) == XBUTTON2)
                KeyState[VK_XBUTTON1] = false;
            else
                KeyState[VK_XBUTTON2] = false;
            break;
    }

    // If the mouse hasn't moved, we're done.
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

static LRESULT CALLBACK KeyHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT *evt = (KBDLLHOOKSTRUCT *) lParam;
    size_t n;

    if (nCode != HC_ACTION)
        goto finish;

    // Track the state of keys
    switch (wParam) {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            KeyState[evt->vkCode] = true;
            break;
        case WM_SYSKEYUP:
        case WM_KEYUP:
            KeyState[evt->vkCode] = false;
            // fallthrough
        default:
            goto finish;
    }

    // Check if a hotkey is being requested.
    for (n = 0; n < _countof(kHotKeyModifier); n++) {
        if (KeyState[kHotKeyModifier[n].KeyCode] != kHotKeyModifier[n].KeyDown)
            goto finish;
    }

    if (kHotKeyCallbacks[evt->vkCode]) {
        kHotKeyCallbacks[evt->vkCode]();
    }

finish:
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static int CALLBACK ToggleHotCorners(void)
{

    if (MouseHook) {
        UnhookWindowsHookEx(MouseHook);
        MouseHook = NULL;
    } else {
        MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookCallback, NULL, 0);
    }

    return !! MouseHook;
}

static int CALLBACK ExitCorners(void)
{
    return TerminateProcess(GetCurrentProcess(), 0);
}


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG Msg;

    if (!ToggleHotCorners())
        return 1;

    if (!(KeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyHookCallback, NULL, 0)))
        return 1;

    while (GetMessage(&Msg, NULL, 0, 0)) {
        DispatchMessage(&Msg);
    }

    UnhookWindowsHookEx(KeyHook);
    UnhookWindowsHookEx(MouseHook);

    return Msg.wParam;
}
