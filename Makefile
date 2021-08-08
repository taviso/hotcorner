CFLAGS=/nologo /O2
RFLAGS=/nologo /n

hotcorner.exe: hotcorner.obj version.res
	$(CC) $(CFLAGS) /Fe:$(@F) $**

clean:
	del *.obj *.res hotcorner.exe hotcorner.bat

install: hotcorner.bat
	copy hotcorner.bat "%USERPROFILE%\Start Menu\Programs\Startup"

uninstall:
	del "%USERPROFILE%\Start Menu\Programs\Startup\hotcorner.bat"
