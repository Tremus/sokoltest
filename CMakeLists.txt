cmake_minimum_required(VERSION 3.7)
project(sokoltest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)

# RtMidi examples
add_executable(midiprobe src/rtmidi/midiprobe.cpp src/rtmidi/RtMidi.cpp)
target_include_directories(midiprobe PRIVATE src/rtmidi/)
add_executable(qmidiin src/rtmidi/qmidiin.c src/rtmidi/rtmidi_c.cpp src/rtmidi/RtMidi.cpp)
target_include_directories(qmidiin PRIVATE src/rtmidi/)

set(MIDI_LIBS "")
if (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND MIDI_LIBS "-lwinmm")
elseif(APPLE)
    target_link_libraries(APPEND MIDI_LIBS "-framework CoreMIDI -framework CoreAudio -framework CoreFoundation")
endif()
target_link_libraries(midiprobe PRIVATE ${MIDI_LIBS})
target_link_libraries(qmidiin PRIVATE ${MIDI_LIBS})

# Standalone apps
set(PLATFORM_SOURCES "")
if(APPLE)
    enable_language(OBJC)
    list(APPEND PLATFORM_SOURCES src/sokol_app.m src/sokol_gfx.m src/sokol_audio.m)
else()
    list(APPEND PLATFORM_SOURCES src/sokol_app.c src/sokol_gfx.c src/sokol_audio.c)
endif()

function(create_app NAME)
    set(options OPTIONAL)
    set(oneValueArgs ONEVALUE)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(XBUNDLE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    if (APPLE)
        add_executable(${NAME} MACOSX_BUNDLE ${XBUNDLE_SOURCES})
        target_link_libraries(${NAME} PRIVATE "-framework Cocoa -framework QuartzCore -framework Metal -framework MetalKit -framework AudioToolbox")
        target_include_directories(${NAME} PRIVATE src)
        set_target_properties(${NAME} PROPERTIES
            BUNDLE True
            MACOSX_BUNDLE_GUI_IDENTIFIER com.${NAME}.test
            MACOSX_BUNDLE_BUNDLE_NAME com.${NAME}.test
            MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION}
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/bundle.plist
        )
        file(TOUCH_NOCREATE ${CMAKE_BINARY_DIR}/${NAME}.app/Contents/PkgInfo)
        file(WRITE ${CMAKE_BINARY_DIR}/${NAME}.app/Contents/PkgInfo "APPL????")
    elseif(WIN32)
        # so elegant
        add_executable(${NAME} ${XBUNDLE_SOURCES})
    endif()
endfunction()

# Note the stats below were calculated before I added sokol_audio.h and RtMidi
# to this project, which have increased compile time & binary size.
# The original code was from boilerplate code found here:
# https://github.com/floooh/sokol-samples

# Debug cold build time: range 671-780ms, median: 680ms (MacBook Air M1)
# Release bundle size: 486kb (MacOS ARM), 402kb (Windows x86)
create_app(sokolnuklear
    SOURCES
        src/nuklear-sapp.c
        ${PLATFORM_SOURCES}
        src/thread.c
        src/nuklear/nuklear.c
        src/rtmidi/rtmidi_c.cpp
        src/rtmidi/RtMidi.cpp
)

# Debug cold build time: range 950-1050ms, median: 1s (MacBook Air M1)
# Release bundle size: 1340kb (MacOS ARM), 1238kb (Windows x86)
create_app(sokolcimgui
    SOURCES
        src/cimgui-sapp.c
        ${PLATFORM_SOURCES}
        src/cimgui/cimgui.cpp
        src/cimgui/imgui/imgui.cpp
        src/cimgui/imgui/imgui_demo.cpp
        src/cimgui/imgui/imgui_draw.cpp
        src/cimgui/imgui/imgui_tables.cpp
        src/cimgui/imgui/imgui_widgets.cpp
)