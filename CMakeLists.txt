cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# Set the project name and specify the C++ standard
project(OpenGLProject)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Libraries
include(FetchContent)

# Set compilers (Clang)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Set build type
set(CMAKE_BUILD_TYPE Debug)

# Fetch ImGui
# Using ImGui v1.91.7 for stability. Update tag for newer features.
FetchContent_Declare(
  ImGui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG v1.91.7
)
FetchContent_MakeAvailable(ImGui)

# Define the ImGui library
add_library(ImGui STATIC
    # Core files
    ${imgui_SOURCE_DIR}/imconfig.h
    ${imgui_SOURCE_DIR}/imgui.h
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_internal.h
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp

    # Backends
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3_loader.h

    # stb libraries
    ${imgui_SOURCE_DIR}/imstb_rectpack.h
    ${imgui_SOURCE_DIR}/imstb_textedit.h
    ${imgui_SOURCE_DIR}/imstb_truetype.h

    # For using std::string
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h
)

# Set the include directories for ImGui
target_include_directories(ImGui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imgui_SOURCE_DIR}/misc/cpp
)

# Set source files
set(SOURCES
    src/main.cpp
)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLEW
find_package(GLEW REQUIRED)

# Find GLFW
find_package(glfw3 REQUIRED)

# Include directories for OpenGL, GLEW, GLFW, and ImGui
include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
)

# Create the executable
add_executable(OpenGLProject ${SOURCES})

# Link the necessary libraries
target_link_libraries(OpenGLProject
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    glfw
    ImGui
)

# Enable Clang-Tidy for static analysis
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*")

# Enable all warnings and treat them as errors
target_compile_options(OpenGLProject PRIVATE 
    -Wall
    -Wextra
    -pedantic
    -Werror
)

# Enable clang-format on save or as part of the build process
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enforce the use of modern C++ features (C++20)
target_compile_features(OpenGLProject PRIVATE cxx_std_20)

# Locate clang-format executable
find_program(CLANG_FORMAT_EXECUTABLE clang-format)

# Check if clang-format exists
if (CLANG_FORMAT_EXECUTABLE)
  # Collect all .cpp and .h files
  file(GLOB ALL_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/include/*.h
  )

  add_custom_command(
    TARGET OpenGLProject PRE_BUILD
    COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${ALL_SOURCE_FILES}
    COMMENT "Running clang-format before build"
  )
else()
  message(FATAL_ERROR "clang-format executable not found!")
endif()

