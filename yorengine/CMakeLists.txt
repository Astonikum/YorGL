add_library(yorengine STATIC
    src/math.cpp
    src/scene.cpp
)

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

install(TARGETS yorengine EXPORT YorEngineTargets)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT YorEngineTargets NAMESPACE YorEngine:: DESTINATION lib/cmake/YorEngine)

if (YORGL_BUILD_TESTS)
    add_executable(yorengine_smoke tests/scene_smoke.cpp)
    target_link_libraries(yorengine_smoke PRIVATE yorengine)
    add_test(NAME yorengine_smoke COMMAND yorengine_smoke)
endif()
