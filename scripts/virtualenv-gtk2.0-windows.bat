@ECHO OFF
IF [%VIRTUAL_ENV%] == [] (
    echo "VIRTUAL_ENV environment variable has not been set.  Please make sure that you have activated a virtualenv before using this script"
    EXIT /B 0
)

IF NOT EXIST "%VIRTUAL_ENV%"\Lib\site-packages\cairo (
    mklink /D "%VIRTUAL_ENV%"\Lib\site-packages\cairo C:\Python27\Lib\site-packages\cairo
)

IF NOT EXIST "%VIRTUAL_ENV%"\Lib\site-packages\gtk-2.0 (
    mklink /D "%VIRTUAL_ENV%"\Lib\site-packages\gtk-2.0 C:\Python27\Lib\site-packages\gtk-2.0
)

IF NOT EXIST "%VIRTUAL_ENV%"\Lib\site-packages\pygtk.pth (
    mklink "%VIRTUAL_ENV%"\Lib\site-packages\pygtk.pth C:\Python27\Lib\site-packages\pygtk.pth
)

echo "Links for GTK have been created"
