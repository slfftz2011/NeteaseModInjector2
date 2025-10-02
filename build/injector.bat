@echo off
chcp 65001
cd ..
setlocal enabledelayedexpansion
title Injector Test
cls
echo ==============================================
echo              Injector 测试工具
echo ==============================================
echo.

:: 编译参数配置
set "CXX=g++"
set "CXXFLAGS=-std=c++20 -I. -Isrc -Wall -Wextra"
set "LDFLAGS=-lws2_32 -lwininet -lurlmon -lshell32 -luser32 -lkernel32 -lgdi32 -liphlpapi -ladvapi32 -lole32 -loleaut32 -Wno-unknown-pragmas"
set "TEST_OUT_DIR=build\lib"
set "SRC_FILES=src\Injector.cpp src\RegistryReader.cpp src\test\InjectorTest.cpp"
set "OUT_EXE=!TEST_OUT_DIR!\test_injector.exe"

:: 创建输出目录
if not exist "!TEST_OUT_DIR!" mkdir "!TEST_OUT_DIR!"

:: 编译测试程序
echo [编译] 开始编译Injector测试程序...
!CXX! !CXXFLAGS! !SRC_FILES! -o "!OUT_EXE!" !LDFLAGS!

:: 检查编译结果
if %errorlevel% equ 0 (
    echo [成功] 编译完成：!OUT_EXE!
    echo.
    echo [运行] 启动测试程序...
    echo.
    "!OUT_EXE!"
) else (
    echo [错误] 编译失败！
)

echo.
echo ==============================================
echo 测试结束，按任意键退出...
pause >nul
endlocal
