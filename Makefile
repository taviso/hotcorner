CFLAGS=/Ox

hotcorner.exe: hotcorner.obj

clean:
	del *.obj hotcorner.exe

install: hotcorner.exe
	copy hotcorner.exe "%USERPROFILE%\Start Menu\Programs\Startup"

uninstall:
	del "%USERPROFILE%\Start Menu\Programs\Startup\hotcorner.exe"
