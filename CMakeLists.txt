cmake_minimum_required(VERSION 3.20)
project(MatSlide)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(dependencies/glfw)

add_executable(MatSlideApp 
    src/main.cpp
    src/core/Application.cpp
    src/core/ParameterManager.cpp
    src/core/ExrLoader.cpp
    src/rendering/BackgroundPass.cpp
    src/rendering/EnvMapPass.cpp
    src/rendering/MeshPass.cpp
    src/rendering/MerlPass.cpp
    src/rendering/IblMerlPass.cpp
    src/inference/ONNXRuntimeWrapper.cpp
    src/inference/BrdfProvider.cpp
)

set(ONNXRUNTIME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/onnxruntime-win-x64-1.24.4")

target_link_libraries(MatSlideApp glfw "${ONNXRUNTIME_ROOT}/lib/onnxruntime.lib")

target_include_directories(MatSlideApp PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm
    ${ONNXRUNTIME_ROOT}/include
)
target_include_directories(MatSlideApp PRIVATE 
    src/core
    src/rendering
    src/inference
)

if(MSVC)
    target_compile_options(MatSlideApp PRIVATE /W3 /MP /utf-8)
    target_compile_definitions(MatSlideApp PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MatSlideApp)
endif()

set(IMGUI_SOURCES
    dependencies/imgui/imgui.cpp
    dependencies/imgui/imgui_draw.cpp
    dependencies/imgui/imgui_tables.cpp
    dependencies/imgui/imgui_widgets.cpp
    dependencies/imgui/backends/imgui_impl_glfw.cpp
    dependencies/imgui/backends/imgui_impl_opengl3.cpp
)

target_sources(MatSlideApp PRIVATE
    ${IMGUI_SOURCES}
)

target_include_directories(MatSlideApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui/backends
)

target_sources(MatSlideApp PRIVATE
    dependencies/glad/src/glad.c
    dependencies/tinyexr/deps/miniz/miniz.c
)

target_include_directories(MatSlideApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/tinyexr
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/tinyexr/deps/miniz
)

add_custom_command(TARGET MatSlideApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ONNXRUNTIME_ROOT}/lib/onnxruntime.dll"
    $<TARGET_FILE_DIR:MatSlideApp>
)


add_executable(Convertor EXCLUDE_FROM_ALL src/utils/convert_to_merl.cpp dependencies/brdf-loader/powitacq_rgb.inl)
target_include_directories(Convertor PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/brdf-loader
)

# add_executable(Objloader src/utils/objLoader.cpp)

add_executable(InferToBinary EXCLUDE_FROM_ALL src/utils/infer_to_binary.cpp src/inference/ONNXRuntimeWrapper.cpp)
target_link_libraries(InferToBinary "${ONNXRUNTIME_ROOT}/lib/onnxruntime.lib")
target_include_directories(InferToBinary PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${ONNXRUNTIME_ROOT}/include
)
target_include_directories(InferToBinary PRIVATE
    src/core
    src/inference
)

if(MSVC)
    target_compile_options(Convertor PRIVATE /W3 /MP /utf-8)
    target_compile_definitions(Convertor PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    target_compile_options(InferToBinary PRIVATE /W3 /MP /utf-8)
    target_compile_definitions(InferToBinary PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()

add_custom_command(TARGET InferToBinary POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ONNXRUNTIME_ROOT}/lib/onnxruntime.dll"
    $<TARGET_FILE_DIR:InferToBinary>
)