# Tiny Hot Corners for Windows 10

In GNOME 3 whenever you move the mouse to the top left corner, GNOME switches to the activities view, it looks like this:

 ![Gnome Activities](https://www.gnome.org/wp-content/uploads/2016/03/window-selection-3.20-420x236.png)

Whenever I'm using Windows 10, I always forget that this doesn't work. Bleh.

I searched around for existing solutions, and wasn't happy with anything I could find.

The options seem to be

 * Some ridiculous AutoHotKey/AutoIT monstrosity (?!?).
 * Massive Delphi application with 100MB of resources.
 * Some naive program that polls GetCursorPos() in a busy loop.

None of these are what I want, I just want that GNOME 3 thing with absolute minimal overhead.

This is a **very** minimal hotcorner app, written in C. It doesn't have any features, but it works for me.

Zero state is stored anywhere, no registry keys or configuration files.

- If you want to configure something, edit the code.
- If you want to uninstall it, just delete it.

## Instructions

Change any of the parameters, compile, then install.

A binary is available [here](https://github.com/taviso/hotcorner/releases) if you prefer.

### Building

` > nmake`

### Installing

`> copy hotcorner.exe "%USERPROFILE%\Start Menu\Programs\Startup"`

(or `nmake install`)

### Uninstalling

 `> del "%USERPROFILE%\Start Menu\Programs\Startup\hotcorner.exe"`

(or `nmake uninstall`)

If you don't have cl or nmake, they come with Visual Studio (or the Windows SDK, I think).

### Configuration

All configuration requires modifying the parameters in `hotcorner.c` and recompiling.

* `RECT kHotcorner` - The co-ordinates of the hot zone.
* `INPUT kCornerInput[]` - Input sent on activation.
* `MODSTATE kHotKeyModifier[]` - Modifier Keys (shift, alt, ctrl, etc) you want to enable the hotkey function.
* `FAPROC HotKeyCallbacks[]` - List of all the hotkey functions, you can change the indexes or comment them out.
* `DWORD kHotDelay` - How long the pointer must wait in the corner before being activated.

## License

GPL3

## Authors

* Tavis Ormandy [@taviso](https://github.com/taviso/) - Original Author
* Ahmed Samy [@asamy](https://github.com/asamy) - HotKey support

## FAQ

* Q: I don't want to compile it, can't you just give me an exe? :(
* A: Checkout the releases, [here](https://github.com/taviso/hotcorner/releases).


* Q: Can you change a setting, and then compile it for me?
* A: No.


* Q: This doesn't work with my Application/Configuration/Whatever!
* A: File an issue, if it's feasible to workaround I'll try.


* Q: How do I turn it off without rebooting?
* A: You can use LCTRL+LALT+C to completely shut down the application or CTRL+LALT+A to toggle.

* Q: Why doesn't it work if my current program is running as an Administrator?
* A: [UIPI](https://en.wikipedia.org/wiki/User_Interface_Privilege_Isolation). I suppose you could "Run As Administrator" if it bothers you.

* Q: I don't want any hotkeys at all!
* Q: Comment out all entries of the `kHotKeyCallbacks[]` array.
