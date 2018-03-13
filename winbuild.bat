call qtver.bat

rd /s /q winqt
mkdir winqt
xcopy assets winqt\assets /E /h /I

cl /I"C:\Qt\%qtver%\msvc2017_64\include" /I"C:\Qt\%qtver%\msvc2017_64\include\QtCore" /I"C:\Qt\%qtver%\msvc2017_64\include\QtGui" /I"C:\Qt\%qtver%\msvc2017_64\include\QtWidgets" /I"C:\Qt\%qtver%\msvc2017_64\include\QtGamepad" /EHsc *.cpp ws2_32.lib C:\Qt\%qtver%\msvc2017_64\lib\Qt5Core.lib C:\Qt\%qtver%\msvc2017_64\lib\Qt5Widgets.lib C:\Qt\%qtver%\msvc2017_64\lib\Qt5Gui.lib c:\Qt\%qtver%\msvc2017_64\lib\Qt5Gamepad.lib /link /out:winqt\stbsrisrates.exe

c:\Qt\%qtver%\msvc2017_64\bin\windeployqt.exe --release winqt\stbsrisrates.exe
