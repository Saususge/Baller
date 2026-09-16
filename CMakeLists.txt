cmake_minimum_required(VERSION 3.24)

project(baller VERSION 0.1.0 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "baller currently targets Windows only.")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "baller requires a 64-bit build. Select an x64 preset.")
endif()

find_package(Vulkan 1.4 REQUIRED)

# Slang is a build tool; no Slang runtime library is linked into baller.
# Override with -DBALLER_SLANGC_EXECUTABLE=... for a standalone compiler.
find_program(BALLER_SLANGC_EXECUTABLE
    NAMES slangc
    HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin"
    DOC "Path to the Slang shader compiler (slangc.exe)"
)
if(NOT BALLER_SLANGC_EXECUTABLE)
    message(FATAL_ERROR
        "slangc was not found. Install the Vulkan SDK Slang component or set "
        "BALLER_SLANGC_EXECUTABLE to the full path to slangc.exe.")
endif()
message(STATUS "Slang compiler: ${BALLER_SLANGC_EXECUTABLE}")

find_package(glm CONFIG QUIET)
if(NOT glm_FOUND)
    include(FetchContent)
    FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glm)
endif()

add_executable(baller
    src/main.cpp
    src/platform/platform.hpp
    src/platform/platform_win32.cpp
    src/renderer/vk_instance.hpp
    src/renderer/vk_instance.cpp
    src/renderer/vk_queue_families.hpp
    src/renderer/vk_queue_families.cpp
    src/renderer/vk_device.hpp
    src/renderer/vk_device.cpp
)
target_compile_features(baller PRIVATE cxx_std_20)
set_target_properties(baller PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(baller PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(baller PRIVATE
    WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE VK_USE_PLATFORM_WIN32_KHR
    GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE
)
if(MSVC)
    target_compile_options(baller PRIVATE /utf-8 /W4 /permissive-)
endif()
target_link_libraries(baller PRIVATE Vulkan::Vulkan glm::glm user32 kernel32 gdi32)

# Separate configurations for both Ninja and Visual Studio generators.
set_target_properties(baller PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:baller>"
)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT baller)

include(cmake/Slang.cmake)
baller_add_slang_shader(shaders/triangle.slang vertexMain vertex triangle.vert)
baller_add_slang_shader(shaders/triangle.slang fragmentMain fragment triangle.frag)
add_custom_target(baller_shaders DEPENDS ${BALLER_SHADER_OUTPUTS})
add_dependencies(baller baller_shaders)

# Show editable shaders in Visual Studio without compiling them as C++.
set_source_files_properties(shaders/triangle.slang PROPERTIES HEADER_FILE_ONLY TRUE)
target_sources(baller PRIVATE shaders/triangle.slang)
source_group("Shaders" FILES shaders/triangle.slang)
