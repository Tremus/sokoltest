cmake_minimum_required(VERSION 3.7)
project(sokoltest)

set(CMAKE_CXX_STANDARD 17)

# NOTE: sokol uses global variables
# Forcing the app to be single insance is recommended
# For Windows its a little hacky, but simple enough
# https://stackoverflow.com/questions/8799646/preventing-multiple-instances-of-my-application

enable_language(OBJC)

function(create_bundle NAME)
    set(options OPTIONAL)
    set(oneValueArgs ONEVALUE)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(XBUNDLE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    add_executable(${NAME} MACOSX_BUNDLE ${XBUNDLE_SOURCES})
    target_link_libraries(${NAME} PRIVATE "-framework Cocoa -framework QuartzCore -framework Metal -framework MetalKit")
    target_include_directories(${NAME} PRIVATE src)
    set_target_properties(sokolnuklear PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_GUI_IDENTIFIER com.${NAME}.test
        MACOSX_BUNDLE_BUNDLE_NAME com.${NAME}.test
        MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION}
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/bundle.plist
    )
    file(TOUCH_NOCREATE ${CMAKE_BINARY_DIR}/${NAME}.app/Contents/PkgInfo)
    file(WRITE ${CMAKE_BINARY_DIR}/${NAME}.app/Contents/PkgInfo "APPL????")
endfunction()

# (MacOS)
# cold build time range 671-780ms, median: 680ms
# bundle size 486kb (Release, ARM)
create_bundle(sokolnuklear
    SOURCES
        src/nuklear-sapp.c
        src/nuklear/nuklear.c
        src/sokol_app.m
        src/sokol_gfx.m
)

# (MacOS)
# cold build time range 950-1050ms, median: 1s
# bundle size 1340kb (Release, ARM)
create_bundle(sokolcimgui SOURCES
    src/cimgui-sapp.c
    src/sokol_app.m
    src/sokol_gfx.m
    src/cimgui/cimgui.cpp
    src/cimgui/imgui/imgui.cpp
    src/cimgui/imgui/imgui_demo.cpp
    src/cimgui/imgui/imgui_draw.cpp
    src/cimgui/imgui/imgui_tables.cpp
    src/cimgui/imgui/imgui_widgets.cpp
    )