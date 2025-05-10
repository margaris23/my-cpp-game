cmake_minimum_required(VERSION 3.24...3.30)
project(my-cpp-game)

include(FetchContent)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
set(RAYLIB_VERSION 5.0)

FetchContent_Declare(
  raylib
  DOWNLOAD_EXTRACT_TIMESTAMP OFF
  URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
      FIND_PACKAGE_ARGS)

FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281) # 10.2.1
FetchContent_MakeAvailable(fmt)

# Print debug information
get_target_property(FMT_INCLUDE_DIRS fmt INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "fmt include directories: ${FMT_INCLUDE_DIRS}")

# Our Project
add_executable(${PROJECT_NAME})
add_subdirectory(src)
add_compile_options(-g -Wall)


target_link_libraries(my-cpp-game fmt::fmt)

set_target_properties(
  ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                             ${CMAKE_BINARY_DIR}/${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY
                                             $<TARGET_FILE_DIR:${PROJECT_NAME}>)

if("${PLATFORM}" STREQUAL "Web")
  add_custom_command(
    TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/resources
      $<TARGET_FILE_DIR:${PROJECT_NAME}>/../resources)
  # DEPENDS ${PROJECT_NAME}
else()
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/resources
      $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)
  # DEPENDS ${PROJECT_NAME}
endif()

# set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib)

# Web Configurations
if("${PLATFORM}" STREQUAL "Web")
  # Tell Emscripten to build an example.html file.
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
  target_link_options(${PROJECT_NAME} PUBLIC -sUSE_GLFW=3 PUBLIC --preload-file
                      resources)
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if(APPLE)
  target_link_libraries(${PROJECT_NAME} "-framework IOKit")
  target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
  target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
