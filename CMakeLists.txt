cmake_minimum_required(VERSION 3.15)

# Project configuration
project(OcctImgui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 20)
# Include directories
find_package(glfw3 REQUIRED)
find_package(OpenCASCADE REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/*.h"
)

file(GLOB_RECURSE SKETCH_CORE_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/Data/*.cpp"
        "${CMAKE_SOURCE_DIR}/Demo/*.cpp"
        "${CMAKE_SOURCE_DIR}/DependencyGraph/*.cpp"
)

add_library(SketchCore STATIC
        ${SKETCH_CORE_SOURCES}
)

target_include_directories(SketchCore PUBLIC
        ${CMAKE_SOURCE_DIR}
)

# Link libraries
target_link_libraries(SketchCore
        PUBLIC TKernel TKMath TKG2d TKG3d TKGeomBase TKGeomAlgo TKBRep TKTopAlgo TKPrim TKMesh TKService TKOpenGl TKV3d
        glfw
)

target_compile_features(SketchCore PUBLIC cxx_std_20)


# Exclude CMake-generated files from the SOURCES list
list(FILTER SOURCES EXCLUDE REGEX ".*CMakeFiles.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/test/.*")

# Add executable target
add_executable(OcctImgui ${SOURCES})

target_include_directories(OcctImgui PRIVATE
    ${CMAKE_SOURCE_DIR}
)

# Link libraries
target_link_libraries(OcctImgui PRIVATE
        SketchCore
)

target_compile_options(OcctImgui PRIVATE
        $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-g>
        $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:Clang>>:-g>
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-O3>
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:-O3>
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/O2>
)

target_compile_definitions(OcctImgui PRIVATE
        $<$<CONFIG:Debug>:DG_ENABLE_TRACE>
)

target_link_directories(OcctImgui PRIVATE
    $<$<CONFIG:Debug>:${DEBUG_LIBS}>
    $<$<CONFIG:Release>:${RELEASE_LIBS}>
)

# Debug environment variables (for Windows)
if(WIN32)
    set(DEBUG_ENVS "path=%path%;D:/OpenCASCADE-7.7.0/opencascade-7.7.0/win64/vc14/bind")
    set(RELEASE_ENVS "path=%path%;D:/OpenCASCADE-7.7.0/opencascade-7.7.0/win64/vc14/bin")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(OcctImgui PROPERTIES VS_DEBUGGER_ENVIRONMENT "${DEBUG_ENVS}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_target_properties(OcctImgui PROPERTIES VS_DEBUGGER_ENVIRONMENT "${RELEASE_ENVS}")
    endif()
endif()

include(CTest)

if (BUILD_TESTING)
    include(FetchContent)
    find_package(OpenCASCADE REQUIRED)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    )
    # 避免 Windows 下 GoogleTest 修改父项目的运行库配置
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    add_executable(SketchRuntimeTest
            test/SketchRuntimeTest.cpp
    )


    target_link_libraries(SketchRuntimeTest PRIVATE
            SketchCore
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(SketchRuntimeTest)
endif ()
