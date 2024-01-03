cmake_minimum_required(VERSION 3.15)

project(CONSOLE_APP_EXAMPLE VERSION 0.0.1)

set (BaseTargetName ConsoleAppExample)

# Adds all the module sources so they appear correctly in the IDE
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

# Set any of these to "ON" if you want to build one of the juce examples or extras (Projucer/AudioPluginHost, etc):
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)
option(JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)

message(STATUS "THIS_JUCE_DIRECTORY: ${THIS_JUCE_DIRECTORY}")
add_subdirectory(${THIS_JUCE_DIRECTORY} JUCE EXCLUDE_FROM_ALL)

set(USE_ATL OFF)
set(USE_SANDBOX OFF)
message(STATUS "THIS_CEF_DIRECTORY: ${THIS_CEF_DIRECTORY}")
add_subdirectory(${THIS_CEF_DIRECTORY} CEF EXCLUDE_FROM_ALL)
set(CEF_ROOT ${THIS_CEF_DIRECTORY})
include(${THIS_CEF_DIRECTORY}/cmake/FindCEF.cmake)
include(${THIS_CEF_DIRECTORY}/cmake/cef_variables.cmake)

set(OS_WINDOWS TRUE)
add_subdirectory(cefsimple)
message(STATUS "CEFSIMPLE_SRCS: ${CEFSIMPLE_SRCS}")

juce_add_console_app(${BaseTargetName}
    PRODUCT_NAME "Console App Example")

juce_generate_juce_header(${BaseTargetName})

target_sources(${BaseTargetName}
    PRIVATE
        Main.cpp
        MainComponent.cpp
        MainComponent.h
        CefJuceAudioSink.cpp
        CefJuceAudioSink.h
        DSPModulePluginDemo.h
        ${CEFSIMPLE_SRCS}
        )

target_compile_definitions(${BaseTargetName}
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_console_app` call
        JUCE_USE_CURL=0
        ${CEF_COMPILER_DEFINES}
        $<IF:$<CONFIG:Debug>,${CEF_COMPILER_DEFINES_DEBUG},${CEF_COMPILER_DEFINES_RELEASE}>)    # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_console_app` call
    
# Use PUBLIC to effect linked target like VST3, Standalone, CLAP, ...
target_compile_features(${BaseTargetName}
    PUBLIC
        cxx_std_17)

target_include_directories(${BaseTargetName}
    PRIVATE
        ${THIS_CEF_DIRECTORY})

add_dependencies(${BaseTargetName} libcef_dll_wrapper)

target_link_directories(${BaseTargetName}
    PUBLIC
        "${THIS_CEF_DIRECTORY}/$<IF:$<CONFIG:Debug>,Debug,Release>")

target_link_libraries(${BaseTargetName}
    PRIVATE
        # ConsoleAppData            # If you'd created a binary data target, you'd link to it here
        juce::juce_core
        juce::juce_audio_utils
        juce::juce_gui_extra
        juce::juce_dsp
        libcef
        libcef_dll_wrapper
        d3d11.lib
        glu32.lib
        imm32.lib
        opengl32.lib
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

#
# Windows configuration.
#

#
set(CEF_TARGET ${BaseTargetName})

get_target_property(CEF_TARGET_OUT_DIR ${CEF_TARGET} LIBRARY_OUTPUT_DIRECTORY)

set(OS_WINDOWS TRUE)
message(STATUS "[${BaseTargetName}] CEF_TARGET: ${CEF_TARGET}")
message(STATUS "[${BaseTargetName}] CEF_TARGET_OUT_DIR: ${CEF_TARGET_OUT_DIR}")
message(STATUS "[${BaseTargetName}] CEF_BINARY_FILES: ${CEF_BINARY_FILES}")
message(STATUS "[${BaseTargetName}] CEF_BINARY_DIR: ${CEF_BINARY_DIR}")
message(STATUS "[${BaseTargetName}] CEF_RESOURCE_FILES: ${CEF_RESOURCE_FILES}")
message(STATUS "[${BaseTargetName}] CEF_RESOURCE_DIR: ${CEF_RESOURCE_DIR}")

if(OS_WINDOWS)
  # Executable target.
#   add_executable(${CEF_TARGET} WIN32 ${CEFSIMPLE_SRCS})
#   add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
#   SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
#   target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

#   if(USE_SANDBOX)
#     # Logical target used to link the cef_sandbox library.
#     ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
#     target_link_libraries(${CEF_TARGET} cef_sandbox_lib ${CEF_SANDBOX_STANDARD_LIBS})
#   endif()

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
endif()