In GNOME 3 whenever you move the mouse to the top left corner, GNOME
switches to the activities view, it looks like this:

 ![Gnome Activities](https://www.gnome.org/wp-content/uploads/2016/03/window-selection-3.20-420x236.png)

Whenever I'm using Windows, I always forget that this doesn't work. Bleh.

I searched around for existing solutions, and wasn't happy with anything I could find.

The options seem to be

 * Some ridiculous AutoHotKey/AutoIT monstrosity (?!?).
 * Massive Delphi application with 100MB of resources.
 * Some naive program that polls GetCursorPos() in a busy loop.

None of these are what I want, I just want that GNOME 3 thing with absolute
minimal overhead.

This is a **very** minimal hotcorner app, written in C. Maybe its not the
optimal way to do this, but it works for me.

Zero state is stored anywhere, no registry keys or configuration files.

- If you want to configure something, edit the code.
- If you want to uninstall it, just delete it.

# Instructions

Change any of the parameters below, compile, then install.

### Building

` > nmake`

### Installing

`> copy hotcorners.exe "%USERPROFILE%\Start Menu\Programs\Startup"`

### Uninstalling

 `> del "%USERPROFILE%\Start Menu\Programs\Startup\hotcorners.exe"`


If you don't have cl or nmake, they come with Visual Studio (or the Windows SDK, I think).

# License

GPL3
