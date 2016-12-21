# Tiny Hot Corners for Windows

In GNOME 3 whenever you move the mouse to the top left corner, GNOME switches to the activities view, it looks like this:

 ![Gnome Activities](https://www.gnome.org/wp-content/uploads/2016/03/window-selection-3.20-420x236.png)

Whenever I'm using Windows, I always forget that this doesn't work. Bleh.

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

### Uninstalling

 `> del "%USERPROFILE%\Start Menu\Programs\Startup\hotcorner.exe"`


If you don't have cl or nmake, they come with Visual Studio (or the Windows SDK, I think).

## License

GPL3

## Author

Tavis Ormandy [@taviso](https://github.com/taviso/)

## FAQ

* Q: I don't want to compile it, can't you just give me an exe? :(
* A: Checkout the releases, [here](https://github.com/taviso/hotcorner/releases).


* Q: Can you change a setting, and then compile it for me?
* A: No.


* Q: This doesn't work with my Application/Configuration/Whatever!
* A: File an issue, if it's feasible to workaround I'll try.


* Q: How do I turn it off without rebooting?
* A: Use [Process Explorer](https://technet.microsoft.com/en-us/sysinternals/processexplorer.aspx), [Process Hacker](http://processhacker.sourceforge.net/), [Task Manager](https://en.wikipedia.org/wiki/Task_Manager_\(Windows\)), or [taskkill.exe](https://technet.microsoft.com/en-us/library/bb491009.aspx) to Suspend or Terminate the process. I told you it was minimal! It also doesn't do anything if you're holding Shift, Control, Alt, etc.

* Q: Why doesn't it work if my current program is running as an Administrator?
* A: [UIPI](https://en.wikipedia.org/wiki/User_Interface_Privilege_Isolation). I suppose you could "Run As Administrator" if it bothers you.
