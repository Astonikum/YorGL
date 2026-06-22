cmake_minimum_required(VERSION 3.24)
project(YorGL VERSION 0.1.0 LANGUAGES CXX)

option(YORGL_BUILD_DX11 "Build DirectX 11 backend on Windows" ON)
option(YORGL_BUILD_JNI "Build JNI entry points" ON)
option(YORGL_BUILD_TESTS "Build smoke tests" ON)

add_library(yorgl SHARED
    src/yorgl/api.cpp
    src/yorgl/renderer.cpp
    src/backends/null/null_backend.cpp
)

target_compile_features(yorgl PUBLIC cxx_std_20)
target_include_directories(yorgl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(yorgl PRIVATE YORGL_BUILDING_DLL)

if (WIN32 AND YORGL_BUILD_DX11)
    target_sources(yorgl PRIVATE
        src/backends/dx11/dx11_backend.cpp
        src/backends/dx11/modules/cubemap_renderer.cpp
        src/backends/dx11/modules/gui_renderer.cpp
        src/backends/dx11/modules/sdf_font.cpp
        src/backends/dx11/modules/world_renderer.cpp
    )
    target_compile_definitions(yorgl PRIVATE YORGL_HAS_DX11=1 NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(yorgl PRIVATE d3d11 dxgi d3dcompiler)
endif()

if (YORGL_BUILD_JNI)
    find_package(JNI)
    if (JNI_FOUND)
        target_sources(yorgl PRIVATE src/jni/yorgl_jni.cpp)
        target_include_directories(yorgl PRIVATE ${JNI_INCLUDE_DIRS})
    endif()
endif()

if (MSVC)
    target_compile_options(yorgl PRIVATE /W4 /permissive-)
else()
    target_compile_options(yorgl PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS yorgl EXPORT YorGLTargets)
install(DIRECTORY src/yorgl/ DESTINATION include/yorgl FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
install(EXPORT YorGLTargets NAMESPACE YorGL:: DESTINATION lib/cmake/YorGL)

if (YORGL_BUILD_TESTS)
    enable_testing()
    add_executable(yorgl_smoke tests/smoke.cpp)
    target_link_libraries(yorgl_smoke PRIVATE yorgl)
    add_test(NAME yorgl_smoke COMMAND yorgl_smoke)
endif()
