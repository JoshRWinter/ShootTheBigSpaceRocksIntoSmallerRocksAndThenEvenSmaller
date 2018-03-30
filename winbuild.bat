call qtpath.bat

move winqt\assets\sfx\music .
rd /s /q winqt
mkdir winqt
xcopy assets winqt\assets /E /h /I
mkdir winqt\assets\sfx
move music winqt\assets\sfx

cl /I%qtpath%\include /I%qtpath%\include\QtCore /I%qtpath%\include\QtGui /I%qtpath%\include\QtWidgets /I%qtpath%\include\QtGamepad /I%qtpath%\include\QtMultimedia /EHsc *.cpp ws2_32.lib %qtpath%\lib\Qt5Core.lib %qtpath%\lib\Qt5Widgets.lib %qtpath%\lib\Qt5Gui.lib %qtpath%\lib\Qt5Gamepad.lib %qtpath%\lib\Qt5Multimedia.lib /link /out:winqt\stbsrisrates.exe

cl /EHsc /DFREE_SERVER Server.cpp GameState.cpp Log.cpp network.cpp ws2_32.lib /link /out:winqt/stbsrisrates-dedicated.exe

%qtpath%\bin\windeployqt.exe --release winqt\stbsrisrates.exe
