cmake_minimum_required(VERSION 3.20)
project(week9_camera LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(week9 main.cpp)
if(TARGET glfw)
    target_link_libraries(week9 PRIVATE glfw)
elseif(TARGET glfw3::glfw)
    target_link_libraries(week9 PRIVATE glfw3::glfw)
endif()
if(TARGET glad::glad)
    target_link_libraries(week9 PRIVATE glad::glad)
elseif(TARGET glad)
    target_link_libraries(week9 PRIVATE glad)
endif()
target_link_libraries(week9 PRIVATE glm::glm OpenGL::GL)

