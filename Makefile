CFLAGS=/nologo /O2
RFLAGS=/nologo /n

hotcorner.exe: hotcorner.obj version.res
	$(CC) $(CFLAGS) /Fe:$(@F) $**

clean:
	del *.obj *.res hotcorner.exe

install: hotcorner.exe
	copy hotcorner.exe "%USERPROFILE%\Start Menu\Programs\Startup"

uninstall:
	del "%USERPROFILE%\Start Menu\Programs\Startup\hotcorner.exe"
