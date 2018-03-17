Asteroids inspired game for Linux and Windows (C++/Qt)

Multiplayer support, includes server software (2 players)

QtGamepad support, tested with Microsoft XBOX360 Controller (wired)

# DEMO
video: https://streamable.com/0z644

# COMPILATION
## LINUX
1. client: `make release`
2. standalone server: `make server`

## WINDOWS
1. Install MSVC++
2. open "Native Tools command prompt (x64)"
3. `echo set qtpath=C:\Path\To\Qt\5.10.0\msvc2017_64>qtpath.bat` <-- don't put spaces around the `<` in `...>qtpath.bat`
4. `.\winbuild.bat`
