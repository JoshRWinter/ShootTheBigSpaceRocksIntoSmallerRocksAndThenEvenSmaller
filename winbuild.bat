rd /s /q winqt
mkdir winqt

cl /I"C:\Qt\5.10.0\msvc2017_64\include" /I"C:\Qt\5.10.0\msvc2017_64\include\QtCore" /I"C:\Qt\5.10.0\msvc2017_64\include\QtGui" /I"C:\Qt\5.10.0\msvc2017_64\include\QtWidgets" /EHsc *.cpp ws2_32.lib C:\Qt\5.10.0\msvc2017_64\lib\Qt5Core.lib C:\Qt\5.10.0\msvc2017_64\lib\Qt5Widgets.lib C:\Qt\5.10.0\msvc2017_64\lib\Qt5Gui.lib /link /out:winqt\stbsrisrates.exe

c:\Qt\5.10.0\msvc2017_64\bin\windeployqt.exe --release winqt\stbsrisrates.exe
