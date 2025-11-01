cmake_minimum_required(VERSION 3.20)
project(MatSlide)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(dependencies/glfw)

set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "")

add_executable(MatSlideApp 
    src/main.cpp
    src/core/Application.cpp
    src/rendering/BackgroundPass.cpp
    src/rendering/MeshPass.cpp
)

target_link_libraries(MatSlideApp glfw)

target_include_directories(MatSlideApp PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm
)
target_include_directories(MatSlideApp PRIVATE 
    src/core
    src/rendering
)
target_sources(MatSlideApp PRIVATE
    dependencies/glad/src/glad.c
)


add_executable(Convertor src/utils/convert_to_merl.cpp dependencies/brdf-loader/powitacq_rgb.inl)
target_include_directories(Convertor PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/brdf-loader
)

# add_executable(Objloader src/utils/objLoader.cpp)