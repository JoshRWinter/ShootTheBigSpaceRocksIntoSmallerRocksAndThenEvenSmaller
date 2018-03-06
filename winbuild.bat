set qtver=5.9.1

rd /s /q winqt
mkdir winqt

cl /I"C:\Qt\%qtver%\msvc2017_64\include" /I"C:\Qt\%qtver%\msvc2017_64\include\QtCore" /I"C:\Qt\%qtver%\msvc2017_64\include\QtGui" /I"C:\Qt\%qtver%\msvc2017_64\include\QtWidgets" /EHsc *.cpp ws2_32.lib C:\Qt\%qtver%\msvc2017_64\lib\Qt5Core.lib C:\Qt\%qtver%\msvc2017_64\lib\Qt5Widgets.lib C:\Qt\%qtver%\msvc2017_64\lib\Qt5Gui.lib /link /out:winqt\stbsrisrates.exe

c:\Qt\%qtver%\msvc2017_64\bin\windeployqt.exe --release winqt\stbsrisrates.exe
