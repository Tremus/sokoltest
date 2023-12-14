cmake_minimum_required(VERSION 3.7)
project(sokoltest)

set(CMAKE_CXX_STANDARD 17)

# NOTE: sokol uses global variables
# Forcing the app to be single insance is recommended
# On macos, set the LSMultipleInstancesProhibited
# field in the info.plist
# For Windows its a little hacky, but simple enough
# https://stackoverflow.com/questions/8799646/preventing-multiple-instances-of-my-application

enable_language(OBJC)

add_executable(nuklear_test src/nuklear-sapp.m)
target_link_libraries(nuklear_test PRIVATE "-framework Cocoa -framework QuartzCore -framework Metal -framework MetalKit")
target_include_directories(nuklear_test PRIVATE src)

# builds faster by about 200ms
add_executable(cimgui_test
    src/cimgui-sapp.m
    src/cimgui/cimgui.cpp
    src/cimgui/imgui/imgui.cpp
    src/cimgui/imgui/imgui_demo.cpp
    src/cimgui/imgui/imgui_draw.cpp
    src/cimgui/imgui/imgui_tables.cpp
    src/cimgui/imgui/imgui_widgets.cpp
    )
target_link_libraries(cimgui_test PRIVATE "-framework Cocoa -framework QuartzCore -framework Metal -framework MetalKit")
target_include_directories(cimgui_test PRIVATE src)