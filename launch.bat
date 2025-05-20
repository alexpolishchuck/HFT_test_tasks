echo off

set "BOOST_INCLUDE=D:\sources\boost_1_85_0"
set "VS_ROOT=C:\Program Files\Microsoft Visual Studio\2022\Professional"

call "%VS_ROOT%\VC\Auxiliary\Build\vcvarsall.bat" x64

@echo on

devenv .\TestTaskAtto.sln