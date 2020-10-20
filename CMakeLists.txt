include(CheckIncludeFileCXX)

cmake_minimum_required(VERSION 3.10)
project(dlog LANGUAGES CXX VERSION 0.0.1)

add_executable(dlog "src/main.cpp"
    "src/commandline.cpp" "src/commandline.h"
    "src/entry.h"
    "src/files.h" "src/files.cpp"
    "src/config.h" "src/config.cpp"
    "src/subcommands/subcommands.h" "src/subcommands/subcommands.cpp"
    "src/subcommands/subcommand.h" "src/subcommands/subcommand.cpp"
    "src/subcommands/main.h" "src/subcommands/main.cpp"
    "src/subcommands/start.h" "src/subcommands/start.cpp"
)
configure_file(version.h.in version.h)

set(CMAKE_CXX_STANDARD 17)
set(ENV{CMAKE_EXPORT_COMPILE_COMMANDS} TRUE)
set(CLANG_TIDY_CHECKS *,-llvm-include-order,-fuchsia-default-arguments,-clang-diagnostic-error,-misc-unused-parameters,-llvm-namespace-comment,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-google-runtime-references,-cppcoreguidelines-owning-memory)
option(USE_LINTERS "Should compilation use linters?" ON)
target_compile_features(dlog PUBLIC cxx_std_17)
target_link_libraries(dlog -lstdc++fs)

CHECK_INCLUDE_FILE_CXX(windows.h IS_WINDOWS)

if (IS_WINDOWS)
    add_compile_definitions(IS_WINDOWS)
endif()

if (USE_LINTERS)
    find_program(CLANG_TIDY NAMES clang-tidy-7 clang-tidy-6.0 clang-tidy-5.0 clang-tidy-4.0 clang-tidy)
    find_program(CPPCHECK NAMES cppcheck)
    find_program(CPPLINT NAMES cpplint cpplint.py)

    if (CLANG_TIDY)
        message(STATUS "Found clang-tidy. Configuring...")
        set_property(TARGET dlog PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY};-checks=${CLANG_TIDY_CHECKS})
    else()
        message(STATUS "Did not find clang-tidy. Proceeding without...")
    endif()

    if (CPPCHECK)
        message(STATUS "Found cppcheck. Configuring...")
        set_property(TARGET dlog PROPERTY CXX_CPPCHECK ${CPPCHECK})
    else()
        message(STATUS "Did not find cppcheck. Proceeding without...")
    endif()

    if (CPPLINT)
        message(STATUS "Found cpplint. Configuring...")
        set_property(TARGET dlog PROPERTY CXX_CPPLINT ${CPPLINT})
    else()
        message(STATUS "Did not find cpplint. Proceeding without...")
    endif()
endif()

if (NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/cxxopts.hpp")
    file(DOWNLOAD "https://raw.githubusercontent.com/jarro2783/cxxopts/master/include/cxxopts.hpp" "${CMAKE_SOURCE_DIR}/libs/cxxopts.hpp")
endif()

include_directories(dlog PUBLIC "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/libs")
target_include_directories(dlog PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_BINARY_DIR}/libs")
