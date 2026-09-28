cmake_minimum_required(VERSION 3.21)

# Pin a deployment target so LSMinimumSystemVersion is meaningful. Must precede project().
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")

# project(VERSION) is the single source of truth for the version: it feeds the
# window title, the macOS Info.plist, the Windows version resource and packaging.
project(Augmentinel VERSION 1.7.0 LANGUAGES C CXX)

# De-duplicate static libraries on link lines (SDL2 also arrives through SDL2_mixer).
if(POLICY CMP0156)
    cmake_policy(SET CMP0156 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(AUGMENTINEL_MACOS_BUNDLE "Build Augmentinel.app instead of a bare executable" OFF)

# Everything is linked statically, including the MSVC runtime, so the Windows
# build is a single self-contained exe and the macOS build needs no dylibs.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# --- Dependencies (fetched as release tarballs, pinned by hash) ---
include(FetchContent)
cmake_policy(SET CMP0135 NEW)                            # extracted files get fresh timestamps
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)       # SDL2's own old cmake_minimum_required

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL_FORCE_STATIC_VCRT ON CACHE BOOL "" FORCE)
FetchContent_Declare(SDL2
    URL https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-2.32.10.tar.gz
    URL_HASH SHA256=5f5993c530f084535c65a6879e9b26ad441169b3e25d789d83287040a9ca5165)

# SDL2_mixer with only the codecs the game uses: WAV effects and MP3 music (minimp3).
set(SDL2MIXER_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_VENDORED OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_DEPS_SHARED OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_CMD OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_FLAC OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_GME OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MOD OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MIDI OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_OPUS OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_VORBIS OFF CACHE STRING "" FORCE)
set(SDL2MIXER_WAVPACK OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MP3 ON CACHE BOOL "" FORCE)
set(SDL2MIXER_MP3_MINIMP3 ON CACHE BOOL "" FORCE)
set(SDL2MIXER_MP3_MPG123 OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_WAVE ON CACHE BOOL "" FORCE)
FetchContent_Declare(SDL2_mixer
    URL https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.1/SDL2_mixer-2.8.1.tar.gz
    URL_HASH SHA256=cb760211b056bfe44f4a1e180cc7cb201137e4d1572f2002cc1be728efd22660)

# DirectXMath is header-only; SOURCE_SUBDIR points nowhere so only the headers are fetched.
FetchContent_Declare(DirectXMath
    URL https://github.com/microsoft/DirectXMath/archive/refs/tags/jun2026.tar.gz
    URL_HASH SHA256=cb46002679b16a25fd11d0a2a13f6a7c7bf9c51bf04258fb5485e5b839da8474
    SOURCE_SUBDIR headers-only)

FetchContent_MakeAvailable(SDL2 SDL2_mixer DirectXMath)

# DirectXMath includes <sal.h>, which only MSVC provides. Elsewhere use empty annotations.
set(SAL_STUB_DIR ${CMAKE_BINARY_DIR}/sal-stub)
if(NOT MSVC)
    configure_file(${CMAKE_SOURCE_DIR}/cmake/sal.h ${SAL_STUB_DIR}/sal.h COPYONLY)
endif()

configure_file(${CMAKE_SOURCE_DIR}/src/Version.h.in ${CMAKE_BINARY_DIR}/generated/Version.h @ONLY)

# --- Game ---
add_executable(Augmentinel
    src/main.cpp
    src/Application.cpp
    src/Animate.cpp
    src/Audio.cpp
    src/Augmentinel.cpp
    src/Camera.cpp
    src/DebugOverlay.cpp
    src/GL.cpp
    src/Model.cpp
    src/OpenGLRenderer.cpp
    src/Settings.cpp
    src/Spectrum.cpp
    src/StbImageWrite.cpp
    src/Utils.cpp
    src/View.cpp
    z80/Z80.c)

target_include_directories(Augmentinel PRIVATE
    src
    z80
    ${CMAKE_BINARY_DIR}/generated
    ${directxmath_SOURCE_DIR}/Inc
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${SAL_STUB_DIR}>)

