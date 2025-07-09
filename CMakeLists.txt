cmake_minimum_required(VERSION 3.15)
project(WIMEEditorCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CPM.cmake for package management
include(FetchContent)
FetchContent_Declare(
  CPM
  GIT_REPOSITORY https://github.com/cpm-cmake/CPM.cmake.git
  GIT_TAG origin/master
)
FetchContent_MakeAvailable(CPM)

# Add GLFW directly with specific commit
CPMAddPackage(
  NAME glfw
  GITHUB_REPOSITORY glfw/glfw
  GIT_TAG 3.3.9
  OPTIONS "GLFW_BUILD_EXAMPLES OFF" "GLFW_BUILD_TESTS OFF" "GLFW_BUILD_DOCS OFF"
)
# Add ImGui
CPMAddPackage(
  NAME imgui
  GITHUB_REPOSITORY ocornut/imgui
  GIT_TAG docking
)

# portable-file-dialogs is included as a single header file

# Configure ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui_lib STATIC ${IMGUI_SOURCES})
target_include_directories(imgui_lib PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)

# Add source files
set(SOURCES
    main.cpp
    src/Game.cpp
    src/EditorSettings.cpp
    src/FileDialog.cpp
    src/BinaryFile.cpp
    src/ResourceLoader.cpp
    src/EditorUI.cpp
    src/GameInfoWindow.cpp
    src/ResourceBrowserWindow.cpp
    src/PropertiesWindow.cpp
    src/ResourceViewers.cpp
    src/PreviewWindow.cpp
    src/ConsoleWindow.cpp
)

add_executable(WIMEEditorCPP ${SOURCES})

target_include_directories(WIMEEditorCPP PRIVATE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends include src)
target_link_libraries(WIMEEditorCPP PRIVATE glfw imgui_lib)

# For Windows: link OpenGL
if (WIN32)
  target_link_libraries(WIMEEditorCPP PRIVATE opengl32)
endif()
