cmake_minimum_required(VERSION 3.20)

# Set the deployment target before CMake initializes the Apple toolchain.
if (CMAKE_HOST_APPLE AND (NOT CMAKE_SYSTEM_NAME OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        AND NOT CMAKE_TOOLCHAIN_FILE AND "$ENV{CMAKE_TOOLCHAIN_FILE}" STREQUAL ""
        AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET AND "$ENV{MACOSX_DEPLOYMENT_TARGET}" STREQUAL "")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS deployment target")
endif ()

project(codeeditor VERSION 0.1.0 LANGUAGES NONE)

include("${CMAKE_CURRENT_SOURCE_DIR}/HuxerUIProject.cmake")
if (HUXERUI_PROJECT_PLAN_ONLY)
    return()
endif ()

file(GLOB_RECURSE LIBRARY_SOURCE_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
)

huxerui_add_library(codeeditor
        SOURCES
            ${LIBRARY_SOURCE_FILES}
)

add_library(HuxerUI::CodeEditor ALIAS codeeditor)

if (HUXERUI_LIBRARY_GRAPH_ONLY)
    return()
endif ()

set(SWEETEDITOR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SWEETEDITOR_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(SWEETEDITOR_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SWEETEDITOR_BUILD_ANDROID_JNI OFF CACHE BOOL "" FORCE)
set(SWEETEDITOR_BUILD_APPLE_FRAMEWORK OFF CACHE BOOL "" FORCE)
set(SWEETEDITOR_BUILD_WASM_C_ABI OFF CACHE BOOL "" FORCE)
set(SWEETEDITOR_BUILD_WASM_EMBIND OFF CACHE BOOL "" FORCE)
include(cmake/CodeEditorVendor.cmake)
set(SWEETEDITOR_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3dparty/SweetEditor")
if (NOT EXISTS "${SWEETEDITOR_SOURCE_DIR}/CMakeLists.txt")
    include(FetchContent)
    FetchContent_Declare(codeeditor_sweeteditor_source
        GIT_REPOSITORY https://github.com/FinalScave/SweetEditor.git
        GIT_TAG bd4330e70eda5c71b71ecac54cf7312d82d2eae2
        SOURCE_SUBDIR codeeditor-vendor
        PATCH_COMMAND bash -c "git apply -R --check '${CMAKE_CURRENT_LIST_DIR}/3dparty/patches/sweeteditor-oslog-gcc.patch' 2>/dev/null || git apply '${CMAKE_CURRENT_LIST_DIR}/3dparty/patches/sweeteditor-oslog-gcc.patch'"
    )
    FetchContent_MakeAvailable(codeeditor_sweeteditor_source)
    set(SWEETEDITOR_SOURCE_DIR "${codeeditor_sweeteditor_source_SOURCE_DIR}")
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT TARGET SweetEditor3p::Simdutf)
    include(FetchContent)
    # Build the matching simdutf with the same C++ runtime as the editor.
    set(SIMDUTF_TESTS OFF CACHE BOOL "" FORCE)
    set(SIMDUTF_TOOLS OFF CACHE BOOL "" FORCE)
    set(SIMDUTF_BENCHMARKS OFF CACHE BOOL "" FORCE)
    set(SIMDUTF_FUZZERS OFF CACHE BOOL "" FORCE)
    set(_codeeditor_saved_build_shared_libs "${BUILD_SHARED_LIBS}")
    set(BUILD_SHARED_LIBS OFF)
    FetchContent_Declare(codeeditor_simdutf_source
        GIT_REPOSITORY https://github.com/simdutf/simdutf.git
        GIT_TAG 94fb52ea0219cb37092d8ae294b4702bacf5d8df
    )
    FetchContent_MakeAvailable(codeeditor_simdutf_source)
    set(BUILD_SHARED_LIBS "${_codeeditor_saved_build_shared_libs}")
    add_library(SweetEditor3p::Simdutf ALIAS simdutf)
    target_include_directories(simdutf INTERFACE
        "$<BUILD_INTERFACE:${SWEETEDITOR_SOURCE_DIR}/3dparty/simdutf/include>"
    )
endif ()

codeeditor_add_vendor("${SWEETEDITOR_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/sweeteditor")

target_compile_features(codeeditor PUBLIC cxx_std_20)
target_include_directories(codeeditor PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
)
target_link_libraries(codeeditor PUBLIC SweetEditor::sweeteditor_static)

# windows.h defines min/max macros (breaking std::min/std::max in the vendored
# editor headers) and, through rpcndr.h, a `small` macro (breaking theme.h
# fields such as `float small`). NOMINMAX keeps the std versions intact and
# WIN32_LEAN_AND_MEAN keeps rpcndr.h out; both match upstream configurations.
if (WIN32)
    target_compile_definitions(codeeditor PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
endif ()

if (ANDROID)
    target_link_libraries(codeeditor PRIVATE log)
endif ()

# Build the public API tests by default for standalone desktop builds.
set(HUXERUI_CODEEDITOR_DEFAULT_TESTS OFF)
if (NOT CMAKE_CROSSCOMPILING AND CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(HUXERUI_CODEEDITOR_DEFAULT_TESTS ON)
endif ()
option(HUXERUI_CODEEDITOR_BUILD_TESTS "Build CodeEditor component tests" ${HUXERUI_CODEEDITOR_DEFAULT_TESTS})
if (HUXERUI_CODEEDITOR_BUILD_TESTS)
    if (NOT TARGET HuxerUI::testing)
        find_package(HuxerUI 0.4 CONFIG REQUIRED COMPONENTS testing)
    endif ()
    enable_testing()
    add_executable(codeeditor_tests
            tests/test_main.cpp
            tests/test_options_validation.cpp
            tests/test_editor_controller.cpp
            tests/test_editor_document.cpp
            tests/test_editor_runtime.cpp
            tests/test_editor_theme.cpp
    )
    target_link_libraries(codeeditor_tests
            PRIVATE
                codeeditor
                HuxerUI::testing
                SweetEditor3p::Catch2
    )
    add_test(NAME codeeditor_tests COMMAND codeeditor_tests)
endif ()
