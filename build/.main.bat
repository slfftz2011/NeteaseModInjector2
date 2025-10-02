@echo off
chcp 65001
cd ..
setlocal enabledelayedexpansion

windres resource.rc -o build\bin\resource.o

:: 移除OpenSSL依赖，保留原有库
set "CXX=g++"
set "CXXFLAGS=-std=c++17 -I. -Isrc -Wall -Wextra -Wno-unknown-pragmas"
set "LDFLAGS=-lws2_32 -lwininet -lurlmon -lshell32 -luser32 -lkernel32 -lgdi32 -liphlpapi -ladvapi32 -lole32 -loleaut32"
set "OUT_DIR=build\bin"
set "RESOURCE=build\bin\resource.o"
set "SRC_FILES=main.cpp src/FileProcessor.cpp src/GameDeployer.cpp src/Injector.cpp src/JsonParser.cpp src/NetworkChecker.cpp src/RegistryReader.cpp src/URLOpener.cpp"

:: 创建输出目录
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

:: 获取版本号
set /p version=<version.txt

:: 编译命令
echo 开始编译 ModInjector %version%...
%CXX% %CXXFLAGS% %SRC_FILES% %RESOURCE% -o "%OUT_DIR%\NeteaseModInjector_%version%.exe" %LDFLAGS%

:: 检查编译结果
if %errorlevel% equ 0 (
    echo 编译成功！输出文件：%OUT_DIR%\NeteaseModInjector_%version%.exe
) else (
    echo 编译失败！
    exit /b 1
)

pause >nul

endlocal