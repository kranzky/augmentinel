cmake_minimum_required(VERSION 3.15)
project(Augmentinel VERSION 1.6.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform definitions
if(APPLE)
    add_definitions(-D__APPLE__ -DPLATFORM_MACOS)
elseif(WIN32)
    add_definitions(-DWIN32 -DPLATFORM_WINDOWS)
elseif(UNIX)
    add_definitions(-D__linux__ -DPLATFORM_LINUX)
endif()

# Find packages
find_package(SDL2 REQUIRED CONFIG)
find_package(OpenGL REQUIRED)

# SDL2_mixer
find_library(SDL2_MIXER_LIBRARY SDL2_mixer REQUIRED)
get_filename_component(SDL2_MIXER_LIB_DIR ${SDL2_MIXER_LIBRARY} DIRECTORY)
find_path(SDL2_MIXER_INCLUDE_DIR SDL2/SDL_mixer.h
    HINTS ${SDL2_MIXER_LIB_DIR}/../include
)

# DirectXMath (cross-platform math library)
include(FetchContent)
FetchContent_Declare(
    directxmath
    GIT_REPOSITORY https://github.com/microsoft/DirectXMath.git
    GIT_TAG dec2022
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(directxmath)

# Source files
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/OpenGLRenderer.cpp
    src/View.cpp
    src/Augmentinel.cpp
    src/Spectrum.cpp
    src/Model.cpp
    src/Camera.cpp
    src/Animate.cpp
    src/Audio.cpp
    src/Settings.cpp
    src/Utils.cpp
    z80/Z80.c
)

set(HEADERS
    src/Platform.h
    src/Application.h
    src/OpenGLRenderer.h
    src/Augmentinel.h
    src/Spectrum.h
    src/Model.h
    src/Camera.h
    src/Animate.h
    src/Audio.h
    src/Settings.h
    src/Utils.h
    src/Game.h
    src/Sentinel.h
    src/Vertex.h
    src/Action.h
    src/SimpleHeap.h
    src/SharedConstants.h
    z80/Z80.h
    z80/Z80-support.h
)

add_executable(Augmentinel ${SOURCES} ${HEADERS})

target_include_directories(Augmentinel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/z80
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIR}
    ${directxmath_SOURCE_DIR}/Inc
)

target_link_libraries(Augmentinel
    SDL2::SDL2
    ${SDL2_MIXER_LIBRARY}
    OpenGL::GL
)

# macOS-specific settings
if(APPLE)
    target_compile_definitions(Augmentinel PRIVATE
        GL_SILENCE_DEPRECATION
        PLATFORM_MACOS
        _XM_NO_INTRINSICS_
        CPU_Z80_DEPENDENCIES_H="Z80-support.h"
        CPU_Z80_USE_LOCAL_HEADER
    )
endif()

# Copy resources to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/48.rom
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sentinel.sna
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copy sounds directory if it exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/sounds)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sounds
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Copy shaders directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)

# Print configuration info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "SDL2 found: ${SDL2_FOUND}")
message(STATUS "SDL2_mixer library: ${SDL2_MIXER_LIBRARY}")
message(STATUS "OpenGL found: ${OPENGL_FOUND}")
message(STATUS "DirectXMath: ${directxmath_SOURCE_DIR}")
