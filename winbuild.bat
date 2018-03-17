call qtpath.bat

rd /s /q winqt
mkdir winqt
xcopy assets winqt\assets /E /h /I

cl /I%qtpath%\include /I%qtpath%\include\QtCore /I%qtpath%\include\QtGui /I%qtpath%\include\QtWidgets /I%qtpath%\include\QtGamepad /EHsc *.cpp ws2_32.lib %qtpath%\lib\Qt5Core.lib %qtpath%\lib\Qt5Widgets.lib %qtpath%\lib\Qt5Gui.lib %qtpath%\lib\Qt5Gamepad.lib /link /out:winqt\stbsrisrates.exe

cl /EHsc /DFREE_SERVER Server.cpp GameState.cpp Log.cpp network.cpp ws2_32.lib /link /out:winqt/stbsrisrates-dedicated.exe

%qtpath%\bin\windeployqt.exe --release winqt\stbsrisrates.exe