target_compile_definitions(Augmentinel PRIVATE
    CPU_Z80_DEPENDENCIES_H="Z80-support.h"
    CPU_Z80_USE_LOCAL_HEADER
    $<$<CONFIG:Debug>:DEBUG>
    $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS>)

if(MSVC)
    target_compile_options(Augmentinel PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W3 /permissive- /Zc:__cplusplus>)
else()
    # DirectXMath vectors are unions, which -Wmissing-braces misreports.
    target_compile_options(Augmentinel PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wno-missing-braces>)
endif()
set_source_files_properties(src/StbImageWrite.cpp PROPERTIES COMPILE_OPTIONS $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)

find_package(OpenGL REQUIRED)
if(WIN32)
    target_link_libraries(Augmentinel PRIVATE SDL2::SDL2main)  # WinMain -> main
endif()
target_link_libraries(Augmentinel PRIVATE SDL2::SDL2-static SDL2_mixer::SDL2_mixer-static OpenGL::GL)

# --- Runtime resources ---
file(GLOB GLSL_SHADERS CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR} shaders/*.vert shaders/*.frag)
file(GLOB_RECURSE SOUND_FILES CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR} sounds/*.wav sounds/*.mp3)
set(GAME_RESOURCES 48.rom sentinel.sna ${GLSL_SHADERS} ${SOUND_FILES})

if(APPLE AND AUGMENTINEL_MACOS_BUNDLE)
    # Resources live in Contents/Resources; bundle name, ID, version and icon come from Info.plist.in.
    foreach(resource ${GAME_RESOURCES} resources/icon.icns)
        get_filename_component(subdir ${resource} DIRECTORY)
        if(resource MATCHES "^resources/")
            set(subdir "")
        endif()
        set_source_files_properties(${resource} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${subdir}")
        target_sources(Augmentinel PRIVATE ${resource})
    endforeach()
    configure_file(${CMAKE_SOURCE_DIR}/packaging/macos/Info.plist.in
                   ${CMAKE_BINARY_DIR}/generated/Info.plist @ONLY)
    set_target_properties(Augmentinel PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_BINARY_DIR}/generated/Info.plist)
else()
    # Copy resources beside the executable, tracking each file so edits are picked up.
    get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(multi_config)
        set(runtime_dir ${CMAKE_BINARY_DIR}/$<CONFIG>)
    else()
        set(runtime_dir ${CMAKE_BINARY_DIR})
    endif()
    set_target_properties(Augmentinel PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${runtime_dir})

    set(resource_outputs "")
    foreach(resource ${GAME_RESOURCES})
        set(output "${runtime_dir}/${resource}")
        add_custom_command(OUTPUT ${output}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/${resource} ${output}
            DEPENDS ${CMAKE_SOURCE_DIR}/${resource}
            VERBATIM)
        list(APPEND resource_outputs ${output})
    endforeach()
    add_custom_target(AugmentinelResources DEPENDS ${resource_outputs})
    add_dependencies(Augmentinel AugmentinelResources)
endif()

if(WIN32)
    set_target_properties(Augmentinel PROPERTIES WIN32_EXECUTABLE TRUE)
    set(AUGMENTINEL_ICON_PATH "${CMAKE_SOURCE_DIR}/resources/augmentinel.ico")
    configure_file(${CMAKE_SOURCE_DIR}/packaging/windows/Augmentinel.rc.in
                   ${CMAKE_BINARY_DIR}/generated/Augmentinel.rc @ONLY)
    target_sources(Augmentinel PRIVATE ${CMAKE_BINARY_DIR}/generated/Augmentinel.rc)
endif()

# --- Smoke test: boot the emulator, tap through the title screen into landscape 0000,
# and save a screenshot of the game. Needs a desktop session for OpenGL.
enable_testing()
add_test(NAME smoke COMMAND Augmentinel --screenshot ${CMAKE_BINARY_DIR}/smoke.png
    --frames 300 --press 30:Space --press 120:Return)
