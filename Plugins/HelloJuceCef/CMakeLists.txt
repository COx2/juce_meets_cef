project(HelloJuceCef VERSION 0.0.1)

set (BaseTargetName HelloJuceCef)

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
add_subdirectory(cefclient)
message(STATUS "CEFCLIENT_SRCS: ${CEFCLIENT_SRCS}")

juce_add_plugin("${BaseTargetName}"
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME "COx2"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Cox2
    PLUGIN_CODE Jcef
    FORMATS Standalone VST3 AU)

juce_generate_juce_header(${BaseTargetName})

target_sources(${BaseTargetName}
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/CEF/CEFRunner.cpp
        Source/CEF/CEFRunner.h
        Source/CEF/simple_app.cc
        Source/CEF/simple_app.h
        Source/CEF/simple_handler.cc
        Source/CEF/simple_handler.h
        cefclient_juce.cpp
        cefclient_juce.h
        ${CEFCLIENT_SRCS}
        )

target_compile_definitions(${BaseTargetName}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        ${CEF_COMPILER_DEFINES}
        $<IF:$<CONFIG:Debug>,${CEF_COMPILER_DEFINES_DEBUG},${CEF_COMPILER_DEFINES_RELEASE}>)

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
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_audio_devices
        libcef
        libcef_dll_wrapper
        d3d11.lib
        glu32.lib
        imm32.lib
        opengl32.lib
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

message(STATUS "[${BaseTargetName}] Binary directory: ${CMAKE_CURRENT_BINARY_DIR}")

#
# Windows configuration.
#

#
set(CEF_TARGET ${BaseTargetName}_Standalone)

get_target_property(CEF_TARGET_OUT_DIR ${CEF_TARGET} LIBRARY_OUTPUT_DIRECTORY)
# set(CEF_TARGET_OUT_DIR ${BaseTargetName}_Standalone)

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
