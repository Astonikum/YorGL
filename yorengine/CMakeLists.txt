add_library(yorengine STATIC
    src/math.cpp
    src/render_snapshot.cpp
    src/runtime.cpp
    src/scene.cpp
)

set_target_properties(yorengine PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_compile_features(yorengine PUBLIC cxx_std_20)
target_include_directories(yorengine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if (MSVC)
    target_compile_options(yorengine PRIVATE /W4 /permissive-)
else()
    target_compile_options(yorengine PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(yorengine_api SHARED src/api.cpp)
target_link_libraries(yorengine_api PRIVATE yorengine)
target_compile_features(yorengine_api PUBLIC cxx_std_20)
target_include_directories(yorengine_api PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(yorengine_api PRIVATE YORENGINE_BUILDING_DLL)

if (YORGL_BUILD_JNI)
    find_package(JNI)
    if (JNI_FOUND)
        target_sources(yorengine_api PRIVATE src/jni/yorengine_jni.cpp)
        target_include_directories(yorengine_api PRIVATE ${JNI_INCLUDE_DIRS})
    endif()
endif()

if (MSVC)
    target_compile_options(yorengine_api PRIVATE /W4 /permissive-)
else()
    target_compile_options(yorengine_api PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS yorengine yorengine_api EXPORT YorEngineTargets)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT YorEngineTargets NAMESPACE YorEngine:: DESTINATION lib/cmake/YorEngine)

if (YORGL_BUILD_TESTS)
    add_executable(yorengine_smoke tests/scene_smoke.cpp)
    target_link_libraries(yorengine_smoke PRIVATE yorengine)
    add_test(NAME yorengine_smoke COMMAND yorengine_smoke)

    add_executable(yorengine_runtime_smoke tests/runtime_smoke.cpp)
    target_link_libraries(yorengine_runtime_smoke PRIVATE yorengine)
    add_test(NAME yorengine_runtime_smoke COMMAND yorengine_runtime_smoke)

    add_executable(yorengine_render_snapshot_smoke tests/render_snapshot_smoke.cpp)
    target_link_libraries(yorengine_render_snapshot_smoke PRIVATE yorengine)
    add_test(NAME yorengine_render_snapshot_smoke COMMAND yorengine_render_snapshot_smoke)

    add_executable(yorengine_api_smoke tests/api_smoke.cpp)
    target_link_libraries(yorengine_api_smoke PRIVATE yorengine_api)
    add_test(NAME yorengine_api_smoke COMMAND yorengine_api_smoke)
endif()
