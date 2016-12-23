#pragma once

// Callbacks for hotkeys.
static int CALLBACK ToggleHotCorners(void); // CTRL+ALT+A
static int CALLBACK ExitCorners(void);      // CTRL+ALT+C

typedef struct {
    int     KeyCode;
    BOOL    KeyDown;
} MODSTATE, *PMODSTATE;

