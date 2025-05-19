set "BOOST_INCLUDE=D:\sources\_uss-vc141\boost_1_85_0"
echo off
call "%USS_VS_ROOT_2022%\VC\Auxiliary\Build\vcvarsall.bat" x64
@echo on

devenv .\TestTaskAtto.sln