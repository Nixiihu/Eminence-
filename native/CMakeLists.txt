cmake_minimum_required(VERSION 3.24)
project(EminenceNative LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "EminenceNative currently targets Windows x64.")
endif()

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.2
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    minhook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG v1.3.4
)
FetchContent_MakeAvailable(minhook)

set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(MINHOOK_DIR ${minhook_SOURCE_DIR})

add_library(EminenceNative SHARED
    src/dllmain.cpp
    src/native_core.cpp
    src/module.cpp
    src/jni_bridge.cpp
    src/imgui_menu.cpp

    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp

    ${MINHOOK_DIR}/src/buffer.c
    ${MINHOOK_DIR}/src/hde/hde32.c
    ${MINHOOK_DIR}/src/hde/hde64.c
    ${MINHOOK_DIR}/src/hook.c
    ${MINHOOK_DIR}/src/trampoline.c
)

target_include_directories(EminenceNative PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${MINHOOK_DIR}/include
    ${MINHOOK_DIR}/src
)

find_package(JNI REQUIRED)
target_include_directories(EminenceNative PRIVATE ${JNI_INCLUDE_DIRS})

target_compile_definitions(EminenceNative PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    IMGUI_IMPL_OPENGL_LOADER_CUSTOM
)

target_link_libraries(EminenceNative PRIVATE
    ${JNI_LIBRARIES}
    opengl32
    user32
    gdi32
)

set_target_properties(EminenceNative PROPERTIES
    OUTPUT_NAME "EminenceNative"
    WIN32_EXECUTABLE FALSE
)
