cmake_minimum_required(VERSION 3.20)

project(ModInjector2 VERSION 2.0.0 LANGUAGES CXX RC)

if(NOT WIN32)
    message(FATAL_ERROR "ModInjector2 currently supports Windows only.")
endif()

add_executable(ModInjector2
    main.cpp
    resource.rc
    src/Application.cpp
    src/ConsoleOutput.cpp
    src/FileProcessor.cpp
    src/GameDeployer.cpp
    src/Injector.cpp
    src/JsonParser.cpp
    src/MciApiClient.cpp
    src/NetworkChecker.cpp
    src/RegistryReader.cpp
    src/URLOpener.cpp
)

target_compile_features(ModInjector2 PRIVATE cxx_std_17)
target_include_directories(ModInjector2 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(MSVC)
    target_compile_options(ModInjector2 PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:/W4>"
        "$<$<COMPILE_LANGUAGE:CXX>:/utf-8>"
    )
else()
    target_compile_options(ModInjector2 PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:-Wall>"
        "$<$<COMPILE_LANGUAGE:CXX>:-Wextra>"
        "$<$<COMPILE_LANGUAGE:CXX>:-Wno-unknown-pragmas>"
        "$<$<COMPILE_LANGUAGE:CXX>:-finput-charset=UTF-8>"
        "$<$<COMPILE_LANGUAGE:CXX>:-fexec-charset=UTF-8>"
    )
endif()

target_link_libraries(ModInjector2 PRIVATE
    ws2_32
    wininet
    urlmon
    shell32
    user32
    kernel32
    gdi32
    iphlpapi
    advapi32
    ole32
    oleaut32
)

set_target_properties(ModInjector2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)