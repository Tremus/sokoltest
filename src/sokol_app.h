#if defined(SOKOL_IMPL) && !defined(SOKOL_APP_IMPL)
#define SOKOL_APP_IMPL
#endif
#ifndef SOKOL_APP_INCLUDED
/*
    sokol_app.h -- cross-platform application wrapper

    Project URL: https://github.com/floooh/sokol

    Do this:
        #define SOKOL_IMPL or
        #define SOKOL_APP_IMPL
    before you include this file in *one* C or C++ file to create the
    implementation.

    In the same place define one of the following to select the 3D-API
    which should be initialized by sokol_app.h (this must also match
    the backend selected for sokol_gfx.h if both are used in the same
    project):

        #define SOKOL_GLCORE33
        #define SOKOL_GLES3
        #define SOKOL_D3D11
        #define SOKOL_METAL
        #define SOKOL_WGPU

    Optionally provide the following defines with your own implementations:

        SOKOL_ASSERT(c)     - your own assert macro (default: assert(c))
        SOKOL_UNREACHABLE() - a guard macro for unreachable code (default: assert(false))
        SOKOL_WIN32_FORCE_MAIN  - define this on Win32 to use a main() entry point instead of WinMain
        SOKOL_NO_ENTRY      - define this if sokol_app.h shouldn't "hijack" the main() function
        SOKOL_APP_API_DECL  - public function declaration prefix (default: extern)
        SOKOL_API_DECL      - same as SOKOL_APP_API_DECL
        SOKOL_API_IMPL      - public function implementation prefix (default: -)

    Optionally define the following to force debug checks and validations
    even in release mode:

        SOKOL_DEBUG         - by default this is defined if _DEBUG is defined

    If sokol_app.h is compiled as a DLL, define the following before
    including the declaration or implementation:

        SOKOL_DLL

    On Windows, SOKOL_DLL will define SOKOL_APP_API_DECL as __declspec(dllexport)
    or __declspec(dllimport) as needed.

    On Linux, SOKOL_GLCORE33 can use either GLX or EGL.
    GLX is default, set SOKOL_FORCE_EGL to override.

    For example code, see https://github.com/floooh/sokol-samples/tree/master/sapp

    Portions of the Windows and Linux GL initialization, event-, icon- etc... code
    have been taken from GLFW (http://www.glfw.org/)

    iOS onscreen keyboard support 'inspired' by libgdx.

    Link with the following system libraries:

    - on macOS with Metal: Cocoa, QuartzCore, Metal, MetalKit
    - on macOS with GL: Cocoa, QuartzCore, OpenGL
    - on iOS with Metal: Foundation, UIKit, Metal, MetalKit
    - on iOS with GL: Foundation, UIKit, OpenGLES, GLKit
    - on Linux with EGL: X11, Xi, Xcursor, EGL, GL (or GLESv2), dl, pthread, m(?)
    - on Linux with GLX: X11, Xi, Xcursor, GL, dl, pthread, m(?)
    - on Android: GLESv3, EGL, log, android
    - on Windows with the MSVC or Clang toolchains: no action needed, libs are defined in-source via pragma-comment-lib
    - on Windows with MINGW/MSYS2 gcc: compile with '-mwin32' so that _WIN32 is defined
        - link with the following libs: -lkernel32 -luser32 -lshell32
        - additionally with the GL backend: -lgdi32
        - additionally with the D3D11 backend: -ld3d11 -ldxgi

    On Linux, you also need to use the -pthread compiler and linker option, otherwise weird
    things will happen, see here for details: https://github.com/floooh/sokol/issues/376

    On macOS and iOS, the implementation must be compiled as Objective-C.

    FEATURE OVERVIEW
    ================
    sokol_app.h provides a minimalistic cross-platform API which
    implements the 'application-wrapper' parts of a 3D application:

    - a common application entry function
    - creates a window and 3D-API context/device with a 'default framebuffer'
    - makes the rendered frame visible
    - provides keyboard-, mouse- and low-level touch-events
    - platforms: MacOS, iOS, HTML5, Win32, Linux/RaspberryPi, Android
    - 3D-APIs: Metal, D3D11, GL3.2, GLES3, WebGL, WebGL2

    FEATURE/PLATFORM MATRIX
    =======================
                        | Windows | macOS | Linux |  iOS  | Android |  HTML5
    --------------------+---------+-------+-------+-------+---------+--------
    gl 3.x              | YES     | YES   | YES   | ---   | ---     |  ---
    gles3/webgl2        | ---     | ---   | YES(2)| YES   | YES     |  YES
    metal               | ---     | YES   | ---   | YES   | ---     |  ---
    d3d11               | YES     | ---   | ---   | ---   | ---     |  ---
    KEY_DOWN            | YES     | YES   | YES   | SOME  | TODO    |  YES
    KEY_UP              | YES     | YES   | YES   | SOME  | TODO    |  YES
    CHAR                | YES     | YES   | YES   | YES   | TODO    |  YES
    MOUSE_DOWN          | YES     | YES   | YES   | ---   | ---     |  YES
    MOUSE_UP            | YES     | YES   | YES   | ---   | ---     |  YES
    MOUSE_SCROLL        | YES     | YES   | YES   | ---   | ---     |  YES
    MOUSE_MOVE          | YES     | YES   | YES   | ---   | ---     |  YES
    MOUSE_ENTER         | YES     | YES   | YES   | ---   | ---     |  YES
    MOUSE_LEAVE         | YES     | YES   | YES   | ---   | ---     |  YES
    TOUCHES_BEGAN       | ---     | ---   | ---   | YES   | YES     |  YES
    TOUCHES_MOVED       | ---     | ---   | ---   | YES   | YES     |  YES
    TOUCHES_ENDED       | ---     | ---   | ---   | YES   | YES     |  YES
    TOUCHES_CANCELLED   | ---     | ---   | ---   | YES   | YES     |  YES
    RESIZED             | YES     | YES   | YES   | YES   | YES     |  YES
    ICONIFIED           | YES     | YES   | YES   | ---   | ---     |  ---
    RESTORED            | YES     | YES   | YES   | ---   | ---     |  ---
    FOCUSED             | YES     | YES   | YES   | ---   | ---     |  YES
    UNFOCUSED           | YES     | YES   | YES   | ---   | ---     |  YES
    SUSPENDED           | ---     | ---   | ---   | YES   | YES     |  TODO
    RESUMED             | ---     | ---   | ---   | YES   | YES     |  TODO
    QUIT_REQUESTED      | YES     | YES   | YES   | ---   | ---     |  YES
    IME                 | TODO    | TODO? | TODO  | ???   | TODO    |  ???
    key repeat flag     | YES     | YES   | YES   | ---   | ---     |  YES
    windowed            | YES     | YES   | YES   | ---   | ---     |  YES
    fullscreen          | YES     | YES   | YES   | YES   | YES     |  ---
    mouse hide          | YES     | YES   | YES   | ---   | ---     |  YES
    mouse lock          | YES     | YES   | YES   | ---   | ---     |  YES
    set cursor type     | YES     | YES   | YES   | ---   | ---     |  YES
    screen keyboard     | ---     | ---   | ---   | YES   | TODO    |  YES
    swap interval       | YES     | YES   | YES   | YES   | TODO    |  YES
    high-dpi            | YES     | YES   | TODO  | YES   | YES     |  YES
    clipboard           | YES     | YES   | TODO  | ---   | ---     |  YES
    MSAA                | YES     | YES   | YES   | YES   | YES     |  YES
    drag'n'drop         | YES     | YES   | YES   | ---   | ---     |  YES
    window icon         | YES     | YES(1)| YES   | ---   | ---     |  YES

    (1) macOS has no regular window icons, instead the dock icon is changed
    (2) supported with EGL only (not GLX)

    STEP BY STEP
    ============
    --- Add a sokol_main() function to your code which returns a sapp_desc structure
        with initialization parameters and callback function pointers. This
        function is called very early, usually at the start of the
        platform's entry function (e.g. main or WinMain). You should do as
        little as possible here, since the rest of your code might be called
        from another thread (this depends on the platform):

            sapp_desc sokol_main(int argc, char* argv[]) {
                return (sapp_desc) {
                    .width = 640,
                    .height = 480,
                    .init_cb = my_init_func,
                    .frame_cb = my_frame_func,
                    .cleanup_cb = my_cleanup_func,
                    .event_cb = my_event_func,
                    ...
                };
            }

        To get any logging output in case of errors you need to provide a log
        callback. The easiest way is via sokol_log.h:

            #include "sokol_log.h"

            sapp_desc sokol_main(int argc, char* argv[]) {
                return (sapp_desc) {
                    ...
                    .logger.func = slog_func,
                };
            }

        There are many more setup parameters, but these are the most important.
        For a complete list search for the sapp_desc structure declaration
        below.

        DO NOT call any sokol-app function from inside sokol_main(), since
        sokol-app will not be initialized at this point.

        The .width and .height parameters are the preferred size of the 3D
        rendering canvas. The actual size may differ from this depending on
        platform and other circumstances. Also the canvas size may change at
        any time (for instance when the user resizes the application window,
        or rotates the mobile device). You can just keep .width and .height
        zero-initialized to open a default-sized window (what "default-size"
        exactly means is platform-specific, but usually it's a size that covers
        most of, but not all, of the display).

        All provided function callbacks will be called from the same thread,
        but this may be different from the thread where sokol_main() was called.

        .init_cb (void (*)(void))
            This function is called once after the application window,
            3D rendering context and swap chain have been created. The
            function takes no arguments and has no return value.
        .frame_cb (void (*)(void))
            This is the per-frame callback, which is usually called 60
            times per second. This is where your application would update
            most of its state and perform all rendering.
        .cleanup_cb (void (*)(void))
            The cleanup callback is called once right before the application
            quits.
        .event_cb (void (*)(const sapp_event* event))
            The event callback is mainly for input handling, but is also
            used to communicate other types of events to the application. Keep the
            event_cb struct member zero-initialized if your application doesn't require
            event handling.

        As you can see, those 'standard callbacks' don't have a user_data
        argument, so any data that needs to be preserved between callbacks
        must live in global variables. If keeping state in global variables
        is not an option, there's an alternative set of callbacks with
        an additional user_data pointer argument:

        .user_data (void*)
            The user-data argument for the callbacks below
        .init_userdata_cb (void (*)(void* user_data))
        .frame_userdata_cb (void (*)(void* user_data))
        .cleanup_userdata_cb (void (*)(void* user_data))
        .event_userdata_cb (void(*)(const sapp_event* event, void* user_data))

        The function sapp_userdata() can be used to query the user_data
        pointer provided in the sapp_desc struct.

        You can also call sapp_query_desc() to get a copy of the
        original sapp_desc structure.

        NOTE that there's also an alternative compile mode where sokol_app.h
        doesn't "hijack" the main() function. Search below for SOKOL_NO_ENTRY.

    --- Implement the initialization callback function (init_cb), this is called
        once after the rendering surface, 3D API and swap chain have been
        initialized by sokol_app. All sokol-app functions can be called
        from inside the initialization callback, the most useful functions
        at this point are:

        int sapp_width(void)
        int sapp_height(void)
            Returns the current width and height of the default framebuffer in pixels,
            this may change from one frame to the next, and it may be different
            from the initial size provided in the sapp_desc struct.

        float sapp_widthf(void)
        float sapp_heightf(void)
            These are alternatives to sapp_width() and sapp_height() which return
            the default framebuffer size as float values instead of integer. This
            may help to prevent casting back and forth between int and float
            in more strongly typed languages than C and C++.

        double sapp_frame_duration(void)
            Returns the frame duration in seconds averaged over a number of
            frames to smooth out any jittering spikes.

        int sapp_color_format(void)
        int sapp_depth_format(void)
            The color and depth-stencil pixelformats of the default framebuffer,
            as integer values which are compatible with sokol-gfx's
            sg_pixel_format enum (so that they can be plugged directly in places
            where sg_pixel_format is expected). Possible values are:

                23 == SG_PIXELFORMAT_RGBA8
                28 == SG_PIXELFORMAT_BGRA8
                42 == SG_PIXELFORMAT_DEPTH
                43 == SG_PIXELFORMAT_DEPTH_STENCIL

        int sapp_sample_count(void)
            Return the MSAA sample count of the default framebuffer.

        const void* sapp_metal_get_device(void)
        const void* sapp_metal_get_renderpass_descriptor(void)
        const void* sapp_metal_get_drawable(void)
            If the Metal backend has been selected, these functions return pointers
            to various Metal API objects required for rendering, otherwise
            they return a null pointer. These void pointers are actually
            Objective-C ids converted with a (ARC) __bridge cast so that
            the ids can be tunnel through C code. Also note that the returned
            pointers to the renderpass-descriptor and drawable may change from one
            frame to the next, only the Metal device object is guaranteed to
            stay the same.

        const void* sapp_macos_get_window(void)
            On macOS, get the NSWindow object pointer, otherwise a null pointer.
            Before being used as Objective-C object, the void* must be converted
            back with a (ARC) __bridge cast.

        const void* sapp_ios_get_window(void)
            On iOS, get the UIWindow object pointer, otherwise a null pointer.
            Before being used as Objective-C object, the void* must be converted
            back with a (ARC) __bridge cast.

        const void* sapp_win32_get_hwnd(void)
            On Windows, get the window's HWND, otherwise a null pointer. The
            HWND has been cast to a void pointer in order to be tunneled
            through code which doesn't include Windows.h.

        const void* sapp_d3d11_get_device(void)
        const void* sapp_d3d11_get_device_context(void)
        const void* sapp_d3d11_get_render_target_view(void)
        const void* sapp_d3d11_get_depth_stencil_view(void)
            Similar to the sapp_metal_* functions, the sapp_d3d11_* functions
            return pointers to D3D11 API objects required for rendering,
            only if the D3D11 backend has been selected. Otherwise they
            return a null pointer. Note that the returned pointers to the
            render-target-view and depth-stencil-view may change from one
            frame to the next!

        const void* sapp_wgpu_get_device(void)
        const void* sapp_wgpu_get_render_view(void)
        const void* sapp_wgpu_get_resolve_view(void)
        const void* sapp_wgpu_get_depth_stencil_view(void)
            These are the WebGPU-specific functions to get the WebGPU
            objects and values required for rendering. If sokol_app.h
            is not compiled with SOKOL_WGPU, these functions return null.

    --- Implement the frame-callback function, this function will be called
        on the same thread as the init callback, but might be on a different
        thread than the sokol_main() function. Note that the size of
        the rendering framebuffer might have changed since the frame callback
        was called last. Call the functions sapp_width() and sapp_height()
        each frame to get the current size.

    --- Optionally implement the event-callback to handle input events.
        sokol-app provides the following type of input events:
            - a 'virtual key' was pressed down or released
            - a single text character was entered (provided as UTF-32 code point)
            - a mouse button was pressed down or released (left, right, middle)
            - mouse-wheel or 2D scrolling events
            - the mouse was moved
            - the mouse has entered or left the application window boundaries
            - low-level, portable multi-touch events (began, moved, ended, cancelled)
            - the application window was resized, iconified or restored
            - the application was suspended or restored (on mobile platforms)
            - the user or application code has asked to quit the application
            - a string was pasted to the system clipboard
            - one or more files have been dropped onto the application window

        To explicitly 'consume' an event and prevent that the event is
        forwarded for further handling to the operating system, call
        sapp_consume_event() from inside the event handler (NOTE that
        this behaviour is currently only implemented for some HTML5
        events, support for other platforms and event types will
        be added as needed, please open a github ticket and/or provide
        a PR if needed).

        NOTE: Do *not* call any 3D API rendering functions in the event
        callback function, since the 3D API context may not be active when the
        event callback is called (it may work on some platforms and 3D APIs,
        but not others, and the exact behaviour may change between
        sokol-app versions).

    --- Implement the cleanup-callback function, this is called once
        after the user quits the application (see the section
        "APPLICATION QUIT" for detailed information on quitting
        behaviour, and how to intercept a pending quit - for instance to show a
        "Really Quit?" dialog box). Note that the cleanup-callback isn't
        guaranteed to be called on the web and mobile platforms.

    MOUSE CURSOR TYPE AND VISIBILITY
    ================================
    You can show and hide the mouse cursor with

        void sapp_show_mouse(bool show)

    And to get the current shown status:

        bool sapp_mouse_shown(void)

    NOTE that hiding the mouse cursor is different and independent from
    the MOUSE/POINTER LOCK feature which will also hide the mouse pointer when
    active (MOUSE LOCK is described below).

    To change the mouse cursor to one of several predefined types, call
    the function:

        void sapp_set_mouse_cursor(sapp_mouse_cursor cursor)

    Setting the default mouse cursor SAPP_MOUSECURSOR_DEFAULT will restore
    the standard look.

    To get the currently active mouse cursor type, call:

        sapp_mouse_cursor sapp_get_mouse_cursor(void)

    MOUSE LOCK (AKA POINTER LOCK, AKA MOUSE CAPTURE)
    ================================================
    In normal mouse mode, no mouse movement events are reported when the
    mouse leaves the windows client area or hits the screen border (whether
    it's one or the other depends on the platform), and the mouse move events
    (SAPP_EVENTTYPE_MOUSE_MOVE) contain absolute mouse positions in
    framebuffer pixels in the sapp_event items mouse_x and mouse_y, and
    relative movement in framebuffer pixels in the sapp_event items mouse_dx
    and mouse_dy.

    To get continuous mouse movement (also when the mouse leaves the window
    client area or hits the screen border), activate mouse-lock mode
    by calling:

        sapp_lock_mouse(true)

    When mouse lock is activated, the mouse pointer is hidden, the
    reported absolute mouse position (sapp_event.mouse_x/y) appears
    frozen, and the relative mouse movement in sapp_event.mouse_dx/dy
    no longer has a direct relation to framebuffer pixels but instead
    uses "raw mouse input" (what "raw mouse input" exactly means also
    differs by platform).

    To deactivate mouse lock and return to normal mouse mode, call

        sapp_lock_mouse(false)

    And finally, to check if mouse lock is currently active, call

        if (sapp_mouse_locked()) { ... }

    On native platforms, the sapp_lock_mouse() and sapp_mouse_locked()
    functions work as expected (mouse lock is activated or deactivated
    immediately when sapp_lock_mouse() is called, and sapp_mouse_locked()
    also immediately returns the new state after sapp_lock_mouse()
    is called.

    On the web platform, sapp_lock_mouse() and sapp_mouse_locked() behave
    differently, as dictated by the limitations of the HTML5 Pointer Lock API:

        - sapp_lock_mouse(true) can be called at any time, but it will
          only take effect in a 'short-lived input event handler of a specific
          type', meaning when one of the following events happens:
            - SAPP_EVENTTYPE_MOUSE_DOWN
            - SAPP_EVENTTYPE_MOUSE_UP
            - SAPP_EVENTTYPE_MOUSE_SCROLL
            - SAPP_EVENTTYPE_KEY_UP
            - SAPP_EVENTTYPE_KEY_DOWN
        - The mouse lock/unlock action on the web platform is asynchronous,
          this means that sapp_mouse_locked() won't immediately return
          the new status after calling sapp_lock_mouse(), instead the
          reported status will only change when the pointer lock has actually
          been activated or deactivated in the browser.
        - On the web, mouse lock can be deactivated by the user at any time
          by pressing the Esc key. When this happens, sokol_app.h behaves
          the same as if sapp_lock_mouse(false) is called.

    For things like camera manipulation it's most straightforward to lock
    and unlock the mouse right from the sokol_app.h event handler, for
    instance the following code enters and leaves mouse lock when the
    left mouse button is pressed and released, and then uses the relative
    movement information to manipulate a camera (taken from the
    cgltf-sapp.c sample in the sokol-samples repository
    at https://github.com/floooh/sokol-samples):

        static void input(const sapp_event* ev) {
            switch (ev->type) {
                case SAPP_EVENTTYPE_MOUSE_DOWN:
                    if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
                        sapp_lock_mouse(true);
                    }
                    break;

                case SAPP_EVENTTYPE_MOUSE_UP:
                    if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
                        sapp_lock_mouse(false);
                    }
                    break;

                case SAPP_EVENTTYPE_MOUSE_MOVE:
                    if (sapp_mouse_locked()) {
                        cam_orbit(&state.camera, ev->mouse_dx * 0.25f, ev->mouse_dy * 0.25f);
                    }
                    break;

                default:
                    break;
            }
        }

    CLIPBOARD SUPPORT
    =================
    Applications can send and receive UTF-8 encoded text data from and to the
    system clipboard. By default, clipboard support is disabled and
    must be enabled at startup via the following sapp_desc struct
    members:

        sapp_desc.enable_clipboard  - set to true to enable clipboard support
        sapp_desc.clipboard_size    - size of the internal clipboard buffer in bytes

    Enabling the clipboard will dynamically allocate a clipboard buffer
    for UTF-8 encoded text data of the requested size in bytes, the default
    size is 8 KBytes. Strings that don't fit into the clipboard buffer
    (including the terminating zero) will be silently clipped, so it's
    important that you provide a big enough clipboard size for your
    use case.

    To send data to the clipboard, call sapp_set_clipboard_string() with
    a pointer to an UTF-8 encoded, null-terminated C-string.

    NOTE that on the HTML5 platform, sapp_set_clipboard_string() must be
    called from inside a 'short-lived event handler', and there are a few
    other HTML5-specific caveats to workaround. You'll basically have to
    tinker until it works in all browsers :/ (maybe the situation will
    improve when all browsers agree on and implement the new
    HTML5 navigator.clipboard API).

    To get data from the clipboard, check for the SAPP_EVENTTYPE_CLIPBOARD_PASTED
    event in your event handler function, and then call sapp_get_clipboard_string()
    to obtain the pasted UTF-8 encoded text.

    NOTE that behaviour of sapp_get_clipboard_string() is slightly different
    depending on platform:

        - on the HTML5 platform, the internal clipboard buffer will only be updated
          right before the SAPP_EVENTTYPE_CLIPBOARD_PASTED event is sent,
          and sapp_get_clipboard_string() will simply return the current content
          of the clipboard buffer
        - on 'native' platforms, the call to sapp_get_clipboard_string() will
          update the internal clipboard buffer with the most recent data
          from the system clipboard

    Portable code should check for the SAPP_EVENTTYPE_CLIPBOARD_PASTED event,
    and then call sapp_get_clipboard_string() right in the event handler.

    The SAPP_EVENTTYPE_CLIPBOARD_PASTED event will be generated by sokol-app
    as follows:

        - on macOS: when the Cmd+V key is pressed down
        - on HTML5: when the browser sends a 'paste' event to the global 'window' object
        - on all other platforms: when the Ctrl+V key is pressed down

    DRAG AND DROP SUPPORT
    =====================
    PLEASE NOTE: the drag'n'drop feature works differently on WASM/HTML5
    and on the native desktop platforms (Win32, Linux and macOS) because
    of security-related restrictions in the HTML5 drag'n'drop API. The
    WASM/HTML5 specifics are described at the end of this documentation
    section:

    Like clipboard support, drag'n'drop support must be explicitly enabled
    at startup in the sapp_desc struct.

        sapp_desc sokol_main() {
            return (sapp_desc) {
                .enable_dragndrop = true,   // default is false
                ...
            };
        }

    You can also adjust the maximum number of files that are accepted
    in a drop operation, and the maximum path length in bytes if needed:

        sapp_desc sokol_main() {
            return (sapp_desc) {
                .enable_dragndrop = true,               // default is false
                .max_dropped_files = 8,                 // default is 1
                .max_dropped_file_path_length = 8192,   // in bytes, default is 2048
                ...
            };
        }

    When drag'n'drop is enabled, the event callback will be invoked with an
    event of type SAPP_EVENTTYPE_FILES_DROPPED whenever the user drops files on
    the application window.

    After the SAPP_EVENTTYPE_FILES_DROPPED is received, you can query the
    number of dropped files, and their absolute paths by calling separate
    functions:

        void on_event(const sapp_event* ev) {
            if (ev->type == SAPP_EVENTTYPE_FILES_DROPPED) {

                // the mouse position where the drop happened
                float x = ev->mouse_x;
                float y = ev->mouse_y;

                // get the number of files and their paths like this:
                const int num_dropped_files = sapp_get_num_dropped_files();
                for (int i = 0; i < num_dropped_files; i++) {
                    const char* path = sapp_get_dropped_file_path(i);
                    ...
                }
            }
        }

    The returned file paths are UTF-8 encoded strings.

    You can call sapp_get_num_dropped_files() and sapp_get_dropped_file_path()
    anywhere, also outside the event handler callback, but be aware that the
    file path strings will be overwritten with the next drop operation.

    In any case, sapp_get_dropped_file_path() will never return a null pointer,
    instead an empty string "" will be returned if the drag'n'drop feature
    hasn't been enabled, the last drop-operation failed, or the file path index
    is out of range.

    Drag'n'drop caveats:

        - if more files are dropped in a single drop-action
          than sapp_desc.max_dropped_files, the additional
          files will be silently ignored
        - if any of the file paths is longer than
          sapp_desc.max_dropped_file_path_length (in number of bytes, after UTF-8
          encoding) the entire drop operation will be silently ignored (this
          needs some sort of error feedback in the future)
        - no mouse positions are reported while the drag is in
          process, this may change in the future

    Drag'n'drop on HTML5/WASM:

    The HTML5 drag'n'drop API doesn't return file paths, but instead
    black-box 'file objects' which must be used to load the content
    of dropped files. This is the reason why sokol_app.h adds two
    HTML5-specific functions to the drag'n'drop API:

        uint32_t sapp_html5_get_dropped_file_size(int index)
            Returns the size in bytes of a dropped file.

        void sapp_html5_fetch_dropped_file(const sapp_html5_fetch_request* request)
            Asynchronously loads the content of a dropped file into a
            provided memory buffer (which must be big enough to hold
            the file content)

    To start loading the first dropped file after an SAPP_EVENTTYPE_FILES_DROPPED
    event is received:

        sapp_html5_fetch_dropped_file(&(sapp_html5_fetch_request){
            .dropped_file_index = 0,
            .callback = fetch_cb
            .buffer = {
                .ptr = buf,
                .size = sizeof(buf)
            },
            .user_data = ...
        });

    Make sure that the memory pointed to by 'buf' stays valid until the
    callback function is called!

    As result of the asynchronous loading operation (no matter if succeeded or
    failed) the 'fetch_cb' function will be called:

        void fetch_cb(const sapp_html5_fetch_response* response) {
            // IMPORTANT: check if the loading operation actually succeeded:
            if (response->succeeded) {
                // the size of the loaded file:
                const size_t num_bytes = response->data.size;
                // and the pointer to the data (same as 'buf' in the fetch-call):
                const void* ptr = response->data.ptr;
            }
            else {
                // on error check the error code:
                switch (response->error_code) {
                    case SAPP_HTML5_FETCH_ERROR_BUFFER_TOO_SMALL:
                        ...
                        break;
                    case SAPP_HTML5_FETCH_ERROR_OTHER:
                        ...
                        break;
                }
            }
        }

    Check the droptest-sapp example for a real-world example which works
    both on native platforms and the web:

    https://github.com/floooh/sokol-samples/blob/master/sapp/droptest-sapp.c

    HIGH-DPI RENDERING
    ==================
    You can set the sapp_desc.high_dpi flag during initialization to request
    a full-resolution framebuffer on HighDPI displays. The default behaviour
    is sapp_desc.high_dpi=false, this means that the application will
    render to a lower-resolution framebuffer on HighDPI displays and the
    rendered content will be upscaled by the window system composer.

    In a HighDPI scenario, you still request the same window size during
    sokol_main(), but the framebuffer sizes returned by sapp_width()
    and sapp_height() will be scaled up according to the DPI scaling
    ratio.

    Note that on some platforms the DPI scaling factor may change at any
    time (for instance when a window is moved from a high-dpi display
    to a low-dpi display).

    To query the current DPI scaling factor, call the function:

    float sapp_dpi_scale(void);

    For instance on a Retina Mac, returning the following sapp_desc
    struct from sokol_main():

    sapp_desc sokol_main() {
        return (sapp_desc) {
            .width = 640,
            .height = 480,
            .high_dpi = true,
            ...
        };
    }

    ...the functions the functions sapp_width(), sapp_height()
    and sapp_dpi_scale() will return the following values:

    sapp_width:     1280
    sapp_height:    960
    sapp_dpi_scale: 2.0

    If the high_dpi flag is false, or you're not running on a Retina display,
    the values would be:

    sapp_width:     640
    sapp_height:    480
    sapp_dpi_scale: 1.0

    If the window is moved from the Retina display to a low-dpi external display,
    the values would change as follows:

    sapp_width:     1280 => 640
    sapp_height:    960  => 480
    sapp_dpi_scale: 2.0  => 1.0

    Currently there is no event associated with a DPI change, but an
    SAPP_EVENTTYPE_RESIZED will be sent as a side effect of the
    framebuffer size changing.

    Per-monitor DPI is currently supported on macOS and Windows.

    APPLICATION QUIT
    ================
    Without special quit handling, a sokol_app.h application will quit
    'gracefully' when the user clicks the window close-button unless a
    platform's application model prevents this (e.g. on web or mobile).
    'Graceful exit' means that the application-provided cleanup callback will
    be called before the application quits.

    On native desktop platforms sokol_app.h provides more control over the
    application-quit-process. It's possible to initiate a 'programmatic quit'
    from the application code, and a quit initiated by the application user can
    be intercepted (for instance to show a custom dialog box).

    This 'programmatic quit protocol' is implemented through 3 functions
    and 1 event:

        - sapp_quit(): This function simply quits the application without
          giving the user a chance to intervene. Usually this might
          be called when the user clicks the 'Ok' button in a 'Really Quit?'
          dialog box
        - sapp_request_quit(): Calling sapp_request_quit() will send the
          event SAPP_EVENTTYPE_QUIT_REQUESTED to the applications event handler
          callback, giving the user code a chance to intervene and cancel the
          pending quit process (for instance to show a 'Really Quit?' dialog
          box). If the event handler callback does nothing, the application
          will be quit as usual. To prevent this, call the function
          sapp_cancel_quit() from inside the event handler.
        - sapp_cancel_quit(): Cancels a pending quit request, either initiated
          by the user clicking the window close button, or programmatically
          by calling sapp_request_quit(). The only place where calling this
          function makes sense is from inside the event handler callback when
          the SAPP_EVENTTYPE_QUIT_REQUESTED event has been received.
        - SAPP_EVENTTYPE_QUIT_REQUESTED: this event is sent when the user
          clicks the window's close button or application code calls the
          sapp_request_quit() function. The event handler callback code can handle
          this event by calling sapp_cancel_quit() to cancel the quit.
          If the event is ignored, the application will quit as usual.

    On the web platform, the quit behaviour differs from native platforms,
    because of web-specific restrictions:

    A `programmatic quit` initiated by calling sapp_quit() or
    sapp_request_quit() will work as described above: the cleanup callback is
    called, platform-specific cleanup is performed (on the web
    this means that JS event handlers are unregistered), and then
    the request-animation-loop will be exited. However that's all. The
    web page itself will continue to exist (e.g. it's not possible to
    programmatically close the browser tab).

    On the web it's also not possible to run custom code when the user
    closes a browser tab, so it's not possible to prevent this with a
    fancy custom dialog box.

    Instead the standard "Leave Site?" dialog box can be activated (or
    deactivated) with the following function:

        sapp_html5_ask_leave_site(bool ask);

    The initial state of the associated internal flag can be provided
    at startup via sapp_desc.html5_ask_leave_site.

    This feature should only be used sparingly in critical situations - for
    instance when the user would loose data - since popping up modal dialog
    boxes is considered quite rude in the web world. Note that there's no way
    to customize the content of this dialog box or run any code as a result
    of the user's decision. Also note that the user must have interacted with
    the site before the dialog box will appear. These are all security measures
    to prevent fishing.

    The Dear ImGui HighDPI sample contains example code of how to
    implement a 'Really Quit?' dialog box with Dear ImGui (native desktop
    platforms only), and for showing the hardwired "Leave Site?" dialog box
    when running on the web platform:

        https://floooh.github.io/sokol-html5/wasm/imgui-highdpi-sapp.html

    FULLSCREEN
    ==========
    If the sapp_desc.fullscreen flag is true, sokol-app will try to create
    a fullscreen window on platforms with a 'proper' window system
    (mobile devices will always use fullscreen). The implementation details
    depend on the target platform, in general sokol-app will use a
    'soft approach' which doesn't interfere too much with the platform's
    window system (for instance borderless fullscreen window instead of
    a 'real' fullscreen mode). Such details might change over time
    as sokol-app is adapted for different needs.

    The most important effect of fullscreen mode to keep in mind is that
    the requested canvas width and height will be ignored for the initial
    window size, calling sapp_width() and sapp_height() will instead return
    the resolution of the fullscreen canvas (however the provided size
    might still be used for the non-fullscreen window, in case the user can
    switch back from fullscreen- to windowed-mode).

    To toggle fullscreen mode programmatically, call sapp_toggle_fullscreen().

    To check if the application window is currently in fullscreen mode,
    call sapp_is_fullscreen().

    WINDOW ICON SUPPORT
    ===================
    Some sokol_app.h backends allow to change the window icon programmatically:

        - on Win32: the small icon in the window's title bar, and the
          bigger icon in the task bar
        - on Linux: highly dependent on the used window manager, but usually
          the window's title bar icon and/or the task bar icon
        - on HTML5: the favicon shown in the page's browser tab

    NOTE that it is not possible to set the actual application icon which is
    displayed by the operating system on the desktop or 'home screen'. Those
    icons must be provided 'traditionally' through operating-system-specific
    resources which are associated with the application (sokol_app.h might
    later support setting the window icon from platform specific resource data
    though).

    There are two ways to set the window icon:

        - at application start in the sokol_main() function by initializing
          the sapp_desc.icon nested struct
        - or later by calling the function sapp_set_icon()

    As a convenient shortcut, sokol_app.h comes with a builtin default-icon
    (a rainbow-colored 'S', which at least looks a bit better than the Windows
    default icon for applications), which can be activated like this:

    At startup in sokol_main():

        sapp_desc sokol_main(...) {
            return (sapp_desc){
                ...
                icon.sokol_default = true
            };
        }

    Or later by calling:

        sapp_set_icon(&(sapp_icon_desc){ .sokol_default = true });

    NOTE that a completely zero-initialized sapp_icon_desc struct will not
    update the window icon in any way. This is an 'escape hatch' so that you
    can handle the window icon update yourself (or if you do this already,
    sokol_app.h won't get in your way, in this case just leave the
    sapp_desc.icon struct zero-initialized).

    Providing your own icon images works exactly like in GLFW (down to the
    data format):

    You provide one or more 'candidate images' in different sizes, and the
    sokol_app.h platform backends pick the best match for the specific backend
    and icon type.

    For each candidate image, you need to provide:

        - the width in pixels
        - the height in pixels
        - and the actual pixel data in RGBA8 pixel format (e.g. 0xFFCC8844
          on a little-endian CPU means: alpha=0xFF, blue=0xCC, green=0x88, red=0x44)

    For instance, if you have 3 candidate images (small, medium, big) of
    sizes 16x16, 32x32 and 64x64 the corresponding sapp_icon_desc struct is setup
    like this:

        // the actual pixel data (RGBA8, origin top-left)
        const uint32_t small[16][16]  = { ... };
        const uint32_t medium[32][32] = { ... };
        const uint32_t big[64][64]    = { ... };

        const sapp_icon_desc icon_desc = {
            .images = {
                { .width = 16, .height = 16, .pixels = SAPP_RANGE(small) },
                { .width = 32, .height = 32, .pixels = SAPP_RANGE(medium) },
                // ...or without the SAPP_RANGE helper macro:
                { .width = 64, .height = 64, .pixels = { .ptr=big, .size=sizeof(big) } }
            }
        };

    An sapp_icon_desc struct initialized like this can then either be applied
    at application start in sokol_main:

        sapp_desc sokol_main(...) {
            return (sapp_desc){
                ...
                icon = icon_desc
            };
        }

    ...or later by calling sapp_set_icon():

        sapp_set_icon(&icon_desc);

    Some window icon caveats:

        - once the window icon has been updated, there's no way to go back to
          the platform's default icon, this is because some platforms (Linux
          and HTML5) don't switch the icon visual back to the default even if
          the custom icon is deleted or removed
        - on HTML5, if the sokol_app.h icon doesn't show up in the browser
          tab, check that there's no traditional favicon 'link' element
          is defined in the page's index.html, sokol_app.h will only
          append a new favicon link element, but not delete any manually
          defined favicon in the page

    For an example and test of the window icon feature, check out the
    'icon-sapp' sample on the sokol-samples git repository.

    ONSCREEN KEYBOARD
    =================
    On some platforms which don't provide a physical keyboard, sokol-app
    can display the platform's integrated onscreen keyboard for text
    input. To request that the onscreen keyboard is shown, call

        sapp_show_keyboard(true);

    Likewise, to hide the keyboard call:

        sapp_show_keyboard(false);

    Note that on the web platform, the keyboard can only be shown from
    inside an input handler. On such platforms, sapp_show_keyboard()
    will only work as expected when it is called from inside the
    sokol-app event callback function. When called from other places,
    an internal flag will be set, and the onscreen keyboard will be
    called at the next 'legal' opportunity (when the next input event
    is handled).

    OPTIONAL: DON'T HIJACK main() (#define SOKOL_NO_ENTRY)
    ======================================================
    NOTE: SOKOL_NO_ENTRY and sapp_run() is currently not supported on Android.

    In its default configuration, sokol_app.h "hijacks" the platform's
    standard main() function. This was done because different platforms
    have different entry point conventions which are not compatible with
    C's main() (for instance WinMain on Windows has completely different
    arguments). However, this "main hijacking" posed a problem for
    usage scenarios like integrating sokol_app.h with other languages than
    C or C++, so an alternative SOKOL_NO_ENTRY mode has been added
    in which the user code provides the platform's main function:

    - define SOKOL_NO_ENTRY before including the sokol_app.h implementation
    - do *not* provide a sokol_main() function
    - instead provide the standard main() function of the platform
    - from the main function, call the function ```sapp_run()``` which
      takes a pointer to an ```sapp_desc``` structure.
    - from here on```sapp_run()``` takes over control and calls the provided
      init-, frame-, event- and cleanup-callbacks just like in the default model.

    sapp_run() behaves differently across platforms:

        - on some platforms, sapp_run() will return when the application quits
        - on other platforms, sapp_run() will never return, even when the
          application quits (the operating system is free to simply terminate
          the application at any time)
        - on Emscripten specifically, sapp_run() will return immediately while
          the frame callback keeps being called

    This different behaviour of sapp_run() essentially means that there shouldn't
    be any code *after* sapp_run(), because that may either never be called, or in
    case of Emscripten will be called at an unexpected time (at application start).

    An application also should not depend on the cleanup-callback being called
    when cross-platform compatibility is required.

    Since sapp_run() returns immediately on Emscripten you shouldn't activate
    the 'EXIT_RUNTIME' linker option (this is disabled by default when compiling
    for the browser target), since the C/C++ exit runtime would be called immediately at
    application start, causing any global objects to be destroyed and global
    variables to be zeroed.

    WINDOWS CONSOLE OUTPUT
    ======================
    On Windows, regular windowed applications don't show any stdout/stderr text
    output, which can be a bit of a hassle for printf() debugging or generally
    logging text to the console. Also, console output by default uses a local
    codepage setting and thus international UTF-8 encoded text is printed
    as garbage.

    To help with these issues, sokol_app.h can be configured at startup
    via the following Windows-specific sapp_desc flags:

        sapp_desc.win32_console_utf8 (default: false)
            When set to true, the output console codepage will be switched
            to UTF-8 (and restored to the original codepage on exit)

        sapp_desc.win32_console_attach (default: false)
            When set to true, stdout and stderr will be attached to the
            console of the parent process (if the parent process actually
            has a console). This means that if the application was started
            in a command line window, stdout and stderr output will be printed
            to the terminal, just like a regular command line program. But if
            the application is started via double-click, it will behave like
            a regular UI application, and stdout/stderr will not be visible.

        sapp_desc.win32_console_create (default: false)
            When set to true, a new console window will be created and
            stdout/stderr will be redirected to that console window. It
            doesn't matter if the application is started from the command
            line or via double-click.

    MEMORY ALLOCATION OVERRIDE
    ==========================
    You can override the memory allocation functions at initialization time
    like this:

        void* my_alloc(size_t size, void* user_data) {
            return malloc(size);
        }

        void my_free(void* ptr, void* user_data) {
            free(ptr);
        }

        sapp_desc sokol_main(int argc, char* argv[]) {
            return (sapp_desc){
                // ...
                .allocator = {
                    .alloc_fn = my_alloc,
                    .free_fn = my_free,
                    .user_data = ...,
                }
            };
        }

    If no overrides are provided, malloc and free will be used.

    This only affects memory allocation calls done by sokol_app.h
    itself though, not any allocations in OS libraries.


    ERROR REPORTING AND LOGGING
    ===========================
    To get any logging information at all you need to provide a logging callback in the setup call
    the easiest way is to use sokol_log.h:

        #include "sokol_log.h"

        sapp_desc sokol_main(int argc, char* argv[]) {
            return (sapp_desc) {
                ...
                .logger.func = slog_func,
            };
        }

    To override logging with your own callback, first write a logging function like this:

        void my_log(const char* tag,                // e.g. 'sapp'
                    uint32_t log_level,             // 0=panic, 1=error, 2=warn, 3=info
                    uint32_t log_item_id,           // SAPP_LOGITEM_*
                    const char* message_or_null,    // a message string, may be nullptr in release mode
                    uint32_t line_nr,               // line number in sokol_app.h
                    const char* filename_or_null,   // source filename, may be nullptr in release mode
                    void* user_data)
        {
            ...
        }

    ...and then setup sokol-app like this:

        sapp_desc sokol_main(int argc, char* argv[]) {
            return (sapp_desc) {
                ...
                .logger = {
                    .func = my_log,
                    .user_data = my_user_data,
                }
            };
        }

    The provided logging function must be reentrant (e.g. be callable from
    different threads).

    If you don't want to provide your own custom logger it is highly recommended to use
    the standard logger in sokol_log.h instead, otherwise you won't see any warnings or
    errors.


    TEMP NOTE DUMP
    ==============
    - onscreen keyboard support on Android requires Java :(, should we even bother?
    - sapp_desc needs a bool whether to initialize depth-stencil surface
    - GL context initialization needs more control (at least what GL version to initialize)
    - application icon
    - the Android implementation calls cleanup_cb() and destroys the egl context in onDestroy
      at the latest but should do it earlier, in onStop, as an app is "killable" after onStop
      on Android Honeycomb and later (it can't be done at the moment as the app may be started
      again after onStop and the sokol lifecycle does not yet handle context teardown/bringup)


    LICENSE
    =======
    zlib/libpng license

    Copyright (c) 2018 Andre Weissflog

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.

        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.

        3. This notice may not be removed or altered from any source
        distribution.
*/
#define SOKOL_APP_INCLUDED (1)
#include <stddef.h> // size_t
#include <stdint.h>
#include <stdbool.h>

#if defined(SOKOL_API_DECL) && !defined(SOKOL_APP_API_DECL)
#define SOKOL_APP_API_DECL SOKOL_API_DECL
#endif
#ifndef SOKOL_APP_API_DECL
#if defined(_WIN32) && defined(SOKOL_DLL) && defined(SOKOL_APP_IMPL)
#define SOKOL_APP_API_DECL __declspec(dllexport)
#elif defined(_WIN32) && defined(SOKOL_DLL)
#define SOKOL_APP_API_DECL __declspec(dllimport)
#else
#define SOKOL_APP_API_DECL extern
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* misc constants */
enum {
    SAPP_MAX_TOUCHPOINTS = 8,
    SAPP_MAX_MOUSEBUTTONS = 3,
    SAPP_MAX_KEYCODES = 512,
    SAPP_MAX_ICONIMAGES = 8,
};

/*
    sapp_event_type

    The type of event that's passed to the event handler callback
    in the sapp_event.type field. These are not just "traditional"
    input events, but also notify the application about state changes
    or other user-invoked actions.
*/
typedef enum sapp_event_type {
    SAPP_EVENTTYPE_INVALID,
    SAPP_EVENTTYPE_KEY_DOWN,
    SAPP_EVENTTYPE_KEY_UP,
    SAPP_EVENTTYPE_CHAR,
    SAPP_EVENTTYPE_MOUSE_DOWN,
    SAPP_EVENTTYPE_MOUSE_UP,
    SAPP_EVENTTYPE_MOUSE_SCROLL,
    SAPP_EVENTTYPE_MOUSE_MOVE,
    SAPP_EVENTTYPE_MOUSE_ENTER,
    SAPP_EVENTTYPE_MOUSE_LEAVE,
    SAPP_EVENTTYPE_TOUCHES_BEGAN,
    SAPP_EVENTTYPE_TOUCHES_MOVED,
    SAPP_EVENTTYPE_TOUCHES_ENDED,
    SAPP_EVENTTYPE_TOUCHES_CANCELLED,
    SAPP_EVENTTYPE_RESIZED,
    SAPP_EVENTTYPE_ICONIFIED,
    SAPP_EVENTTYPE_RESTORED,
    SAPP_EVENTTYPE_FOCUSED,
    SAPP_EVENTTYPE_UNFOCUSED,
    SAPP_EVENTTYPE_SUSPENDED,
    SAPP_EVENTTYPE_RESUMED,
    SAPP_EVENTTYPE_QUIT_REQUESTED,
    SAPP_EVENTTYPE_CLIPBOARD_PASTED,
    SAPP_EVENTTYPE_FILES_DROPPED,
    _SAPP_EVENTTYPE_NUM,
    _SAPP_EVENTTYPE_FORCE_U32 = 0x7FFFFFFF
} sapp_event_type;

/*
    sapp_keycode

    The 'virtual keycode' of a KEY_DOWN or KEY_UP event in the
    struct field sapp_event.key_code.

    Note that the keycode values are identical with GLFW.
*/
typedef enum sapp_keycode {
    SAPP_KEYCODE_INVALID          = 0,
    SAPP_KEYCODE_SPACE            = 32,
    SAPP_KEYCODE_APOSTROPHE       = 39,  /* ' */
    SAPP_KEYCODE_COMMA            = 44,  /* , */
    SAPP_KEYCODE_MINUS            = 45,  /* - */
    SAPP_KEYCODE_PERIOD           = 46,  /* . */
    SAPP_KEYCODE_SLASH            = 47,  /* / */
    SAPP_KEYCODE_0                = 48,
    SAPP_KEYCODE_1                = 49,
    SAPP_KEYCODE_2                = 50,
    SAPP_KEYCODE_3                = 51,
    SAPP_KEYCODE_4                = 52,
    SAPP_KEYCODE_5                = 53,
    SAPP_KEYCODE_6                = 54,
    SAPP_KEYCODE_7                = 55,
    SAPP_KEYCODE_8                = 56,
    SAPP_KEYCODE_9                = 57,
    SAPP_KEYCODE_SEMICOLON        = 59,  /* ; */
    SAPP_KEYCODE_EQUAL            = 61,  /* = */
    SAPP_KEYCODE_A                = 65,
    SAPP_KEYCODE_B                = 66,
    SAPP_KEYCODE_C                = 67,
    SAPP_KEYCODE_D                = 68,
    SAPP_KEYCODE_E                = 69,
    SAPP_KEYCODE_F                = 70,
    SAPP_KEYCODE_G                = 71,
    SAPP_KEYCODE_H                = 72,
    SAPP_KEYCODE_I                = 73,
    SAPP_KEYCODE_J                = 74,
    SAPP_KEYCODE_K                = 75,
    SAPP_KEYCODE_L                = 76,
    SAPP_KEYCODE_M                = 77,
    SAPP_KEYCODE_N                = 78,
    SAPP_KEYCODE_O                = 79,
    SAPP_KEYCODE_P                = 80,
    SAPP_KEYCODE_Q                = 81,
    SAPP_KEYCODE_R                = 82,
    SAPP_KEYCODE_S                = 83,
    SAPP_KEYCODE_T                = 84,
    SAPP_KEYCODE_U                = 85,
    SAPP_KEYCODE_V                = 86,
    SAPP_KEYCODE_W                = 87,
    SAPP_KEYCODE_X                = 88,
    SAPP_KEYCODE_Y                = 89,
    SAPP_KEYCODE_Z                = 90,
    SAPP_KEYCODE_LEFT_BRACKET     = 91,  /* [ */
    SAPP_KEYCODE_BACKSLASH        = 92,  /* \ */
    SAPP_KEYCODE_RIGHT_BRACKET    = 93,  /* ] */
    SAPP_KEYCODE_GRAVE_ACCENT     = 96,  /* ` */
    SAPP_KEYCODE_WORLD_1          = 161, /* non-US #1 */
    SAPP_KEYCODE_WORLD_2          = 162, /* non-US #2 */
    SAPP_KEYCODE_ESCAPE           = 256,
    SAPP_KEYCODE_ENTER            = 257,
    SAPP_KEYCODE_TAB              = 258,
    SAPP_KEYCODE_BACKSPACE        = 259,
    SAPP_KEYCODE_INSERT           = 260,
    SAPP_KEYCODE_DELETE           = 261,
    SAPP_KEYCODE_RIGHT            = 262,
    SAPP_KEYCODE_LEFT             = 263,
    SAPP_KEYCODE_DOWN             = 264,
    SAPP_KEYCODE_UP               = 265,
    SAPP_KEYCODE_PAGE_UP          = 266,
    SAPP_KEYCODE_PAGE_DOWN        = 267,
    SAPP_KEYCODE_HOME             = 268,
    SAPP_KEYCODE_END              = 269,
    SAPP_KEYCODE_CAPS_LOCK        = 280,
    SAPP_KEYCODE_SCROLL_LOCK      = 281,
    SAPP_KEYCODE_NUM_LOCK         = 282,
    SAPP_KEYCODE_PRINT_SCREEN     = 283,
    SAPP_KEYCODE_PAUSE            = 284,
    SAPP_KEYCODE_F1               = 290,
    SAPP_KEYCODE_F2               = 291,
    SAPP_KEYCODE_F3               = 292,
    SAPP_KEYCODE_F4               = 293,
    SAPP_KEYCODE_F5               = 294,
    SAPP_KEYCODE_F6               = 295,
    SAPP_KEYCODE_F7               = 296,
    SAPP_KEYCODE_F8               = 297,
    SAPP_KEYCODE_F9               = 298,
    SAPP_KEYCODE_F10              = 299,
    SAPP_KEYCODE_F11              = 300,
    SAPP_KEYCODE_F12              = 301,
    SAPP_KEYCODE_F13              = 302,
    SAPP_KEYCODE_F14              = 303,
    SAPP_KEYCODE_F15              = 304,
    SAPP_KEYCODE_F16              = 305,
    SAPP_KEYCODE_F17              = 306,
    SAPP_KEYCODE_F18              = 307,
    SAPP_KEYCODE_F19              = 308,
    SAPP_KEYCODE_F20              = 309,
    SAPP_KEYCODE_F21              = 310,
    SAPP_KEYCODE_F22              = 311,
    SAPP_KEYCODE_F23              = 312,
    SAPP_KEYCODE_F24              = 313,
    SAPP_KEYCODE_F25              = 314,
    SAPP_KEYCODE_KP_0             = 320,
    SAPP_KEYCODE_KP_1             = 321,
    SAPP_KEYCODE_KP_2             = 322,
    SAPP_KEYCODE_KP_3             = 323,
    SAPP_KEYCODE_KP_4             = 324,
    SAPP_KEYCODE_KP_5             = 325,
    SAPP_KEYCODE_KP_6             = 326,
    SAPP_KEYCODE_KP_7             = 327,
    SAPP_KEYCODE_KP_8             = 328,
    SAPP_KEYCODE_KP_9             = 329,
    SAPP_KEYCODE_KP_DECIMAL       = 330,
    SAPP_KEYCODE_KP_DIVIDE        = 331,
    SAPP_KEYCODE_KP_MULTIPLY      = 332,
    SAPP_KEYCODE_KP_SUBTRACT      = 333,
    SAPP_KEYCODE_KP_ADD           = 334,
    SAPP_KEYCODE_KP_ENTER         = 335,
    SAPP_KEYCODE_KP_EQUAL         = 336,
    SAPP_KEYCODE_LEFT_SHIFT       = 340,
    SAPP_KEYCODE_LEFT_CONTROL     = 341,
    SAPP_KEYCODE_LEFT_ALT         = 342,
    SAPP_KEYCODE_LEFT_SUPER       = 343,
    SAPP_KEYCODE_RIGHT_SHIFT      = 344,
    SAPP_KEYCODE_RIGHT_CONTROL    = 345,
    SAPP_KEYCODE_RIGHT_ALT        = 346,
    SAPP_KEYCODE_RIGHT_SUPER      = 347,
    SAPP_KEYCODE_MENU             = 348,
} sapp_keycode;

/*
    Android specific 'tool type' enum for touch events. This lets the
    application check what type of input device was used for
    touch events.

    NOTE: the values must remain in sync with the corresponding
    Android SDK type, so don't change those.

    See https://developer.android.com/reference/android/view/MotionEvent#TOOL_TYPE_UNKNOWN
*/
typedef enum sapp_android_tooltype {
    SAPP_ANDROIDTOOLTYPE_UNKNOWN = 0,   // TOOL_TYPE_UNKNOWN
    SAPP_ANDROIDTOOLTYPE_FINGER = 1,    // TOOL_TYPE_FINGER
    SAPP_ANDROIDTOOLTYPE_STYLUS = 2,    // TOOL_TYPE_STYLUS
    SAPP_ANDROIDTOOLTYPE_MOUSE = 3,     // TOOL_TYPE_MOUSE
} sapp_android_tooltype;

/*
    sapp_touchpoint

    Describes a single touchpoint in a multitouch event (TOUCHES_BEGAN,
    TOUCHES_MOVED, TOUCHES_ENDED).

    Touch points are stored in the nested array sapp_event.touches[],
    and the number of touches is stored in sapp_event.num_touches.
*/
typedef struct sapp_touchpoint {
    uintptr_t identifier;
    float pos_x;
    float pos_y;
    sapp_android_tooltype android_tooltype; // only valid on Android
    bool changed;
} sapp_touchpoint;

/*
    sapp_mousebutton

    The currently pressed mouse button in the events MOUSE_DOWN
    and MOUSE_UP, stored in the struct field sapp_event.mouse_button.
*/
typedef enum sapp_mousebutton {
    SAPP_MOUSEBUTTON_LEFT = 0x0,
    SAPP_MOUSEBUTTON_RIGHT = 0x1,
    SAPP_MOUSEBUTTON_MIDDLE = 0x2,
    SAPP_MOUSEBUTTON_INVALID = 0x100,
} sapp_mousebutton;

/*
    These are currently pressed modifier keys (and mouse buttons) which are
    passed in the event struct field sapp_event.modifiers.
*/
enum {
    SAPP_MODIFIER_SHIFT = 0x1,      // left or right shift key
    SAPP_MODIFIER_CTRL  = 0x2,      // left or right control key
    SAPP_MODIFIER_ALT   = 0x4,      // left or right alt key
    SAPP_MODIFIER_SUPER = 0x8,      // left or right 'super' key
    SAPP_MODIFIER_LMB   = 0x100,    // left mouse button
    SAPP_MODIFIER_RMB   = 0x200,    // right mouse button
    SAPP_MODIFIER_MMB   = 0x400,    // middle mouse button
};

/*
    sapp_event

    This is an all-in-one event struct passed to the event handler
    user callback function. Note that it depends on the event
    type what struct fields actually contain useful values, so you
    should first check the event type before reading other struct
    fields.
*/
typedef struct sapp_event {
    uint64_t frame_count;               // current frame counter, always valid, useful for checking if two events were issued in the same frame
    sapp_event_type type;               // the event type, always valid
    sapp_keycode key_code;              // the virtual key code, only valid in KEY_UP, KEY_DOWN
    uint32_t char_code;                 // the UTF-32 character code, only valid in CHAR events
    bool key_repeat;                    // true if this is a key-repeat event, valid in KEY_UP, KEY_DOWN and CHAR
    uint32_t modifiers;                 // current modifier keys, valid in all key-, char- and mouse-events
    sapp_mousebutton mouse_button;      // mouse button that was pressed or released, valid in MOUSE_DOWN, MOUSE_UP
    float mouse_x;                      // current horizontal mouse position in pixels, always valid except during mouse lock
    float mouse_y;                      // current vertical mouse position in pixels, always valid except during mouse lock
    float mouse_dx;                     // relative horizontal mouse movement since last frame, always valid
    float mouse_dy;                     // relative vertical mouse movement since last frame, always valid
    float scroll_x;                     // horizontal mouse wheel scroll distance, valid in MOUSE_SCROLL events
    float scroll_y;                     // vertical mouse wheel scroll distance, valid in MOUSE_SCROLL events
    int num_touches;                    // number of valid items in the touches[] array
    sapp_touchpoint touches[SAPP_MAX_TOUCHPOINTS];  // current touch points, valid in TOUCHES_BEGIN, TOUCHES_MOVED, TOUCHES_ENDED
    int window_width;                   // current window- and framebuffer sizes in pixels, always valid
    int window_height;
    int framebuffer_width;              // = window_width * dpi_scale
    int framebuffer_height;             // = window_height * dpi_scale
} sapp_event;

/*
    sg_range

    A general pointer/size-pair struct and constructor macros for passing binary blobs
    into sokol_app.h.
*/
typedef struct sapp_range {
    const void* ptr;
    size_t size;
} sapp_range;
// disabling this for every includer isn't great, but the warnings are also quite pointless
#if defined(_MSC_VER)
#pragma warning(disable:4221)   /* /W4 only: nonstandard extension used: 'x': cannot be initialized using address of automatic variable 'y' */
#pragma warning(disable:4204)   /* VS2015: nonstandard extension used: non-constant aggregate initializer */
#endif
#if defined(__cplusplus)
#define SAPP_RANGE(x) sapp_range{ &x, sizeof(x) }
#else
#define SAPP_RANGE(x) (sapp_range){ &x, sizeof(x) }
#endif

/*
    sapp_image_desc

    This is used to describe image data to sokol_app.h (at first, window
    icons, later maybe cursor images).

    Note that the actual image pixel format depends on the use case:

    - window icon pixels are RGBA8
    - cursor images are ??? (FIXME)
*/
typedef struct sapp_image_desc {
    int width;
    int height;
    sapp_range pixels;
} sapp_image_desc;

/*
    sapp_icon_desc

    An icon description structure for use in sapp_desc.icon and
    sapp_set_icon().

    When setting a custom image, the application can provide a number of
    candidates differing in size, and sokol_app.h will pick the image(s)
    closest to the size expected by the platform's window system.

    To set sokol-app's default icon, set .sokol_default to true.

    Otherwise provide candidate images of different sizes in the
    images[] array.

    If both the sokol_default flag is set to true, any image candidates
    will be ignored and the sokol_app.h default icon will be set.
*/
typedef struct sapp_icon_desc {
    bool sokol_default;
    sapp_image_desc images[SAPP_MAX_ICONIMAGES];
} sapp_icon_desc;

/*
    sapp_allocator

    Used in sapp_desc to provide custom memory-alloc and -free functions
    to sokol_app.h. If memory management should be overridden, both the
    alloc_fn and free_fn function must be provided (e.g. it's not valid to
    override one function but not the other).
*/
typedef struct sapp_allocator {
    void* (*alloc_fn)(size_t size, void* user_data);
    void (*free_fn)(void* ptr, void* user_data);
    void* user_data;
} sapp_allocator;

/*
    sapp_log_item

    Log items are defined via X-Macros and expanded to an enum
    'sapp_log_item', and in debug mode to corresponding
    human readable error messages.
*/
#define _SAPP_LOG_ITEMS \
    _SAPP_LOGITEM_XMACRO(OK, "Ok") \
    _SAPP_LOGITEM_XMACRO(MALLOC_FAILED, "memory allocation failed") \
    _SAPP_LOGITEM_XMACRO(MACOS_INVALID_NSOPENGL_PROFILE, "macos: invalid NSOpenGLProfile (valid choices are 1.0, 3.2 and 4.1)") \
    _SAPP_LOGITEM_XMACRO(WIN32_LOAD_OPENGL32_DLL_FAILED, "failed loading opengl32.dll") \
    _SAPP_LOGITEM_XMACRO(WIN32_CREATE_HELPER_WINDOW_FAILED, "failed to create helper window") \
    _SAPP_LOGITEM_XMACRO(WIN32_HELPER_WINDOW_GETDC_FAILED, "failed to get helper window DC") \
    _SAPP_LOGITEM_XMACRO(WIN32_DUMMY_CONTEXT_SET_PIXELFORMAT_FAILED, "failed to set pixel format for dummy GL context") \
    _SAPP_LOGITEM_XMACRO(WIN32_CREATE_DUMMY_CONTEXT_FAILED, "failed to create dummy GL context") \
    _SAPP_LOGITEM_XMACRO(WIN32_DUMMY_CONTEXT_MAKE_CURRENT_FAILED, "failed to make dummy GL context current") \
    _SAPP_LOGITEM_XMACRO(WIN32_GET_PIXELFORMAT_ATTRIB_FAILED, "failed to get WGL pixel format attribute") \
    _SAPP_LOGITEM_XMACRO(WIN32_WGL_FIND_PIXELFORMAT_FAILED, "failed to find matching WGL pixel format") \
    _SAPP_LOGITEM_XMACRO(WIN32_WGL_DESCRIBE_PIXELFORMAT_FAILED, "failed to get pixel format descriptor") \
    _SAPP_LOGITEM_XMACRO(WIN32_WGL_SET_PIXELFORMAT_FAILED, "failed to set selected pixel format") \
    _SAPP_LOGITEM_XMACRO(WIN32_WGL_ARB_CREATE_CONTEXT_REQUIRED, "ARB_create_context required") \
    _SAPP_LOGITEM_XMACRO(WIN32_WGL_ARB_CREATE_CONTEXT_PROFILE_REQUIRED, "ARB_create_context_profile required") \
    _SAPP_LOGITEM_XMACRO(WIN32_WGL_OPENGL_3_2_NOT_SUPPORTED, "OpenGL 3.2 not supported by GL driver (ERROR_INVALID_VERSION_ARB)") \
    _SAPP_LOGITEM_XMACRO(WIN32_WGL_OPENGL_PROFILE_NOT_SUPPORTED, "requested OpenGL profile not support by GL driver (ERROR_INVALID_PROFILE_ARB)") \
    _SAPP_LOGITEM_XMACRO(WIN32_WGL_INCOMPATIBLE_DEVICE_CONTEXT, "CreateContextAttribsARB failed with ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB") \
    _SAPP_LOGITEM_XMACRO(WIN32_WGL_CREATE_CONTEXT_ATTRIBS_FAILED_OTHER, "CreateContextAttribsARB failed for other reason") \
    _SAPP_LOGITEM_XMACRO(WIN32_D3D11_CREATE_DEVICE_AND_SWAPCHAIN_WITH_DEBUG_FAILED, "D3D11CreateDeviceAndSwapChain() with D3D11_CREATE_DEVICE_DEBUG failed, retrying without debug flag.") \
    _SAPP_LOGITEM_XMACRO(WIN32_D3D11_GET_IDXGIFACTORY_FAILED, "could not obtain IDXGIFactory object") \
    _SAPP_LOGITEM_XMACRO(WIN32_D3D11_GET_IDXGIADAPTER_FAILED, "could not obtain IDXGIAdapter object") \
    _SAPP_LOGITEM_XMACRO(WIN32_D3D11_QUERY_INTERFACE_IDXGIDEVICE1_FAILED, "could not obtain IDXGIDevice1 interface") \
    _SAPP_LOGITEM_XMACRO(WIN32_REGISTER_RAW_INPUT_DEVICES_FAILED_MOUSE_LOCK, "RegisterRawInputDevices() failed (on mouse lock)") \
    _SAPP_LOGITEM_XMACRO(WIN32_REGISTER_RAW_INPUT_DEVICES_FAILED_MOUSE_UNLOCK, "RegisterRawInputDevices() failed (on mouse unlock)") \
    _SAPP_LOGITEM_XMACRO(WIN32_GET_RAW_INPUT_DATA_FAILED, "GetRawInputData() failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_GLX_LOAD_LIBGL_FAILED, "failed to load libGL") \
    _SAPP_LOGITEM_XMACRO(LINUX_GLX_LOAD_ENTRY_POINTS_FAILED, "failed to load GLX entry points") \
    _SAPP_LOGITEM_XMACRO(LINUX_GLX_EXTENSION_NOT_FOUND, "GLX extension not found") \
    _SAPP_LOGITEM_XMACRO(LINUX_GLX_QUERY_VERSION_FAILED, "failed to query GLX version") \
    _SAPP_LOGITEM_XMACRO(LINUX_GLX_VERSION_TOO_LOW, "GLX version too low (need at least 1.3)") \
    _SAPP_LOGITEM_XMACRO(LINUX_GLX_NO_GLXFBCONFIGS, "glXGetFBConfigs() returned no configs") \
    _SAPP_LOGITEM_XMACRO(LINUX_GLX_NO_SUITABLE_GLXFBCONFIG, "failed to find a suitable GLXFBConfig") \
    _SAPP_LOGITEM_XMACRO(LINUX_GLX_GET_VISUAL_FROM_FBCONFIG_FAILED, "glXGetVisualFromFBConfig failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_GLX_REQUIRED_EXTENSIONS_MISSING, "GLX extensions ARB_create_context and ARB_create_context_profile missing") \
    _SAPP_LOGITEM_XMACRO(LINUX_GLX_CREATE_CONTEXT_FAILED, "Failed to create GL context via glXCreateContextAttribsARB") \
    _SAPP_LOGITEM_XMACRO(LINUX_GLX_CREATE_WINDOW_FAILED, "glXCreateWindow() failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_X11_CREATE_WINDOW_FAILED, "XCreateWindow() failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_EGL_BIND_OPENGL_API_FAILED, "eglBindAPI(EGL_OPENGL_API) failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_EGL_BIND_OPENGL_ES_API_FAILED, "eglBindAPI(EGL_OPENGL_ES_API) failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_EGL_GET_DISPLAY_FAILED, "eglGetDisplay() failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_EGL_INITIALIZE_FAILED, "eglInitialize() failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_EGL_NO_CONFIGS, "eglChooseConfig() returned no configs") \
    _SAPP_LOGITEM_XMACRO(LINUX_EGL_NO_NATIVE_VISUAL, "eglGetConfigAttrib() for EGL_NATIVE_VISUAL_ID failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_EGL_GET_VISUAL_INFO_FAILED, "XGetVisualInfo() failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_EGL_CREATE_WINDOW_SURFACE_FAILED, "eglCreateWindowSurface() failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_EGL_CREATE_CONTEXT_FAILED, "eglCreateContext() failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_EGL_MAKE_CURRENT_FAILED, "eglMakeCurrent() failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_X11_OPEN_DISPLAY_FAILED, "XOpenDisplay() failed") \
    _SAPP_LOGITEM_XMACRO(LINUX_X11_QUERY_SYSTEM_DPI_FAILED, "failed to query system dpi value, assuming default 96.0") \
    _SAPP_LOGITEM_XMACRO(LINUX_X11_DROPPED_FILE_URI_WRONG_SCHEME, "dropped file URL doesn't start with 'file://'") \
    _SAPP_LOGITEM_XMACRO(ANDROID_UNSUPPORTED_INPUT_EVENT_INPUT_CB, "unsupported input event encountered in _sapp_android_input_cb()") \
    _SAPP_LOGITEM_XMACRO(ANDROID_UNSUPPORTED_INPUT_EVENT_MAIN_CB, "unsupported input event encountered in _sapp_android_main_cb()") \
    _SAPP_LOGITEM_XMACRO(ANDROID_READ_MSG_FAILED, "failed to read message in _sapp_android_main_cb()") \
    _SAPP_LOGITEM_XMACRO(ANDROID_WRITE_MSG_FAILED, "failed to write message in _sapp_android_msg") \
    _SAPP_LOGITEM_XMACRO(ANDROID_MSG_CREATE, "MSG_CREATE") \
    _SAPP_LOGITEM_XMACRO(ANDROID_MSG_RESUME, "MSG_RESUME") \
    _SAPP_LOGITEM_XMACRO(ANDROID_MSG_PAUSE, "MSG_PAUSE") \
    _SAPP_LOGITEM_XMACRO(ANDROID_MSG_FOCUS, "MSG_FOCUS") \
    _SAPP_LOGITEM_XMACRO(ANDROID_MSG_NO_FOCUS, "MSG_NO_FOCUS") \
    _SAPP_LOGITEM_XMACRO(ANDROID_MSG_SET_NATIVE_WINDOW, "MSG_SET_NATIVE_WINDOW") \
    _SAPP_LOGITEM_XMACRO(ANDROID_MSG_SET_INPUT_QUEUE, "MSG_SET_INPUT_QUEUE") \
    _SAPP_LOGITEM_XMACRO(ANDROID_MSG_DESTROY, "MSG_DESTROY") \
    _SAPP_LOGITEM_XMACRO(ANDROID_UNKNOWN_MSG, "unknown msg type received") \
    _SAPP_LOGITEM_XMACRO(ANDROID_LOOP_THREAD_STARTED, "loop thread started") \
    _SAPP_LOGITEM_XMACRO(ANDROID_LOOP_THREAD_DONE, "loop thread done") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONSTART, "NativeActivity onStart()") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONRESUME, "NativeActivity onResume") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONSAVEINSTANCESTATE, "NativeActivity onSaveInstanceState") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONWINDOWFOCUSCHANGED, "NativeActivity onWindowFocusChanged") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONPAUSE, "NativeActivity onPause") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONSTOP, "NativeActivity onStop()") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONNATIVEWINDOWCREATED, "NativeActivity onNativeWindowCreated") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONNATIVEWINDOWDESTROYED, "NativeActivity onNativeWindowDestroyed") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONINPUTQUEUECREATED, "NativeActivity onInputQueueCreated") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONINPUTQUEUEDESTROYED, "NativeActivity onInputQueueDestroyed") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONCONFIGURATIONCHANGED, "NativeActivity onConfigurationChanged") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONLOWMEMORY, "NativeActivity onLowMemory") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONDESTROY, "NativeActivity onDestroy") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_DONE, "NativeActivity done") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_ONCREATE, "NativeActivity onCreate") \
    _SAPP_LOGITEM_XMACRO(ANDROID_CREATE_THREAD_PIPE_FAILED, "failed to create thread pipe") \
    _SAPP_LOGITEM_XMACRO(ANDROID_NATIVE_ACTIVITY_CREATE_SUCCESS, "NativeActivity sucessfully created") \
    _SAPP_LOGITEM_XMACRO(WGPU_SWAPCHAIN_CREATE_SURFACE_FAILED, "wgpu: failed to create surface for swapchain") \
    _SAPP_LOGITEM_XMACRO(WGPU_SWAPCHAIN_CREATE_SWAPCHAIN_FAILED, "wgpu: failed to create swapchain object") \
    _SAPP_LOGITEM_XMACRO(WGPU_SWAPCHAIN_CREATE_DEPTH_STENCIL_TEXTURE_FAILED, "wgpu: failed to create depth-stencil texture for swapchain") \
    _SAPP_LOGITEM_XMACRO(WGPU_SWAPCHAIN_CREATE_DEPTH_STENCIL_VIEW_FAILED, "wgpu: failed to create view object for swapchain depth-stencil texture") \
    _SAPP_LOGITEM_XMACRO(WGPU_SWAPCHAIN_CREATE_MSAA_TEXTURE_FAILED, "wgpu: failed to create msaa texture for swapchain") \
    _SAPP_LOGITEM_XMACRO(WGPU_SWAPCHAIN_CREATE_MSAA_VIEW_FAILED, "wgpu: failed to create view object for swapchain msaa texture") \
    _SAPP_LOGITEM_XMACRO(WGPU_REQUEST_DEVICE_STATUS_ERROR, "wgpu: requesting device failed with status 'error'") \
    _SAPP_LOGITEM_XMACRO(WGPU_REQUEST_DEVICE_STATUS_UNKNOWN, "wgpu: requesting device failed with status 'unknown'") \
    _SAPP_LOGITEM_XMACRO(WGPU_REQUEST_ADAPTER_STATUS_UNAVAILABLE, "wgpu: requesting adapter failed with 'unavailable'") \
    _SAPP_LOGITEM_XMACRO(WGPU_REQUEST_ADAPTER_STATUS_ERROR, "wgpu: requesting adapter failed with status 'error'") \
    _SAPP_LOGITEM_XMACRO(WGPU_REQUEST_ADAPTER_STATUS_UNKNOWN, "wgpu: requesting adapter failed with status 'unknown'") \
    _SAPP_LOGITEM_XMACRO(WGPU_CREATE_INSTANCE_FAILED, "wgpu: failed to create instance") \
    _SAPP_LOGITEM_XMACRO(IMAGE_DATA_SIZE_MISMATCH, "image data size mismatch (must be width*height*4 bytes)") \
    _SAPP_LOGITEM_XMACRO(DROPPED_FILE_PATH_TOO_LONG, "dropped file path too long (sapp_desc.max_dropped_filed_path_length)") \
    _SAPP_LOGITEM_XMACRO(CLIPBOARD_STRING_TOO_BIG, "clipboard string didn't fit into clipboard buffer") \

#define _SAPP_LOGITEM_XMACRO(item,msg) SAPP_LOGITEM_##item,
typedef enum sapp_log_item {
    _SAPP_LOG_ITEMS
} sapp_log_item;
#undef _SAPP_LOGITEM_XMACRO

/*
    sapp_logger

    Used in sapp_desc to provide a logging function. Please be aware that
    without logging function, sokol-app will be completely silent, e.g. it will
    not report errors or warnings. For maximum error verbosity, compile in
    debug mode (e.g. NDEBUG *not* defined) and install a logger (for instance
    the standard logging function from sokol_log.h).
*/
typedef struct sapp_logger {
    void (*func)(
        const char* tag,                // always "sapp"
        uint32_t log_level,             // 0=panic, 1=error, 2=warning, 3=info
        uint32_t log_item_id,           // SAPP_LOGITEM_*
        const char* message_or_null,    // a message string, may be nullptr in release mode
        uint32_t line_nr,               // line number in sokol_app.h
        const char* filename_or_null,   // source filename, may be nullptr in release mode
        void* user_data);
    void* user_data;
} sapp_logger;

typedef struct sapp_desc {
    void (*init_cb)(void);                  // these are the user-provided callbacks without user data
    void (*frame_cb)(void);
    void (*cleanup_cb)(void);
    void (*event_cb)(const sapp_event*);

    void* user_data;                        // these are the user-provided callbacks with user data
    void (*init_userdata_cb)(void*);
    void (*frame_userdata_cb)(void*);
    void (*cleanup_userdata_cb)(void*);
    void (*event_userdata_cb)(const sapp_event*, void*);

    int width;                          // the preferred width of the window / canvas
    int height;                         // the preferred height of the window / canvas
    int sample_count;                   // MSAA sample count
    int swap_interval;                  // the preferred swap interval (ignored on some platforms)
    bool high_dpi;                      // whether the rendering canvas is full-resolution on HighDPI displays
    bool fullscreen;                    // whether the window should be created in fullscreen mode
    bool alpha;                         // whether the framebuffer should have an alpha channel (ignored on some platforms)
    const char* window_title;           // the window title as UTF-8 encoded string
    bool enable_clipboard;              // enable clipboard access, default is false
    int clipboard_size;                 // max size of clipboard content in bytes
    bool enable_dragndrop;              // enable file dropping (drag'n'drop), default is false
    int max_dropped_files;              // max number of dropped files to process (default: 1)
    int max_dropped_file_path_length;   // max length in bytes of a dropped UTF-8 file path (default: 2048)
    sapp_icon_desc icon;                // the initial window icon to set
    sapp_allocator allocator;           // optional memory allocation overrides (default: malloc/free)
    sapp_logger logger;                 // logging callback override (default: NO LOGGING!)

    /* backend-specific options */
    int gl_major_version;               // override GL major and minor version (the default GL version is 3.2)
    int gl_minor_version;
    bool win32_console_utf8;            // if true, set the output console codepage to UTF-8
    bool win32_console_create;          // if true, attach stdout/stderr to a new console window
    bool win32_console_attach;          // if true, attach stdout/stderr to parent process
    const char* html5_canvas_name;      // the name (id) of the HTML5 canvas element, default is "canvas"
    bool html5_canvas_resize;           // if true, the HTML5 canvas size is set to sapp_desc.width/height, otherwise canvas size is tracked
    bool html5_preserve_drawing_buffer; // HTML5 only: whether to preserve default framebuffer content between frames
    bool html5_premultiplied_alpha;     // HTML5 only: whether the rendered pixels use premultiplied alpha convention
    bool html5_ask_leave_site;          // initial state of the internal html5_ask_leave_site flag (see sapp_html5_ask_leave_site())
    bool ios_keyboard_resizes_canvas;   // if true, showing the iOS keyboard shrinks the canvas
} sapp_desc;

/* HTML5 specific: request and response structs for
   asynchronously loading dropped-file content.
*/
typedef enum sapp_html5_fetch_error {
    SAPP_HTML5_FETCH_ERROR_NO_ERROR,
    SAPP_HTML5_FETCH_ERROR_BUFFER_TOO_SMALL,
    SAPP_HTML5_FETCH_ERROR_OTHER,
} sapp_html5_fetch_error;

typedef struct sapp_html5_fetch_response {
    bool succeeded;         // true if the loading operation has succeeded
    sapp_html5_fetch_error error_code;
    int file_index;         // index of the dropped file (0..sapp_get_num_dropped_filed()-1)
    sapp_range data;        // pointer and size of the fetched data (data.ptr == buffer.ptr, data.size <= buffer.size)
    sapp_range buffer;      // the user-provided buffer ptr/size pair (buffer.ptr == data.ptr, buffer.size >= data.size)
    void* user_data;        // user-provided user data pointer
} sapp_html5_fetch_response;

typedef struct sapp_html5_fetch_request {
    int dropped_file_index; // 0..sapp_get_num_dropped_files()-1
    void (*callback)(const sapp_html5_fetch_response*);     // response callback function pointer (required)
    sapp_range buffer;      // ptr/size of a memory buffer to load the data into
    void* user_data;        // optional userdata pointer
} sapp_html5_fetch_request;

/*
    sapp_mouse_cursor

    Predefined cursor image definitions, set with sapp_set_mouse_cursor(sapp_mouse_cursor cursor)
*/
typedef enum sapp_mouse_cursor {
    SAPP_MOUSECURSOR_DEFAULT = 0,   // equivalent with system default cursor
    SAPP_MOUSECURSOR_ARROW,
    SAPP_MOUSECURSOR_IBEAM,
    SAPP_MOUSECURSOR_CROSSHAIR,
    SAPP_MOUSECURSOR_POINTING_HAND,
    SAPP_MOUSECURSOR_RESIZE_EW,
    SAPP_MOUSECURSOR_RESIZE_NS,
    SAPP_MOUSECURSOR_RESIZE_NWSE,
    SAPP_MOUSECURSOR_RESIZE_NESW,
    SAPP_MOUSECURSOR_RESIZE_ALL,
    SAPP_MOUSECURSOR_NOT_ALLOWED,
    _SAPP_MOUSECURSOR_NUM,
} sapp_mouse_cursor;

/* user-provided functions */
extern sapp_desc sokol_main(int argc, char* argv[]);

/* returns true after sokol-app has been initialized */
SOKOL_APP_API_DECL bool sapp_isvalid(void);
/* returns the current framebuffer width in pixels */
SOKOL_APP_API_DECL int sapp_width(void);
/* same as sapp_width(), but returns float */
SOKOL_APP_API_DECL float sapp_widthf(void);
/* returns the current framebuffer height in pixels */
SOKOL_APP_API_DECL int sapp_height(void);
/* same as sapp_height(), but returns float */
SOKOL_APP_API_DECL float sapp_heightf(void);
/* get default framebuffer color pixel format */
SOKOL_APP_API_DECL int sapp_color_format(void);
/* get default framebuffer depth pixel format */
SOKOL_APP_API_DECL int sapp_depth_format(void);
/* get default framebuffer sample count */
SOKOL_APP_API_DECL int sapp_sample_count(void);
/* returns true when high_dpi was requested and actually running in a high-dpi scenario */
SOKOL_APP_API_DECL bool sapp_high_dpi(void);
/* returns the dpi scaling factor (window pixels to framebuffer pixels) */
SOKOL_APP_API_DECL float sapp_dpi_scale(void);
/* show or hide the mobile device onscreen keyboard */
SOKOL_APP_API_DECL void sapp_show_keyboard(bool show);
/* return true if the mobile device onscreen keyboard is currently shown */
SOKOL_APP_API_DECL bool sapp_keyboard_shown(void);
/* query fullscreen mode */
SOKOL_APP_API_DECL bool sapp_is_fullscreen(void);
/* toggle fullscreen mode */
SOKOL_APP_API_DECL void sapp_toggle_fullscreen(void);
/* show or hide the mouse cursor */
SOKOL_APP_API_DECL void sapp_show_mouse(bool show);
/* show or hide the mouse cursor */
SOKOL_APP_API_DECL bool sapp_mouse_shown(void);
/* enable/disable mouse-pointer-lock mode */
SOKOL_APP_API_DECL void sapp_lock_mouse(bool lock);
/* return true if in mouse-pointer-lock mode (this may toggle a few frames later) */
SOKOL_APP_API_DECL bool sapp_mouse_locked(void);
/* set mouse cursor type */
SOKOL_APP_API_DECL void sapp_set_mouse_cursor(sapp_mouse_cursor cursor);
/* get current mouse cursor type */
SOKOL_APP_API_DECL sapp_mouse_cursor sapp_get_mouse_cursor(void);
/* return the userdata pointer optionally provided in sapp_desc */
SOKOL_APP_API_DECL void* sapp_userdata(void);
/* return a copy of the sapp_desc structure */
SOKOL_APP_API_DECL sapp_desc sapp_query_desc(void);
/* initiate a "soft quit" (sends SAPP_EVENTTYPE_QUIT_REQUESTED) */
SOKOL_APP_API_DECL void sapp_request_quit(void);
/* cancel a pending quit (when SAPP_EVENTTYPE_QUIT_REQUESTED has been received) */
SOKOL_APP_API_DECL void sapp_cancel_quit(void);
/* initiate a "hard quit" (quit application without sending SAPP_EVENTTYPE_QUIT_REQUSTED) */
SOKOL_APP_API_DECL void sapp_quit(void);
/* call from inside event callback to consume the current event (don't forward to platform) */
SOKOL_APP_API_DECL void sapp_consume_event(void);
/* get the current frame counter (for comparison with sapp_event.frame_count) */
SOKOL_APP_API_DECL uint64_t sapp_frame_count(void);
/* get an averaged/smoothed frame duration in seconds */
SOKOL_APP_API_DECL double sapp_frame_duration(void);
/* write string into clipboard */
SOKOL_APP_API_DECL void sapp_set_clipboard_string(const char* str);
/* read string from clipboard (usually during SAPP_EVENTTYPE_CLIPBOARD_PASTED) */
SOKOL_APP_API_DECL const char* sapp_get_clipboard_string(void);
/* set the window title (only on desktop platforms) */
SOKOL_APP_API_DECL void sapp_set_window_title(const char* str);
/* set the window icon (only on Windows and Linux) */
SOKOL_APP_API_DECL void sapp_set_icon(const sapp_icon_desc* icon_desc);
/* gets the total number of dropped files (after an SAPP_EVENTTYPE_FILES_DROPPED event) */
SOKOL_APP_API_DECL int sapp_get_num_dropped_files(void);
/* gets the dropped file paths */
SOKOL_APP_API_DECL const char* sapp_get_dropped_file_path(int index);

/* special run-function for SOKOL_NO_ENTRY (in standard mode this is an empty stub) */
SOKOL_APP_API_DECL void sapp_run(const sapp_desc* desc);

/* EGL: get EGLDisplay object */
SOKOL_APP_API_DECL const void* sapp_egl_get_display(void);
/* EGL: get EGLContext object */
SOKOL_APP_API_DECL const void* sapp_egl_get_context(void);

/* HTML5: enable or disable the hardwired "Leave Site?" dialog box */
SOKOL_APP_API_DECL void sapp_html5_ask_leave_site(bool ask);
/* HTML5: get byte size of a dropped file */
SOKOL_APP_API_DECL uint32_t sapp_html5_get_dropped_file_size(int index);
/* HTML5: asynchronously load the content of a dropped file */
SOKOL_APP_API_DECL void sapp_html5_fetch_dropped_file(const sapp_html5_fetch_request* request);

/* Metal: get bridged pointer to Metal device object */
SOKOL_APP_API_DECL const void* sapp_metal_get_device(void);
/* Metal: get bridged pointer to this frame's renderpass descriptor */
SOKOL_APP_API_DECL const void* sapp_metal_get_renderpass_descriptor(void);
/* Metal: get bridged pointer to current drawable */
SOKOL_APP_API_DECL const void* sapp_metal_get_drawable(void);
/* macOS: get bridged pointer to macOS NSWindow */
SOKOL_APP_API_DECL const void* sapp_macos_get_window(void);
/* iOS: get bridged pointer to iOS UIWindow */
SOKOL_APP_API_DECL const void* sapp_ios_get_window(void);

/* D3D11: get pointer to ID3D11Device object */
SOKOL_APP_API_DECL const void* sapp_d3d11_get_device(void);
/* D3D11: get pointer to ID3D11DeviceContext object */
SOKOL_APP_API_DECL const void* sapp_d3d11_get_device_context(void);
/* D3D11: get pointer to IDXGISwapChain object */
SOKOL_APP_API_DECL const void* sapp_d3d11_get_swap_chain(void);
/* D3D11: get pointer to ID3D11RenderTargetView object */
SOKOL_APP_API_DECL const void* sapp_d3d11_get_render_target_view(void);
/* D3D11: get pointer to ID3D11DepthStencilView */
SOKOL_APP_API_DECL const void* sapp_d3d11_get_depth_stencil_view(void);
/* Win32: get the HWND window handle */
SOKOL_APP_API_DECL const void* sapp_win32_get_hwnd(void);

/* WebGPU: get WGPUDevice handle */
SOKOL_APP_API_DECL const void* sapp_wgpu_get_device(void);
/* WebGPU: get swapchain's WGPUTextureView handle for rendering */
SOKOL_APP_API_DECL const void* sapp_wgpu_get_render_view(void);
/* WebGPU: get swapchain's MSAA-resolve WGPUTextureView (may return null) */
SOKOL_APP_API_DECL const void* sapp_wgpu_get_resolve_view(void);
/* WebGPU: get swapchain's WGPUTextureView for the depth-stencil surface */
SOKOL_APP_API_DECL const void* sapp_wgpu_get_depth_stencil_view(void);

#ifdef __cplusplus
} /* extern "C" */

/* reference-based equivalents for C++ */
inline void sapp_run(const sapp_desc& desc) { return sapp_run(&desc); }

#endif

// this WinRT specific hack is required when wWinMain is in a static library
#if defined(_MSC_VER) && defined(UNICODE)
#include <winapifamily.h>
#if defined(WINAPI_FAMILY_PARTITION) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#pragma comment(linker, "/include:wWinMain")
#endif
#endif

#endif // SOKOL_APP_INCLUDED

// ██ ███    ███ ██████  ██      ███████ ███    ███ ███████ ███    ██ ████████  █████  ████████ ██  ██████  ███    ██
// ██ ████  ████ ██   ██ ██      ██      ████  ████ ██      ████   ██    ██    ██   ██    ██    ██ ██    ██ ████   ██
// ██ ██ ████ ██ ██████  ██      █████   ██ ████ ██ █████   ██ ██  ██    ██    ███████    ██    ██ ██    ██ ██ ██  ██
// ██ ██  ██  ██ ██      ██      ██      ██  ██  ██ ██      ██  ██ ██    ██    ██   ██    ██    ██ ██    ██ ██  ██ ██
// ██ ██      ██ ██      ███████ ███████ ██      ██ ███████ ██   ████    ██    ██   ██    ██    ██  ██████  ██   ████
//
// >>implementation
#ifdef SOKOL_APP_IMPL
#define SOKOL_APP_IMPL_INCLUDED (1)

#if defined(SOKOL_MALLOC) || defined(SOKOL_CALLOC) || defined(SOKOL_FREE)
#error "SOKOL_MALLOC/CALLOC/FREE macros are no longer supported, please use sapp_desc.allocator to override memory allocation functions"
#endif

#include <stdlib.h> // malloc, free
#include <string.h> // memset
#include <stddef.h> // size_t
#include <math.h>   // roundf

// helper macros
#define _sapp_def(val, def) (((val) == 0) ? (def) : (val))
#define _sapp_absf(a) (((a)<0.0f)?-(a):(a))

#define _SAPP_MAX_TITLE_LENGTH (128)
#define _SAPP_FALLBACK_DEFAULT_WINDOW_WIDTH (640)
#define _SAPP_FALLBACK_DEFAULT_WINDOW_HEIGHT (480)
// NOTE: the pixel format values *must* be compatible with sg_pixel_format
#define _SAPP_PIXELFORMAT_RGBA8 (23)
#define _SAPP_PIXELFORMAT_BGRA8 (28)
#define _SAPP_PIXELFORMAT_DEPTH (42)
#define _SAPP_PIXELFORMAT_DEPTH_STENCIL (43)

#if defined(_SAPP_MACOS) || defined(_SAPP_IOS)
    // this is ARC compatible
    #if defined(__cplusplus)
        #define _SAPP_CLEAR_ARC_STRUCT(type, item) { item = type(); }
    #else
        #define _SAPP_CLEAR_ARC_STRUCT(type, item) { item = (type) { 0 }; }
    #endif
#else
    #define _SAPP_CLEAR_ARC_STRUCT(type, item) { _sapp_clear(&item, sizeof(item)); }
#endif

// check if the config defines are alright
#if defined(__APPLE__)
    // see https://clang.llvm.org/docs/LanguageExtensions.html#automatic-reference-counting
    #if !defined(__cplusplus)
        #if __has_feature(objc_arc) && !__has_feature(objc_arc_fields)
            #error "sokol_app.h requires __has_feature(objc_arc_field) if ARC is enabled (use a more recent compiler version)"
        #endif
    #endif
    #define _SAPP_APPLE (1)
    #include <TargetConditionals.h>
    #if defined(TARGET_OS_IPHONE) && !TARGET_OS_IPHONE
        /* MacOS */
        #define _SAPP_MACOS (1)
        #if !defined(SOKOL_METAL) && !defined(SOKOL_GLCORE33)
        #error("sokol_app.h: unknown 3D API selected for MacOS, must be SOKOL_METAL or SOKOL_GLCORE33")
        #endif
    #else
        /* iOS or iOS Simulator */
        #define _SAPP_IOS (1)
        #if !defined(SOKOL_METAL) && !defined(SOKOL_GLES3)
        #error("sokol_app.h: unknown 3D API selected for iOS, must be SOKOL_METAL or SOKOL_GLES3")
        #endif
    #endif
#elif defined(_WIN32)
    /* Windows (D3D11 or GL) */
    #define _SAPP_WIN32 (1)
    #if !defined(SOKOL_D3D11) && !defined(SOKOL_GLCORE33)
    #error("sokol_app.h: unknown 3D API selected for Win32, must be SOKOL_D3D11 or SOKOL_GLCORE33")
    #endif
#elif defined(__ANDROID__)
    /* Android */
    #define _SAPP_ANDROID (1)
    #if !defined(SOKOL_GLES3)
    #error("sokol_app.h: unknown 3D API selected for Android, must be SOKOL_GLES3")
    #endif
    #if defined(SOKOL_NO_ENTRY)
    #error("sokol_app.h: SOKOL_NO_ENTRY is not supported on Android")
    #endif
#else
#error "sokol_app.h: Unknown platform"
#endif

#ifndef SOKOL_API_IMPL
    #define SOKOL_API_IMPL
#endif
#ifndef SOKOL_DEBUG
    #ifndef NDEBUG
        #define SOKOL_DEBUG
    #endif
#endif
#ifndef SOKOL_ASSERT
    #include <assert.h>
    #define SOKOL_ASSERT(c) assert(c)
#endif
#ifndef SOKOL_UNREACHABLE
    #define SOKOL_UNREACHABLE SOKOL_ASSERT(false)
#endif

#ifndef _SOKOL_PRIVATE
    #if defined(__GNUC__) || defined(__clang__)
        #define _SOKOL_PRIVATE __attribute__((unused)) static
    #else
        #define _SOKOL_PRIVATE static
    #endif
#endif
#ifndef _SOKOL_UNUSED
    #define _SOKOL_UNUSED(x) (void)(x)
#endif

#if defined(_SAPP_APPLE)
    #if defined(SOKOL_METAL)
        #import <Metal/Metal.h>
        #import <MetalKit/MetalKit.h>
    #endif
    #if defined(_SAPP_MACOS)
        #if !defined(SOKOL_METAL)
            #ifndef GL_SILENCE_DEPRECATION
            #define GL_SILENCE_DEPRECATION
            #endif
            #include <Cocoa/Cocoa.h>
        #endif
    #elif defined(_SAPP_IOS)
        #import <UIKit/UIKit.h>
        #if !defined(SOKOL_METAL)
            #import <GLKit/GLKit.h>
        #endif
    #endif
    #include <AvailabilityMacros.h>
    #include <mach/mach_time.h>
#elif defined(_SAPP_WIN32)
    #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable:4201)   /* nonstandard extension used: nameless struct/union */
        #pragma warning(disable:4204)   /* nonstandard extension used: non-constant aggregate initializer */
        #pragma warning(disable:4054)   /* 'type cast': from function pointer */
        #pragma warning(disable:4055)   /* 'type cast': from data pointer */
        #pragma warning(disable:4505)   /* unreferenced local function has been removed */
        #pragma warning(disable:4115)   /* /W4: 'ID3D11ModuleInstance': named type definition in parentheses (in d3d11.h) */
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include <windowsx.h>
    #include <shellapi.h>
    #if !defined(SOKOL_NO_ENTRY)    // if SOKOL_NO_ENTRY is defined, it's the applications' responsibility to use the right subsystem
        #if defined(SOKOL_WIN32_FORCE_MAIN)
            #pragma comment (linker, "/subsystem:console")
        #else
            #pragma comment (linker, "/subsystem:windows")
        #endif
    #endif
    #include <stdio.h>  /* freopen_s() */
    #include <wchar.h>  /* wcslen() */

    #pragma comment (lib, "kernel32")
    #pragma comment (lib, "user32")
    #pragma comment (lib, "shell32")    /* CommandLineToArgvW, DragQueryFileW, DragFinished */
    #pragma comment (lib, "gdi32")
    #if defined(SOKOL_D3D11)
        #pragma comment (lib, "dxgi")
        #pragma comment (lib, "d3d11")
    #endif

    #if defined(SOKOL_D3D11)
        #ifndef D3D11_NO_HELPERS
            #define D3D11_NO_HELPERS
        #endif
        #include <d3d11.h>
        #include <dxgi.h>
        // DXGI_SWAP_EFFECT_FLIP_DISCARD is only defined in newer Windows SDKs, so don't depend on it
        #define _SAPP_DXGI_SWAP_EFFECT_FLIP_DISCARD (4)
    #endif
    #ifndef WM_MOUSEHWHEEL /* see https://github.com/floooh/sokol/issues/138 */
        #define WM_MOUSEHWHEEL (0x020E)
    #endif
    #ifndef WM_DPICHANGED
        #define WM_DPICHANGED (0x02E0)
    #endif
#endif

// ███████ ██████   █████  ███    ███ ███████     ████████ ██ ███    ███ ██ ███    ██  ██████
// ██      ██   ██ ██   ██ ████  ████ ██             ██    ██ ████  ████ ██ ████   ██ ██
// █████   ██████  ███████ ██ ████ ██ █████          ██    ██ ██ ████ ██ ██ ██ ██  ██ ██   ███
// ██      ██   ██ ██   ██ ██  ██  ██ ██             ██    ██ ██  ██  ██ ██ ██  ██ ██ ██    ██
// ██      ██   ██ ██   ██ ██      ██ ███████        ██    ██ ██      ██ ██ ██   ████  ██████
//
// >>frame timing
#define _SAPP_RING_NUM_SLOTS (256)
typedef struct {
    int head;
    int tail;
    double buf[_SAPP_RING_NUM_SLOTS];
} _sapp_ring_t;

_SOKOL_PRIVATE int _sapp_ring_idx(int i) {
    return i % _SAPP_RING_NUM_SLOTS;
}

_SOKOL_PRIVATE void _sapp_ring_init(_sapp_ring_t* ring) {
    ring->head = 0;
    ring->tail = 0;
}

_SOKOL_PRIVATE bool _sapp_ring_full(_sapp_ring_t* ring) {
    return _sapp_ring_idx(ring->head + 1) == ring->tail;
}

_SOKOL_PRIVATE bool _sapp_ring_empty(_sapp_ring_t* ring) {
    return ring->head == ring->tail;
}

_SOKOL_PRIVATE int _sapp_ring_count(_sapp_ring_t* ring) {
    int count;
    if (ring->head >= ring->tail) {
        count = ring->head - ring->tail;
    }
    else {
        count = (ring->head + _SAPP_RING_NUM_SLOTS) - ring->tail;
    }
    SOKOL_ASSERT((count >= 0) && (count < _SAPP_RING_NUM_SLOTS));
    return count;
}

_SOKOL_PRIVATE void _sapp_ring_enqueue(_sapp_ring_t* ring, double val) {
    SOKOL_ASSERT(!_sapp_ring_full(ring));
    ring->buf[ring->head] = val;
    ring->head = _sapp_ring_idx(ring->head + 1);
}

_SOKOL_PRIVATE double _sapp_ring_dequeue(_sapp_ring_t* ring) {
    SOKOL_ASSERT(!_sapp_ring_empty(ring));
    double val = ring->buf[ring->tail];
    ring->tail = _sapp_ring_idx(ring->tail + 1);
    return val;
}

/*
    NOTE:

    Q: Why not use CAMetalDrawable.presentedTime on macOS and iOS?
    A: The value appears to be highly unstable during the first few
    seconds, sometimes several frames are dropped in sequence, or
    switch between 120 and 60 Hz for a few frames. Simply measuring
    and averaging the frame time yielded a more stable frame duration.
    Maybe switching to CVDisplayLink would yield better results.
    Until then just measure the time.
*/
typedef struct {
    #if defined(_SAPP_APPLE)
        struct {
            mach_timebase_info_data_t timebase;
            uint64_t start;
        } mach;
    #elif defined(_SAPP_WIN32)
        struct {
            LARGE_INTEGER freq;
            LARGE_INTEGER start;
        } win;
    #else // Linux, Android, ...
        #ifdef CLOCK_MONOTONIC
        #define _SAPP_CLOCK_MONOTONIC CLOCK_MONOTONIC
        #else
        // on some embedded platforms, CLOCK_MONOTONIC isn't defined
        #define _SAPP_CLOCK_MONOTONIC (1)
        #endif
        struct {
            uint64_t start;
        } posix;
    #endif
} _sapp_timestamp_t;

_SOKOL_PRIVATE int64_t _sapp_int64_muldiv(int64_t value, int64_t numer, int64_t denom) {
    int64_t q = value / denom;
    int64_t r = value % denom;
    return q * numer + r * numer / denom;
}

_SOKOL_PRIVATE void _sapp_timestamp_init(_sapp_timestamp_t* ts) {
    #if defined(_SAPP_APPLE)
        mach_timebase_info(&ts->mach.timebase);
        ts->mach.start = mach_absolute_time();
    #elif defined(_SAPP_WIN32)
        QueryPerformanceFrequency(&ts->win.freq);
        QueryPerformanceCounter(&ts->win.start);
    #else
        struct timespec tspec;
        clock_gettime(_SAPP_CLOCK_MONOTONIC, &tspec);
        ts->posix.start = (uint64_t)tspec.tv_sec*1000000000 + (uint64_t)tspec.tv_nsec;
    #endif
}

_SOKOL_PRIVATE double _sapp_timestamp_now(_sapp_timestamp_t* ts) {
    #if defined(_SAPP_APPLE)
        const uint64_t traw = mach_absolute_time() - ts->mach.start;
        const uint64_t now = (uint64_t) _sapp_int64_muldiv((int64_t)traw, (int64_t)ts->mach.timebase.numer, (int64_t)ts->mach.timebase.denom);
        return (double)now / 1000000000.0;
    #elif defined(_SAPP_WIN32)
        LARGE_INTEGER qpc;
        QueryPerformanceCounter(&qpc);
        const uint64_t now = (uint64_t)_sapp_int64_muldiv(qpc.QuadPart - ts->win.start.QuadPart, 1000000000, ts->win.freq.QuadPart);
        return (double)now / 1000000000.0;
    #else
        struct timespec tspec;
        clock_gettime(_SAPP_CLOCK_MONOTONIC, &tspec);
        const uint64_t now = ((uint64_t)tspec.tv_sec*1000000000 + (uint64_t)tspec.tv_nsec) - ts->posix.start;
        return (double)now / 1000000000.0;
    #endif
}

typedef struct {
    double last;
    double accum;
    double avg;
    int spike_count;
    int num;
    _sapp_timestamp_t timestamp;
    _sapp_ring_t ring;
} _sapp_timing_t;

_SOKOL_PRIVATE void _sapp_timing_reset(_sapp_timing_t* t) {
    t->last = 0.0;
    t->accum = 0.0;
    t->spike_count = 0;
    t->num = 0;
    _sapp_ring_init(&t->ring);
}

_SOKOL_PRIVATE void _sapp_timing_init(_sapp_timing_t* t) {
    t->avg = 1.0 / 60.0;    // dummy value until first actual value is available
    _sapp_timing_reset(t);
    _sapp_timestamp_init(&t->timestamp);
}

_SOKOL_PRIVATE void _sapp_timing_put(_sapp_timing_t* t, double dur) {
    // arbitrary upper limit to ignore outliers (e.g. during window resizing, or debugging)
    double min_dur = 0.0;
    double max_dur = 0.1;
    // if we have enough samples for a useful average, use a much tighter 'valid window'
    if (_sapp_ring_full(&t->ring)) {
        min_dur = t->avg * 0.8;
        max_dur = t->avg * 1.2;
    }
    if ((dur < min_dur) || (dur > max_dur)) {
        t->spike_count++;
        // if there have been many spikes in a row, the display refresh rate
        // might have changed, so a timing reset is needed
        if (t->spike_count > 20) {
            _sapp_timing_reset(t);
        }
        return;
    }
    if (_sapp_ring_full(&t->ring)) {
        double old_val = _sapp_ring_dequeue(&t->ring);
        t->accum -= old_val;
        t->num -= 1;
    }
    _sapp_ring_enqueue(&t->ring, dur);
    t->accum += dur;
    t->num += 1;
    SOKOL_ASSERT(t->num > 0);
    t->avg = t->accum / t->num;
    t->spike_count = 0;
}

_SOKOL_PRIVATE void _sapp_timing_discontinuity(_sapp_timing_t* t) {
    t->last = 0.0;
}

_SOKOL_PRIVATE void _sapp_timing_measure(_sapp_timing_t* t) {
    const double now = _sapp_timestamp_now(&t->timestamp);
    if (t->last > 0.0) {
        double dur = now - t->last;
        _sapp_timing_put(t, dur);
    }
    t->last = now;
}

_SOKOL_PRIVATE void _sapp_timing_external(_sapp_timing_t* t, double now) {
    if (t->last > 0.0) {
        double dur = now - t->last;
        _sapp_timing_put(t, dur);
    }
    t->last = now;
}

_SOKOL_PRIVATE double _sapp_timing_get_avg(_sapp_timing_t* t) {
    return t->avg;
}

// ███████ ████████ ██████  ██    ██  ██████ ████████ ███████
// ██         ██    ██   ██ ██    ██ ██         ██    ██
// ███████    ██    ██████  ██    ██ ██         ██    ███████
//      ██    ██    ██   ██ ██    ██ ██         ██         ██
// ███████    ██    ██   ██  ██████   ██████    ██    ███████
//
// >> structs
#if defined(_SAPP_MACOS)
@interface _sapp_macos_app_delegate : NSObject<NSApplicationDelegate>
@end
@interface _sapp_macos_window : NSWindow
@end
@interface _sapp_macos_window_delegate : NSObject<NSWindowDelegate>
@end
#if defined(SOKOL_METAL)
    @interface _sapp_macos_view : MTKView
    @end
#elif defined(SOKOL_GLCORE33)
    @interface _sapp_macos_view : NSOpenGLView
    - (void)timerFired:(id)sender;
    @end
#endif // SOKOL_GLCORE33

typedef struct {
    uint32_t flags_changed_store;
    uint8_t mouse_buttons;
    NSWindow* window;
    NSTrackingArea* tracking_area;
    id keyup_monitor;
    _sapp_macos_app_delegate* app_dlg;
    _sapp_macos_window_delegate* win_dlg;
    _sapp_macos_view* view;
    NSCursor* cursors[_SAPP_MOUSECURSOR_NUM];
    #if defined(SOKOL_METAL)
        id<MTLDevice> mtl_device;
    #endif
} _sapp_macos_t;

#endif // _SAPP_MACOS

#if defined(_SAPP_IOS)

@interface _sapp_app_delegate : NSObject<UIApplicationDelegate>
@end
@interface _sapp_textfield_dlg : NSObject<UITextFieldDelegate>
- (void)keyboardWasShown:(NSNotification*)notif;
- (void)keyboardWillBeHidden:(NSNotification*)notif;
- (void)keyboardDidChangeFrame:(NSNotification*)notif;
@end
#if defined(SOKOL_METAL)
    @interface _sapp_ios_view : MTKView;
    @end
#else
    @interface _sapp_ios_view : GLKView
    @end
#endif

typedef struct {
    UIWindow* window;
    _sapp_ios_view* view;
    UITextField* textfield;
    _sapp_textfield_dlg* textfield_dlg;
    #if defined(SOKOL_METAL)
        UIViewController* view_ctrl;
        id<MTLDevice> mtl_device;
    #else
        GLKViewController* view_ctrl;
        EAGLContext* eagl_ctx;
    #endif
    bool suspended;
} _sapp_ios_t;

#endif // _SAPP_IOS

#if defined(SOKOL_D3D11) && defined(_SAPP_WIN32)
typedef struct {
    ID3D11Device* device;
    ID3D11DeviceContext* device_context;
    ID3D11Texture2D* rt;
    ID3D11RenderTargetView* rtv;
    ID3D11Texture2D* msaa_rt;
    ID3D11RenderTargetView* msaa_rtv;
    ID3D11Texture2D* ds;
    ID3D11DepthStencilView* dsv;
    DXGI_SWAP_CHAIN_DESC swap_chain_desc;
    IDXGISwapChain* swap_chain;
    IDXGIDevice1* dxgi_device;
    bool use_dxgi_frame_stats;
    UINT sync_refresh_count;
} _sapp_d3d11_t;
#endif

#if defined(_SAPP_WIN32)

#ifndef DPI_ENUMS_DECLARED
typedef enum PROCESS_DPI_AWARENESS
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
typedef enum MONITOR_DPI_TYPE {
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;
#endif // DPI_ENUMS_DECLARED

typedef struct {
    bool aware;
    float content_scale;
    float window_scale;
    float mouse_scale;
} _sapp_win32_dpi_t;

typedef struct {
    HWND hwnd;
    HMONITOR hmonitor;
    HDC dc;
    HICON big_icon;
    HICON small_icon;
    HCURSOR cursors[_SAPP_MOUSECURSOR_NUM];
    UINT orig_codepage;
    LONG mouse_locked_x, mouse_locked_y;
    RECT stored_window_rect;    // used to restore window pos/size when toggling fullscreen => windowed
    bool is_win10_or_greater;
    bool in_create_window;
    bool iconified;
    bool mouse_tracked;
    uint8_t mouse_capture_mask;
    _sapp_win32_dpi_t dpi;
    bool raw_input_mousepos_valid;
    LONG raw_input_mousepos_x;
    LONG raw_input_mousepos_y;
    uint8_t raw_input_data[256];
} _sapp_win32_t;

#if defined(SOKOL_GLCORE33)
#define WGL_NUMBER_PIXEL_FORMATS_ARB 0x2000
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_TYPE_RGBA_ARB 0x202b
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_NO_ACCELERATION_ARB 0x2025
#define WGL_RED_BITS_ARB 0x2015
#define WGL_GREEN_BITS_ARB 0x2017
#define WGL_BLUE_BITS_ARB 0x2019
#define WGL_ALPHA_BITS_ARB 0x201b
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_SAMPLES_ARB 0x2042
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x00000001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define ERROR_INVALID_VERSION_ARB 0x2095
#define ERROR_INVALID_PROFILE_ARB 0x2096
#define ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB 0x2054
typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int);
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBIVARBPROC)(HDC,int,int,UINT,const int*,int*);
typedef const char* (WINAPI * PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);
typedef const char* (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC);
typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC,HGLRC,const int*);
typedef HGLRC (WINAPI * PFN_wglCreateContext)(HDC);
typedef BOOL (WINAPI * PFN_wglDeleteContext)(HGLRC);
typedef PROC (WINAPI * PFN_wglGetProcAddress)(LPCSTR);
typedef HDC (WINAPI * PFN_wglGetCurrentDC)(void);
typedef BOOL (WINAPI * PFN_wglMakeCurrent)(HDC,HGLRC);

typedef struct {
    HINSTANCE opengl32;
    HGLRC gl_ctx;
    PFN_wglCreateContext CreateContext;
    PFN_wglDeleteContext DeleteContext;
    PFN_wglGetProcAddress GetProcAddress;
    PFN_wglGetCurrentDC GetCurrentDC;
    PFN_wglMakeCurrent MakeCurrent;
    PFNWGLSWAPINTERVALEXTPROC SwapIntervalEXT;
    PFNWGLGETPIXELFORMATATTRIBIVARBPROC GetPixelFormatAttribivARB;
    PFNWGLGETEXTENSIONSSTRINGEXTPROC GetExtensionsStringEXT;
    PFNWGLGETEXTENSIONSSTRINGARBPROC GetExtensionsStringARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB;
    bool ext_swap_control;
    bool arb_multisample;
    bool arb_pixel_format;
    bool arb_create_context;
    bool arb_create_context_profile;
    HWND msg_hwnd;
    HDC msg_dc;
} _sapp_wgl_t;
#endif // SOKOL_GLCORE33

#endif // _SAPP_WIN32

typedef struct {
    bool enabled;
    int buf_size;
    char* buffer;
} _sapp_clipboard_t;

typedef struct {
    bool enabled;
    int max_files;
    int max_path_length;
    int num_files;
    int buf_size;
    char* buffer;
} _sapp_drop_t;

typedef struct {
    float x, y;
    float dx, dy;
    bool shown;
    bool locked;
    bool pos_valid;
    sapp_mouse_cursor current_cursor;
} _sapp_mouse_t;

typedef struct {
    sapp_desc desc;
    bool valid;
    bool fullscreen;
    bool first_frame;
    bool init_called;
    bool cleanup_called;
    bool quit_requested;
    bool quit_ordered;
    bool event_consumed;
    bool html5_ask_leave_site;
    bool onscreen_keyboard_shown;
    int window_width;
    int window_height;
    int framebuffer_width;
    int framebuffer_height;
    int sample_count;
    int swap_interval;
    float dpi_scale;
    uint64_t frame_count;
    _sapp_timing_t timing;
    sapp_event event;
    _sapp_mouse_t mouse;
    _sapp_clipboard_t clipboard;
    _sapp_drop_t drop;
    sapp_icon_desc default_icon_desc;
    uint32_t* default_icon_pixels;
    #if defined(_SAPP_MACOS)
        _sapp_macos_t macos;
    #elif defined(_SAPP_IOS)
        _sapp_ios_t ios;
    #elif defined(_SAPP_WIN32)
        _sapp_win32_t win32;
        #if defined(SOKOL_D3D11)
            _sapp_d3d11_t d3d11;
        #elif defined(SOKOL_GLCORE33)
            _sapp_wgl_t wgl;
        #endif
    #endif
    char html5_canvas_selector[_SAPP_MAX_TITLE_LENGTH];
    char window_title[_SAPP_MAX_TITLE_LENGTH];      // UTF-8
    wchar_t window_title_wide[_SAPP_MAX_TITLE_LENGTH];   // UTF-32 or UCS-2 */
    sapp_keycode keycodes[SAPP_MAX_KEYCODES];
} _sapp_t;
static _sapp_t _sapp;

// ██       ██████   ██████   ██████  ██ ███    ██  ██████
// ██      ██    ██ ██       ██       ██ ████   ██ ██
// ██      ██    ██ ██   ███ ██   ███ ██ ██ ██  ██ ██   ███
// ██      ██    ██ ██    ██ ██    ██ ██ ██  ██ ██ ██    ██
// ███████  ██████   ██████   ██████  ██ ██   ████  ██████
//
// >>logging
#if defined(SOKOL_DEBUG)
#define _SAPP_LOGITEM_XMACRO(item,msg) #item ": " msg,
static const char* _sapp_log_messages[] = {
    _SAPP_LOG_ITEMS
};
#undef _SAPP_LOGITEM_XMACRO
#endif // SOKOL_DEBUG

#define _SAPP_PANIC(code) _sapp_log(SAPP_LOGITEM_ ##code, 0, 0, __LINE__)
#define _SAPP_ERROR(code) _sapp_log(SAPP_LOGITEM_ ##code, 1, 0, __LINE__)
#define _SAPP_WARN(code) _sapp_log(SAPP_LOGITEM_ ##code, 2, 0, __LINE__)
#define _SAPP_INFO(code) _sapp_log(SAPP_LOGITEM_ ##code, 3, 0, __LINE__)

static void _sapp_log(sapp_log_item log_item, uint32_t log_level, const char* msg, uint32_t line_nr) {
    if (_sapp.desc.logger.func) {
        const char* filename = 0;
        #if defined(SOKOL_DEBUG)
            filename = __FILE__;
            if (0 == msg) {
                msg = _sapp_log_messages[log_item];
            }
        #endif
        _sapp.desc.logger.func("sapp", log_level, log_item, msg, line_nr, filename, _sapp.desc.logger.user_data);
    }
    else {
        // for log level PANIC it would be 'undefined behaviour' to continue
        if (log_level == 0) {
            abort();
        }
    }
}

// ███    ███ ███████ ███    ███  ██████  ██████  ██    ██
// ████  ████ ██      ████  ████ ██    ██ ██   ██  ██  ██
// ██ ████ ██ █████   ██ ████ ██ ██    ██ ██████    ████
// ██  ██  ██ ██      ██  ██  ██ ██    ██ ██   ██    ██
// ██      ██ ███████ ██      ██  ██████  ██   ██    ██
//
// >>memory
_SOKOL_PRIVATE void _sapp_clear(void* ptr, size_t size) {
    SOKOL_ASSERT(ptr && (size > 0));
    memset(ptr, 0, size);
}

_SOKOL_PRIVATE void* _sapp_malloc(size_t size) {
    SOKOL_ASSERT(size > 0);
    void* ptr;
    if (_sapp.desc.allocator.alloc_fn) {
        ptr = _sapp.desc.allocator.alloc_fn(size, _sapp.desc.allocator.user_data);
    } else {
        ptr = malloc(size);
    }
    if (0 == ptr) {
        _SAPP_PANIC(MALLOC_FAILED);
    }
    return ptr;
}

_SOKOL_PRIVATE void* _sapp_malloc_clear(size_t size) {
    void* ptr = _sapp_malloc(size);
    _sapp_clear(ptr, size);
    return ptr;
}

_SOKOL_PRIVATE void _sapp_free(void* ptr) {
    if (_sapp.desc.allocator.free_fn) {
        _sapp.desc.allocator.free_fn(ptr, _sapp.desc.allocator.user_data);
    }
    else {
        free(ptr);
    }
}

// ██   ██ ███████ ██      ██████  ███████ ██████  ███████
// ██   ██ ██      ██      ██   ██ ██      ██   ██ ██
// ███████ █████   ██      ██████  █████   ██████  ███████
// ██   ██ ██      ██      ██      ██      ██   ██      ██
// ██   ██ ███████ ███████ ██      ███████ ██   ██ ███████
//
// >>helpers
_SOKOL_PRIVATE void _sapp_call_init(void) {
    if (_sapp.desc.init_cb) {
        _sapp.desc.init_cb();
    }
    else if (_sapp.desc.init_userdata_cb) {
        _sapp.desc.init_userdata_cb(_sapp.desc.user_data);
    }
    _sapp.init_called = true;
}

_SOKOL_PRIVATE void _sapp_call_frame(void) {
    if (_sapp.init_called && !_sapp.cleanup_called) {
        if (_sapp.desc.frame_cb) {
            _sapp.desc.frame_cb();
        }
        else if (_sapp.desc.frame_userdata_cb) {
            _sapp.desc.frame_userdata_cb(_sapp.desc.user_data);
        }
    }
}

_SOKOL_PRIVATE void _sapp_call_cleanup(void) {
    if (!_sapp.cleanup_called) {
        if (_sapp.desc.cleanup_cb) {
            _sapp.desc.cleanup_cb();
        }
        else if (_sapp.desc.cleanup_userdata_cb) {
            _sapp.desc.cleanup_userdata_cb(_sapp.desc.user_data);
        }
        _sapp.cleanup_called = true;
    }
}

_SOKOL_PRIVATE bool _sapp_call_event(const sapp_event* e) {
    if (!_sapp.cleanup_called) {
        if (_sapp.desc.event_cb) {
            _sapp.desc.event_cb(e);
        }
        else if (_sapp.desc.event_userdata_cb) {
            _sapp.desc.event_userdata_cb(e, _sapp.desc.user_data);
        }
    }
    if (_sapp.event_consumed) {
        _sapp.event_consumed = false;
        return true;
    }
    else {
        return false;
    }
}

_SOKOL_PRIVATE char* _sapp_dropped_file_path_ptr(int index) {
    SOKOL_ASSERT(_sapp.drop.buffer);
    SOKOL_ASSERT((index >= 0) && (index <= _sapp.drop.max_files));
    int offset = index * _sapp.drop.max_path_length;
    SOKOL_ASSERT(offset < _sapp.drop.buf_size);
    return &_sapp.drop.buffer[offset];
}

/* Copy a string into a fixed size buffer with guaranteed zero-
   termination.

   Return false if the string didn't fit into the buffer and had to be clamped.

   FIXME: Currently UTF-8 strings might become invalid if the string
   is clamped, because the last zero-byte might be written into
   the middle of a multi-byte sequence.
*/
_SOKOL_PRIVATE bool _sapp_strcpy(const char* src, char* dst, int max_len) {
    SOKOL_ASSERT(src && dst && (max_len > 0));
    char* const end = &(dst[max_len-1]);
    char c = 0;
    for (int i = 0; i < max_len; i++) {
        c = *src;
        if (c != 0) {
            src++;
        }
        *dst++ = c;
    }
    /* truncated? */
    if (c != 0) {
        *end = 0;
        return false;
    }
    else {
        return true;
    }
}

_SOKOL_PRIVATE sapp_desc _sapp_desc_defaults(const sapp_desc* desc) {
    SOKOL_ASSERT((desc->allocator.alloc_fn && desc->allocator.free_fn) || (!desc->allocator.alloc_fn && !desc->allocator.free_fn));
    sapp_desc res = *desc;
    res.sample_count = _sapp_def(res.sample_count, 1);
    res.swap_interval = _sapp_def(res.swap_interval, 1);
    // NOTE: can't patch the default for gl_major_version and gl_minor_version
    // independently, because a desired version 4.0 would be patched to 4.2
    // (or expressed differently: zero is a valid value for gl_minor_version
    // and can't be used to indicate 'default')
    if (0 == res.gl_major_version) {
        res.gl_major_version = 3;
        res.gl_minor_version = 2;
    }
    res.html5_canvas_name = _sapp_def(res.html5_canvas_name, "canvas");
    res.clipboard_size = _sapp_def(res.clipboard_size, 8192);
    res.max_dropped_files = _sapp_def(res.max_dropped_files, 1);
    res.max_dropped_file_path_length = _sapp_def(res.max_dropped_file_path_length, 2048);
    res.window_title = _sapp_def(res.window_title, "sokol_app");
    return res;
}

_SOKOL_PRIVATE void _sapp_init_state(const sapp_desc* desc) {
    SOKOL_ASSERT(desc);
    SOKOL_ASSERT(desc->width >= 0);
    SOKOL_ASSERT(desc->height >= 0);
    SOKOL_ASSERT(desc->sample_count >= 0);
    SOKOL_ASSERT(desc->swap_interval >= 0);
    SOKOL_ASSERT(desc->clipboard_size >= 0);
    SOKOL_ASSERT(desc->max_dropped_files >= 0);
    SOKOL_ASSERT(desc->max_dropped_file_path_length >= 0);
    _SAPP_CLEAR_ARC_STRUCT(_sapp_t, _sapp);
    _sapp.desc = _sapp_desc_defaults(desc);
    _sapp.first_frame = true;
    // NOTE: _sapp.desc.width/height may be 0! Platform backends need to deal with this
    _sapp.window_width = _sapp.desc.width;
    _sapp.window_height = _sapp.desc.height;
    _sapp.framebuffer_width = _sapp.window_width;
    _sapp.framebuffer_height = _sapp.window_height;
    _sapp.sample_count = _sapp.desc.sample_count;
    _sapp.swap_interval = _sapp.desc.swap_interval;
    _sapp.html5_canvas_selector[0] = '#';
    _sapp_strcpy(_sapp.desc.html5_canvas_name, &_sapp.html5_canvas_selector[1], sizeof(_sapp.html5_canvas_selector) - 1);
    _sapp.desc.html5_canvas_name = &_sapp.html5_canvas_selector[1];
    _sapp.html5_ask_leave_site = _sapp.desc.html5_ask_leave_site;
    _sapp.clipboard.enabled = _sapp.desc.enable_clipboard;
    if (_sapp.clipboard.enabled) {
        _sapp.clipboard.buf_size = _sapp.desc.clipboard_size;
        _sapp.clipboard.buffer = (char*) _sapp_malloc_clear((size_t)_sapp.clipboard.buf_size);
    }
    _sapp.drop.enabled = _sapp.desc.enable_dragndrop;
    if (_sapp.drop.enabled) {
        _sapp.drop.max_files = _sapp.desc.max_dropped_files;
        _sapp.drop.max_path_length = _sapp.desc.max_dropped_file_path_length;
        _sapp.drop.buf_size = _sapp.drop.max_files * _sapp.drop.max_path_length;
        _sapp.drop.buffer = (char*) _sapp_malloc_clear((size_t)_sapp.drop.buf_size);
    }
    _sapp_strcpy(_sapp.desc.window_title, _sapp.window_title, sizeof(_sapp.window_title));
    _sapp.desc.window_title = _sapp.window_title;
    _sapp.dpi_scale = 1.0f;
    _sapp.fullscreen = _sapp.desc.fullscreen;
    _sapp.mouse.shown = true;
    _sapp_timing_init(&_sapp.timing);
}

_SOKOL_PRIVATE void _sapp_discard_state(void) {
    if (_sapp.clipboard.enabled) {
        SOKOL_ASSERT(_sapp.clipboard.buffer);
        _sapp_free((void*)_sapp.clipboard.buffer);
    }
    if (_sapp.drop.enabled) {
        SOKOL_ASSERT(_sapp.drop.buffer);
        _sapp_free((void*)_sapp.drop.buffer);
    }
    if (_sapp.default_icon_pixels) {
        _sapp_free((void*)_sapp.default_icon_pixels);
    }
    _SAPP_CLEAR_ARC_STRUCT(_sapp_t, _sapp);
}

_SOKOL_PRIVATE void _sapp_init_event(sapp_event_type type) {
    _sapp_clear(&_sapp.event, sizeof(_sapp.event));
    _sapp.event.type = type;
    _sapp.event.frame_count = _sapp.frame_count;
    _sapp.event.mouse_button = SAPP_MOUSEBUTTON_INVALID;
    _sapp.event.window_width = _sapp.window_width;
    _sapp.event.window_height = _sapp.window_height;
    _sapp.event.framebuffer_width = _sapp.framebuffer_width;
    _sapp.event.framebuffer_height = _sapp.framebuffer_height;
    _sapp.event.mouse_x = _sapp.mouse.x;
    _sapp.event.mouse_y = _sapp.mouse.y;
    _sapp.event.mouse_dx = _sapp.mouse.dx;
    _sapp.event.mouse_dy = _sapp.mouse.dy;
}

_SOKOL_PRIVATE bool _sapp_events_enabled(void) {
    /* only send events when an event callback is set, and the init function was called */
    return (_sapp.desc.event_cb || _sapp.desc.event_userdata_cb) && _sapp.init_called;
}

_SOKOL_PRIVATE sapp_keycode _sapp_translate_key(int scan_code) {
    if ((scan_code >= 0) && (scan_code < SAPP_MAX_KEYCODES)) {
        return _sapp.keycodes[scan_code];
    }
    else {
        return SAPP_KEYCODE_INVALID;
    }
}

_SOKOL_PRIVATE void _sapp_clear_drop_buffer(void) {
    if (_sapp.drop.enabled) {
        SOKOL_ASSERT(_sapp.drop.buffer);
        _sapp_clear(_sapp.drop.buffer, (size_t)_sapp.drop.buf_size);
    }
}

_SOKOL_PRIVATE void _sapp_frame(void) {
    if (_sapp.first_frame) {
        _sapp.first_frame = false;
        _sapp_call_init();
    }
    _sapp_call_frame();
    _sapp.frame_count++;
}

_SOKOL_PRIVATE bool _sapp_image_validate(const sapp_image_desc* desc) {
    SOKOL_ASSERT(desc->width > 0);
    SOKOL_ASSERT(desc->height > 0);
    SOKOL_ASSERT(desc->pixels.ptr != 0);
    SOKOL_ASSERT(desc->pixels.size > 0);
    const size_t wh_size = (size_t)(desc->width * desc->height) * sizeof(uint32_t);
    if (wh_size != desc->pixels.size) {
        _SAPP_ERROR(IMAGE_DATA_SIZE_MISMATCH);
        return false;
    }
    return true;
}

_SOKOL_PRIVATE int _sapp_image_bestmatch(const sapp_image_desc image_descs[], int num_images, int width, int height) {
    int least_diff = 0x7FFFFFFF;
    int least_index = 0;
    for (int i = 0; i < num_images; i++) {
        int diff = (image_descs[i].width * image_descs[i].height) - (width * height);
        if (diff < 0) {
            diff = -diff;
        }
        if (diff < least_diff) {
            least_diff = diff;
            least_index = i;
        }
    }
    return least_index;
}

_SOKOL_PRIVATE int _sapp_icon_num_images(const sapp_icon_desc* desc) {
    int index = 0;
    for (; index < SAPP_MAX_ICONIMAGES; index++) {
        if (0 == desc->images[index].pixels.ptr) {
            break;
        }
    }
    return index;
}

_SOKOL_PRIVATE bool _sapp_validate_icon_desc(const sapp_icon_desc* desc, int num_images) {
    SOKOL_ASSERT(num_images <= SAPP_MAX_ICONIMAGES);
    for (int i = 0; i < num_images; i++) {
        const sapp_image_desc* img_desc = &desc->images[i];
        if (!_sapp_image_validate(img_desc)) {
            return false;
        }
    }
    return true;
}

_SOKOL_PRIVATE void _sapp_setup_default_icon(void) {
    SOKOL_ASSERT(0 == _sapp.default_icon_pixels);

    const int num_icons = 3;
    const int icon_sizes[3] = { 16, 32, 64 };   // must be multiple of 8!

    // allocate a pixel buffer for all icon pixels
    int all_num_pixels = 0;
    for (int i = 0; i < num_icons; i++) {
        all_num_pixels += icon_sizes[i] * icon_sizes[i];
    }
    _sapp.default_icon_pixels = (uint32_t*) _sapp_malloc_clear((size_t)all_num_pixels * sizeof(uint32_t));

    // initialize default_icon_desc struct
    uint32_t* dst = _sapp.default_icon_pixels;
    const uint32_t* dst_end = dst + all_num_pixels;
    (void)dst_end; // silence unused warning in release mode
    for (int i = 0; i < num_icons; i++) {
        const int dim = (int) icon_sizes[i];
        const int num_pixels = dim * dim;
        sapp_image_desc* img_desc = &_sapp.default_icon_desc.images[i];
        img_desc->width = dim;
        img_desc->height = dim;
        img_desc->pixels.ptr = dst;
        img_desc->pixels.size = (size_t)num_pixels * sizeof(uint32_t);
        dst += num_pixels;
    }
    SOKOL_ASSERT(dst == dst_end);

    // Amstrad CPC font 'S'
    const uint8_t tile[8] = {
        0x3C,
        0x66,
        0x60,
        0x3C,
        0x06,
        0x66,
        0x3C,
        0x00,
    };
    // rainbow colors
    const uint32_t colors[8] = {
        0xFF4370FF,
        0xFF26A7FF,
        0xFF58EEFF,
        0xFF57E1D4,
        0xFF65CC9C,
        0xFF6ABB66,
        0xFFF5A542,
        0xFFC2577E,
    };
    dst = _sapp.default_icon_pixels;
    const uint32_t blank = 0x00FFFFFF;
    const uint32_t shadow = 0xFF000000;
    for (int i = 0; i < num_icons; i++) {
        const int dim = icon_sizes[i];
        SOKOL_ASSERT((dim % 8) == 0);
        const int scale = dim / 8;
        for (int ty = 0, y = 0; ty < 8; ty++) {
            const uint32_t color = colors[ty];
            for (int sy = 0; sy < scale; sy++, y++) {
                uint8_t bits = tile[ty];
                for (int tx = 0, x = 0; tx < 8; tx++, bits<<=1) {
                    uint32_t pixel = (0 == (bits & 0x80)) ? blank : color;
                    for (int sx = 0; sx < scale; sx++, x++) {
                        SOKOL_ASSERT(dst < dst_end);
                        *dst++ = pixel;
                    }
                }
            }
        }
    }
    SOKOL_ASSERT(dst == dst_end);

    // right shadow
    dst = _sapp.default_icon_pixels;
    for (int i = 0; i < num_icons; i++) {
        const int dim = icon_sizes[i];
        for (int y = 0; y < dim; y++) {
            uint32_t prev_color = blank;
            for (int x = 0; x < dim; x++) {
                const int dst_index = y * dim + x;
                const uint32_t cur_color = dst[dst_index];
                if ((cur_color == blank) && (prev_color != blank)) {
                    dst[dst_index] = shadow;
                }
                prev_color = cur_color;
            }
        }
        dst += dim * dim;
    }
    SOKOL_ASSERT(dst == dst_end);

    // bottom shadow
    dst = _sapp.default_icon_pixels;
    for (int i = 0; i < num_icons; i++) {
        const int dim = icon_sizes[i];
        for (int x = 0; x < dim; x++) {
            uint32_t prev_color = blank;
            for (int y = 0; y < dim; y++) {
                const int dst_index = y * dim + x;
                const uint32_t cur_color = dst[dst_index];
                if ((cur_color == blank) && (prev_color != blank)) {
                    dst[dst_index] = shadow;
                }
                prev_color = cur_color;
            }
        }
        dst += dim * dim;
    }
    SOKOL_ASSERT(dst == dst_end);
}

//  █████  ██████  ██████  ██      ███████
// ██   ██ ██   ██ ██   ██ ██      ██
// ███████ ██████  ██████  ██      █████
// ██   ██ ██      ██      ██      ██
// ██   ██ ██      ██      ███████ ███████
//
// >>apple
#if defined(_SAPP_APPLE)

#if __has_feature(objc_arc)
#define _SAPP_OBJC_RELEASE(obj) { obj = nil; }
#else
#define _SAPP_OBJC_RELEASE(obj) { [obj release]; obj = nil; }
#endif

// ███    ███  █████   ██████  ██████  ███████
// ████  ████ ██   ██ ██      ██    ██ ██
// ██ ████ ██ ███████ ██      ██    ██ ███████
// ██  ██  ██ ██   ██ ██      ██    ██      ██
// ██      ██ ██   ██  ██████  ██████  ███████
//
// >>macos
#if defined(_SAPP_MACOS)

_SOKOL_PRIVATE void _sapp_macos_init_keytable(void) {
    _sapp.keycodes[0x1D] = SAPP_KEYCODE_0;
    _sapp.keycodes[0x12] = SAPP_KEYCODE_1;
    _sapp.keycodes[0x13] = SAPP_KEYCODE_2;
    _sapp.keycodes[0x14] = SAPP_KEYCODE_3;
    _sapp.keycodes[0x15] = SAPP_KEYCODE_4;
    _sapp.keycodes[0x17] = SAPP_KEYCODE_5;
    _sapp.keycodes[0x16] = SAPP_KEYCODE_6;
    _sapp.keycodes[0x1A] = SAPP_KEYCODE_7;
    _sapp.keycodes[0x1C] = SAPP_KEYCODE_8;
    _sapp.keycodes[0x19] = SAPP_KEYCODE_9;
    _sapp.keycodes[0x00] = SAPP_KEYCODE_A;
    _sapp.keycodes[0x0B] = SAPP_KEYCODE_B;
    _sapp.keycodes[0x08] = SAPP_KEYCODE_C;
    _sapp.keycodes[0x02] = SAPP_KEYCODE_D;
    _sapp.keycodes[0x0E] = SAPP_KEYCODE_E;
    _sapp.keycodes[0x03] = SAPP_KEYCODE_F;
    _sapp.keycodes[0x05] = SAPP_KEYCODE_G;
    _sapp.keycodes[0x04] = SAPP_KEYCODE_H;
    _sapp.keycodes[0x22] = SAPP_KEYCODE_I;
    _sapp.keycodes[0x26] = SAPP_KEYCODE_J;
    _sapp.keycodes[0x28] = SAPP_KEYCODE_K;
    _sapp.keycodes[0x25] = SAPP_KEYCODE_L;
    _sapp.keycodes[0x2E] = SAPP_KEYCODE_M;
    _sapp.keycodes[0x2D] = SAPP_KEYCODE_N;
    _sapp.keycodes[0x1F] = SAPP_KEYCODE_O;
    _sapp.keycodes[0x23] = SAPP_KEYCODE_P;
    _sapp.keycodes[0x0C] = SAPP_KEYCODE_Q;
    _sapp.keycodes[0x0F] = SAPP_KEYCODE_R;
    _sapp.keycodes[0x01] = SAPP_KEYCODE_S;
    _sapp.keycodes[0x11] = SAPP_KEYCODE_T;
    _sapp.keycodes[0x20] = SAPP_KEYCODE_U;
    _sapp.keycodes[0x09] = SAPP_KEYCODE_V;
    _sapp.keycodes[0x0D] = SAPP_KEYCODE_W;
    _sapp.keycodes[0x07] = SAPP_KEYCODE_X;
    _sapp.keycodes[0x10] = SAPP_KEYCODE_Y;
    _sapp.keycodes[0x06] = SAPP_KEYCODE_Z;
    _sapp.keycodes[0x27] = SAPP_KEYCODE_APOSTROPHE;
    _sapp.keycodes[0x2A] = SAPP_KEYCODE_BACKSLASH;
    _sapp.keycodes[0x2B] = SAPP_KEYCODE_COMMA;
    _sapp.keycodes[0x18] = SAPP_KEYCODE_EQUAL;
    _sapp.keycodes[0x32] = SAPP_KEYCODE_GRAVE_ACCENT;
    _sapp.keycodes[0x21] = SAPP_KEYCODE_LEFT_BRACKET;
    _sapp.keycodes[0x1B] = SAPP_KEYCODE_MINUS;
    _sapp.keycodes[0x2F] = SAPP_KEYCODE_PERIOD;
    _sapp.keycodes[0x1E] = SAPP_KEYCODE_RIGHT_BRACKET;
    _sapp.keycodes[0x29] = SAPP_KEYCODE_SEMICOLON;
    _sapp.keycodes[0x2C] = SAPP_KEYCODE_SLASH;
    _sapp.keycodes[0x0A] = SAPP_KEYCODE_WORLD_1;
    _sapp.keycodes[0x33] = SAPP_KEYCODE_BACKSPACE;
    _sapp.keycodes[0x39] = SAPP_KEYCODE_CAPS_LOCK;
    _sapp.keycodes[0x75] = SAPP_KEYCODE_DELETE;
    _sapp.keycodes[0x7D] = SAPP_KEYCODE_DOWN;
    _sapp.keycodes[0x77] = SAPP_KEYCODE_END;
    _sapp.keycodes[0x24] = SAPP_KEYCODE_ENTER;
    _sapp.keycodes[0x35] = SAPP_KEYCODE_ESCAPE;
    _sapp.keycodes[0x7A] = SAPP_KEYCODE_F1;
    _sapp.keycodes[0x78] = SAPP_KEYCODE_F2;
    _sapp.keycodes[0x63] = SAPP_KEYCODE_F3;
    _sapp.keycodes[0x76] = SAPP_KEYCODE_F4;
    _sapp.keycodes[0x60] = SAPP_KEYCODE_F5;
    _sapp.keycodes[0x61] = SAPP_KEYCODE_F6;
    _sapp.keycodes[0x62] = SAPP_KEYCODE_F7;
    _sapp.keycodes[0x64] = SAPP_KEYCODE_F8;
    _sapp.keycodes[0x65] = SAPP_KEYCODE_F9;
    _sapp.keycodes[0x6D] = SAPP_KEYCODE_F10;
    _sapp.keycodes[0x67] = SAPP_KEYCODE_F11;
    _sapp.keycodes[0x6F] = SAPP_KEYCODE_F12;
    _sapp.keycodes[0x69] = SAPP_KEYCODE_F13;
    _sapp.keycodes[0x6B] = SAPP_KEYCODE_F14;
    _sapp.keycodes[0x71] = SAPP_KEYCODE_F15;
    _sapp.keycodes[0x6A] = SAPP_KEYCODE_F16;
    _sapp.keycodes[0x40] = SAPP_KEYCODE_F17;
    _sapp.keycodes[0x4F] = SAPP_KEYCODE_F18;
    _sapp.keycodes[0x50] = SAPP_KEYCODE_F19;
    _sapp.keycodes[0x5A] = SAPP_KEYCODE_F20;
    _sapp.keycodes[0x73] = SAPP_KEYCODE_HOME;
    _sapp.keycodes[0x72] = SAPP_KEYCODE_INSERT;
    _sapp.keycodes[0x7B] = SAPP_KEYCODE_LEFT;
    _sapp.keycodes[0x3A] = SAPP_KEYCODE_LEFT_ALT;
    _sapp.keycodes[0x3B] = SAPP_KEYCODE_LEFT_CONTROL;
    _sapp.keycodes[0x38] = SAPP_KEYCODE_LEFT_SHIFT;
    _sapp.keycodes[0x37] = SAPP_KEYCODE_LEFT_SUPER;
    _sapp.keycodes[0x6E] = SAPP_KEYCODE_MENU;
    _sapp.keycodes[0x47] = SAPP_KEYCODE_NUM_LOCK;
    _sapp.keycodes[0x79] = SAPP_KEYCODE_PAGE_DOWN;
    _sapp.keycodes[0x74] = SAPP_KEYCODE_PAGE_UP;
    _sapp.keycodes[0x7C] = SAPP_KEYCODE_RIGHT;
    _sapp.keycodes[0x3D] = SAPP_KEYCODE_RIGHT_ALT;
    _sapp.keycodes[0x3E] = SAPP_KEYCODE_RIGHT_CONTROL;
    _sapp.keycodes[0x3C] = SAPP_KEYCODE_RIGHT_SHIFT;
    _sapp.keycodes[0x36] = SAPP_KEYCODE_RIGHT_SUPER;
    _sapp.keycodes[0x31] = SAPP_KEYCODE_SPACE;
    _sapp.keycodes[0x30] = SAPP_KEYCODE_TAB;
    _sapp.keycodes[0x7E] = SAPP_KEYCODE_UP;
    _sapp.keycodes[0x52] = SAPP_KEYCODE_KP_0;
    _sapp.keycodes[0x53] = SAPP_KEYCODE_KP_1;
    _sapp.keycodes[0x54] = SAPP_KEYCODE_KP_2;
    _sapp.keycodes[0x55] = SAPP_KEYCODE_KP_3;
    _sapp.keycodes[0x56] = SAPP_KEYCODE_KP_4;
    _sapp.keycodes[0x57] = SAPP_KEYCODE_KP_5;
    _sapp.keycodes[0x58] = SAPP_KEYCODE_KP_6;
    _sapp.keycodes[0x59] = SAPP_KEYCODE_KP_7;
    _sapp.keycodes[0x5B] = SAPP_KEYCODE_KP_8;
    _sapp.keycodes[0x5C] = SAPP_KEYCODE_KP_9;
    _sapp.keycodes[0x45] = SAPP_KEYCODE_KP_ADD;
    _sapp.keycodes[0x41] = SAPP_KEYCODE_KP_DECIMAL;
    _sapp.keycodes[0x4B] = SAPP_KEYCODE_KP_DIVIDE;
    _sapp.keycodes[0x4C] = SAPP_KEYCODE_KP_ENTER;
    _sapp.keycodes[0x51] = SAPP_KEYCODE_KP_EQUAL;
    _sapp.keycodes[0x43] = SAPP_KEYCODE_KP_MULTIPLY;
    _sapp.keycodes[0x4E] = SAPP_KEYCODE_KP_SUBTRACT;
}

_SOKOL_PRIVATE void _sapp_macos_discard_state(void) {
    // NOTE: it's safe to call [release] on a nil object
    if (_sapp.macos.keyup_monitor != nil) {
        [NSEvent removeMonitor:_sapp.macos.keyup_monitor];
        // NOTE: removeMonitor also releases the object
        _sapp.macos.keyup_monitor = nil;
    }
    _SAPP_OBJC_RELEASE(_sapp.macos.tracking_area);
    _SAPP_OBJC_RELEASE(_sapp.macos.app_dlg);
    _SAPP_OBJC_RELEASE(_sapp.macos.win_dlg);
    _SAPP_OBJC_RELEASE(_sapp.macos.view);
    #if defined(SOKOL_METAL)
        _SAPP_OBJC_RELEASE(_sapp.macos.mtl_device);
    #endif
    _SAPP_OBJC_RELEASE(_sapp.macos.window);
}

// undocumented methods for creating cursors (see GLFW 3.4 and imgui_impl_osx.mm)
@interface NSCursor()
+ (id)_windowResizeNorthWestSouthEastCursor;
+ (id)_windowResizeNorthEastSouthWestCursor;
+ (id)_windowResizeNorthSouthCursor;
+ (id)_windowResizeEastWestCursor;
@end

_SOKOL_PRIVATE void _sapp_macos_init_cursors(void) {
    _sapp.macos.cursors[SAPP_MOUSECURSOR_DEFAULT] = nil; // not a bug
    _sapp.macos.cursors[SAPP_MOUSECURSOR_ARROW] = [NSCursor arrowCursor];
    _sapp.macos.cursors[SAPP_MOUSECURSOR_IBEAM] = [NSCursor IBeamCursor];
    _sapp.macos.cursors[SAPP_MOUSECURSOR_CROSSHAIR] = [NSCursor crosshairCursor];
    _sapp.macos.cursors[SAPP_MOUSECURSOR_POINTING_HAND] = [NSCursor pointingHandCursor];
    _sapp.macos.cursors[SAPP_MOUSECURSOR_RESIZE_EW] = [NSCursor respondsToSelector:@selector(_windowResizeEastWestCursor)] ? [NSCursor _windowResizeEastWestCursor] : [NSCursor resizeLeftRightCursor];
    _sapp.macos.cursors[SAPP_MOUSECURSOR_RESIZE_NS] = [NSCursor respondsToSelector:@selector(_windowResizeNorthSouthCursor)] ? [NSCursor _windowResizeNorthSouthCursor] : [NSCursor resizeUpDownCursor];
    _sapp.macos.cursors[SAPP_MOUSECURSOR_RESIZE_NWSE] = [NSCursor respondsToSelector:@selector(_windowResizeNorthWestSouthEastCursor)] ? [NSCursor _windowResizeNorthWestSouthEastCursor] : [NSCursor closedHandCursor];
    _sapp.macos.cursors[SAPP_MOUSECURSOR_RESIZE_NESW] = [NSCursor respondsToSelector:@selector(_windowResizeNorthEastSouthWestCursor)] ? [NSCursor _windowResizeNorthEastSouthWestCursor] : [NSCursor closedHandCursor];
    _sapp.macos.cursors[SAPP_MOUSECURSOR_RESIZE_ALL] = [NSCursor closedHandCursor];
    _sapp.macos.cursors[SAPP_MOUSECURSOR_NOT_ALLOWED] = [NSCursor operationNotAllowedCursor];
}

_SOKOL_PRIVATE void _sapp_macos_run(const sapp_desc* desc) {
    _sapp_init_state(desc);
    _sapp_macos_init_keytable();
    [NSApplication sharedApplication];

    // set the application dock icon as early as possible, otherwise
    // the dummy icon will be visible for a short time
    sapp_set_icon(&_sapp.desc.icon);
    _sapp.macos.app_dlg = [[_sapp_macos_app_delegate alloc] init];
    NSApp.delegate = _sapp.macos.app_dlg;

    // workaround for "no key-up sent while Cmd is pressed" taken from GLFW:
    NSEvent* (^keyup_monitor)(NSEvent*) = ^NSEvent* (NSEvent* event) {
        if ([event modifierFlags] & NSEventModifierFlagCommand) {
            [[NSApp keyWindow] sendEvent:event];
        }
        return event;
    };
    _sapp.macos.keyup_monitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyUp handler:keyup_monitor];

    [NSApp run];
    // NOTE: [NSApp run] never returns, instead cleanup code
    // must be put into applicationWillTerminate
}

/* MacOS entry function */
#if !defined(SOKOL_NO_ENTRY)
int main(int argc, char* argv[]) {
    sapp_desc desc = sokol_main(argc, argv);
    _sapp_macos_run(&desc);
    return 0;
}
#endif /* SOKOL_NO_ENTRY */

_SOKOL_PRIVATE uint32_t _sapp_macos_mods(NSEvent* ev) {
    const NSEventModifierFlags f = (ev == nil) ? NSEvent.modifierFlags : ev.modifierFlags;
    const NSUInteger b = NSEvent.pressedMouseButtons;
    uint32_t m = 0;
    if (f & NSEventModifierFlagShift) {
        m |= SAPP_MODIFIER_SHIFT;
    }
    if (f & NSEventModifierFlagControl) {
        m |= SAPP_MODIFIER_CTRL;
    }
    if (f & NSEventModifierFlagOption) {
        m |= SAPP_MODIFIER_ALT;
    }
    if (f & NSEventModifierFlagCommand) {
        m |= SAPP_MODIFIER_SUPER;
    }
    if (0 != (b & (1<<0))) {
        m |= SAPP_MODIFIER_LMB;
    }
    if (0 != (b & (1<<1))) {
        m |= SAPP_MODIFIER_RMB;
    }
    if (0 != (b & (1<<2))) {
        m |= SAPP_MODIFIER_MMB;
    }
    return m;
}

_SOKOL_PRIVATE void _sapp_macos_mouse_event(sapp_event_type type, sapp_mousebutton btn, uint32_t mod) {
    if (_sapp_events_enabled()) {
        _sapp_init_event(type);
        _sapp.event.mouse_button = btn;
        _sapp.event.modifiers = mod;
        _sapp_call_event(&_sapp.event);
    }
}

_SOKOL_PRIVATE void _sapp_macos_key_event(sapp_event_type type, sapp_keycode key, bool repeat, uint32_t mod) {
    if (_sapp_events_enabled()) {
        _sapp_init_event(type);
        _sapp.event.key_code = key;
        _sapp.event.key_repeat = repeat;
        _sapp.event.modifiers = mod;
        _sapp_call_event(&_sapp.event);
    }
}

_SOKOL_PRIVATE void _sapp_macos_app_event(sapp_event_type type) {
    if (_sapp_events_enabled()) {
        _sapp_init_event(type);
        _sapp_call_event(&_sapp.event);
    }
}

/* NOTE: unlike the iOS version of this function, the macOS version
    can dynamically update the DPI scaling factor when a window is moved
    between HighDPI / LowDPI screens.
*/
_SOKOL_PRIVATE void _sapp_macos_update_dimensions(void) {
    if (_sapp.desc.high_dpi) {
        _sapp.dpi_scale = [_sapp.macos.window screen].backingScaleFactor;
    }
    else {
        _sapp.dpi_scale = 1.0f;
    }
    const NSRect bounds = [_sapp.macos.view bounds];
    _sapp.window_width = (int)roundf(bounds.size.width);
    _sapp.window_height = (int)roundf(bounds.size.height);
    #if defined(SOKOL_METAL)
        _sapp.framebuffer_width = (int)roundf(bounds.size.width * _sapp.dpi_scale);
        _sapp.framebuffer_height = (int)roundf(bounds.size.height * _sapp.dpi_scale);
        const CGSize fb_size = _sapp.macos.view.drawableSize;
        const int cur_fb_width = (int)roundf(fb_size.width);
        const int cur_fb_height = (int)roundf(fb_size.height);
        const bool dim_changed = (_sapp.framebuffer_width != cur_fb_width) ||
                                 (_sapp.framebuffer_height != cur_fb_height);
    #elif defined(SOKOL_GLCORE33)
        const int cur_fb_width = (int)roundf(bounds.size.width * _sapp.dpi_scale);
        const int cur_fb_height = (int)roundf(bounds.size.height * _sapp.dpi_scale);
        const bool dim_changed = (_sapp.framebuffer_width != cur_fb_width) ||
                                 (_sapp.framebuffer_height != cur_fb_height);
        _sapp.framebuffer_width = cur_fb_width;
        _sapp.framebuffer_height = cur_fb_height;
    #endif
    if (_sapp.framebuffer_width == 0) {
        _sapp.framebuffer_width = 1;
    }
    if (_sapp.framebuffer_height == 0) {
        _sapp.framebuffer_height = 1;
    }
    if (_sapp.window_width == 0) {
        _sapp.window_width = 1;
    }
    if (_sapp.window_height == 0) {
        _sapp.window_height = 1;
    }
    if (dim_changed) {
        #if defined(SOKOL_METAL)
            CGSize drawable_size = { (CGFloat) _sapp.framebuffer_width, (CGFloat) _sapp.framebuffer_height };
            _sapp.macos.view.drawableSize = drawable_size;
        #else
            // nothing to do for GL?
        #endif
        if (!_sapp.first_frame) {
            _sapp_macos_app_event(SAPP_EVENTTYPE_RESIZED);
        }
    }
}

_SOKOL_PRIVATE void _sapp_macos_toggle_fullscreen(void) {
    /* NOTE: the _sapp.fullscreen flag is also notified by the
       windowDidEnterFullscreen / windowDidExitFullscreen
       event handlers
    */
    _sapp.fullscreen = !_sapp.fullscreen;
    [_sapp.macos.window toggleFullScreen:nil];
}

_SOKOL_PRIVATE void _sapp_macos_set_clipboard_string(const char* str) {
    @autoreleasepool {
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        [pasteboard declareTypes:@[NSPasteboardTypeString] owner:nil];
        [pasteboard setString:@(str) forType:NSPasteboardTypeString];
    }
}

_SOKOL_PRIVATE const char* _sapp_macos_get_clipboard_string(void) {
    SOKOL_ASSERT(_sapp.clipboard.buffer);
    @autoreleasepool {
        _sapp.clipboard.buffer[0] = 0;
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        if (![[pasteboard types] containsObject:NSPasteboardTypeString]) {
            return _sapp.clipboard.buffer;
        }
        NSString* str = [pasteboard stringForType:NSPasteboardTypeString];
        if (!str) {
            return _sapp.clipboard.buffer;
        }
        _sapp_strcpy([str UTF8String], _sapp.clipboard.buffer, _sapp.clipboard.buf_size);
    }
    return _sapp.clipboard.buffer;
}

_SOKOL_PRIVATE void _sapp_macos_update_window_title(void) {
    [_sapp.macos.window setTitle: [NSString stringWithUTF8String:_sapp.window_title]];
}

_SOKOL_PRIVATE void _sapp_macos_mouse_update_from_nspoint(NSPoint mouse_pos, bool clear_dxdy) {
    if (!_sapp.mouse.locked) {
        float new_x = mouse_pos.x * _sapp.dpi_scale;
        float new_y = _sapp.framebuffer_height - (mouse_pos.y * _sapp.dpi_scale) - 1;
        if (clear_dxdy) {
            _sapp.mouse.dx = 0.0f;
            _sapp.mouse.dy = 0.0f;
        }
        else if (_sapp.mouse.pos_valid) {
            // don't update dx/dy in the very first update
            _sapp.mouse.dx = new_x - _sapp.mouse.x;
            _sapp.mouse.dy = new_y - _sapp.mouse.y;
        }
        _sapp.mouse.x = new_x;
        _sapp.mouse.y = new_y;
        _sapp.mouse.pos_valid = true;
    }
}

_SOKOL_PRIVATE void _sapp_macos_mouse_update_from_nsevent(NSEvent* event, bool clear_dxdy) {
    _sapp_macos_mouse_update_from_nspoint(event.locationInWindow, clear_dxdy);
}

_SOKOL_PRIVATE void _sapp_macos_show_mouse(bool visible) {
    /* NOTE: this function is only called when the mouse visibility actually changes */
    if (visible) {
        CGDisplayShowCursor(kCGDirectMainDisplay);
    }
    else {
        CGDisplayHideCursor(kCGDirectMainDisplay);
    }
}

_SOKOL_PRIVATE void _sapp_macos_lock_mouse(bool lock) {
    if (lock == _sapp.mouse.locked) {
        return;
    }
    _sapp.mouse.dx = 0.0f;
    _sapp.mouse.dy = 0.0f;
    _sapp.mouse.locked = lock;
    /*
        NOTE that this code doesn't warp the mouse cursor to the window
        center as everybody else does it. This lead to a spike in the
        *second* mouse-moved event after the warp happened. The
        mouse centering doesn't seem to be required (mouse-moved events
        are reported correctly even when the cursor is at an edge of the screen).

        NOTE also that the hide/show of the mouse cursor should properly
        stack with calls to sapp_show_mouse()
    */
    if (_sapp.mouse.locked) {
        CGAssociateMouseAndMouseCursorPosition(NO);
        [NSCursor hide];
    }
    else {
        [NSCursor unhide];
        CGAssociateMouseAndMouseCursorPosition(YES);
    }
}

_SOKOL_PRIVATE void _sapp_macos_update_cursor(sapp_mouse_cursor cursor, bool shown) {
    // show/hide cursor only if visibility status has changed (required because show/hide stacks)
    if (shown != _sapp.mouse.shown) {
        if (shown) {
            [NSCursor unhide];
        }
        else {
            [NSCursor hide];
        }
    }
    // update cursor type
    SOKOL_ASSERT((cursor >= 0) && (cursor < _SAPP_MOUSECURSOR_NUM));
    if (_sapp.macos.cursors[cursor]) {
        [_sapp.macos.cursors[cursor] set];
    }
    else {
        [[NSCursor arrowCursor] set];
    }
}

_SOKOL_PRIVATE void _sapp_macos_set_icon(const sapp_icon_desc* icon_desc, int num_images) {
    NSDockTile* dock_tile = NSApp.dockTile;
    const int wanted_width = (int) dock_tile.size.width;
    const int wanted_height = (int) dock_tile.size.height;
    const int img_index = _sapp_image_bestmatch(icon_desc->images, num_images, wanted_width, wanted_height);
    const sapp_image_desc* img_desc = &icon_desc->images[img_index];

    CGColorSpaceRef cg_color_space = CGColorSpaceCreateDeviceRGB();
    CFDataRef cf_data = CFDataCreate(kCFAllocatorDefault, (const UInt8*)img_desc->pixels.ptr, (CFIndex)img_desc->pixels.size);
    CGDataProviderRef cg_data_provider = CGDataProviderCreateWithCFData(cf_data);
    CGImageRef cg_img = CGImageCreate(
        (size_t)img_desc->width,    // width
        (size_t)img_desc->height,   // height
        8,                          // bitsPerComponent
        32,                         // bitsPerPixel
        (size_t)img_desc->width * 4,// bytesPerRow
        cg_color_space,             // space
        kCGImageAlphaLast | kCGImageByteOrderDefault,  // bitmapInfo
        cg_data_provider,           // provider
        NULL,                       // decode
        false,                      // shouldInterpolate
        kCGRenderingIntentDefault);
    CFRelease(cf_data);
    CGDataProviderRelease(cg_data_provider);
    CGColorSpaceRelease(cg_color_space);

    NSImage* ns_image = [[NSImage alloc] initWithCGImage:cg_img size:dock_tile.size];
    dock_tile.contentView = [NSImageView imageViewWithImage:ns_image];
    [dock_tile display];
    _SAPP_OBJC_RELEASE(ns_image);
    CGImageRelease(cg_img);
}

_SOKOL_PRIVATE void _sapp_macos_frame(void) {
    _sapp_frame();
    if (_sapp.quit_requested || _sapp.quit_ordered) {
        [_sapp.macos.window performClose:nil];
    }
}

@implementation _sapp_macos_app_delegate
- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    _SOKOL_UNUSED(aNotification);
    _sapp_macos_init_cursors();
    if ((_sapp.window_width == 0) || (_sapp.window_height == 0)) {
        // use 4/5 of screen size as default size
        NSRect screen_rect = NSScreen.mainScreen.frame;
        if (_sapp.window_width == 0) {
            _sapp.window_width = (int)roundf((screen_rect.size.width * 4.0f) / 5.0f);
        }
        if (_sapp.window_height == 0) {
            _sapp.window_height = (int)roundf((screen_rect.size.height * 4.0f) / 5.0f);
        }
    }
    const NSUInteger style =
        NSWindowStyleMaskTitled |
        NSWindowStyleMaskClosable |
        NSWindowStyleMaskMiniaturizable |
        NSWindowStyleMaskResizable;
    NSRect window_rect = NSMakeRect(0, 0, _sapp.window_width, _sapp.window_height);
    _sapp.macos.window = [[_sapp_macos_window alloc]
        initWithContentRect:window_rect
        styleMask:style
        backing:NSBackingStoreBuffered
        defer:NO];
    _sapp.macos.window.releasedWhenClosed = NO; // this is necessary for proper cleanup in applicationWillTerminate
    _sapp.macos.window.title = [NSString stringWithUTF8String:_sapp.window_title];
    _sapp.macos.window.acceptsMouseMovedEvents = YES;
    _sapp.macos.window.restorable = YES;

    _sapp.macos.win_dlg = [[_sapp_macos_window_delegate alloc] init];
    _sapp.macos.window.delegate = _sapp.macos.win_dlg;
    #if defined(SOKOL_METAL)
        NSInteger max_fps = 60;
        #if (__MAC_OS_X_VERSION_MAX_ALLOWED >= 120000)
        if (@available(macOS 12.0, *)) {
            max_fps = [NSScreen.mainScreen maximumFramesPerSecond];
        }
        #endif
        _sapp.macos.mtl_device = MTLCreateSystemDefaultDevice();
        _sapp.macos.view = [[_sapp_macos_view alloc] init];
        [_sapp.macos.view updateTrackingAreas];
        _sapp.macos.view.preferredFramesPerSecond = max_fps / _sapp.swap_interval;
        _sapp.macos.view.device = _sapp.macos.mtl_device;
        _sapp.macos.view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        _sapp.macos.view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
        _sapp.macos.view.sampleCount = (NSUInteger) _sapp.sample_count;
        _sapp.macos.view.autoResizeDrawable = false;
        _sapp.macos.window.contentView = _sapp.macos.view;
        [_sapp.macos.window makeFirstResponder:_sapp.macos.view];
        _sapp.macos.view.layer.magnificationFilter = kCAFilterNearest;
    #elif defined(SOKOL_GLCORE33)
        NSOpenGLPixelFormatAttribute attrs[32];
        int i = 0;
        attrs[i++] = NSOpenGLPFAAccelerated;
        attrs[i++] = NSOpenGLPFADoubleBuffer;
        attrs[i++] = NSOpenGLPFAOpenGLProfile;
        const int glVersion = _sapp.desc.gl_major_version * 10 + _sapp.desc.gl_minor_version;
        switch(glVersion) {
            case 10: attrs[i++] = NSOpenGLProfileVersionLegacy;  break;
            case 32: attrs[i++] = NSOpenGLProfileVersion3_2Core; break;
            case 41: attrs[i++] = NSOpenGLProfileVersion4_1Core; break;
            default:
                _SAPP_PANIC(MACOS_INVALID_NSOPENGL_PROFILE);
        }
        attrs[i++] = NSOpenGLPFAColorSize; attrs[i++] = 24;
        attrs[i++] = NSOpenGLPFAAlphaSize; attrs[i++] = 8;
        attrs[i++] = NSOpenGLPFADepthSize; attrs[i++] = 24;
        attrs[i++] = NSOpenGLPFAStencilSize; attrs[i++] = 8;
        if (_sapp.sample_count > 1) {
            attrs[i++] = NSOpenGLPFAMultisample;
            attrs[i++] = NSOpenGLPFASampleBuffers; attrs[i++] = 1;
            attrs[i++] = NSOpenGLPFASamples; attrs[i++] = (NSOpenGLPixelFormatAttribute)_sapp.sample_count;
        }
        else {
            attrs[i++] = NSOpenGLPFASampleBuffers; attrs[i++] = 0;
        }
        attrs[i++] = 0;
        NSOpenGLPixelFormat* glpixelformat_obj = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
        SOKOL_ASSERT(glpixelformat_obj != nil);

        _sapp.macos.view = [[_sapp_macos_view alloc]
            initWithFrame:window_rect
            pixelFormat:glpixelformat_obj];
        _SAPP_OBJC_RELEASE(glpixelformat_obj);
        [_sapp.macos.view updateTrackingAreas];
        if (_sapp.desc.high_dpi) {
            [_sapp.macos.view setWantsBestResolutionOpenGLSurface:YES];
        }
        else {
            [_sapp.macos.view setWantsBestResolutionOpenGLSurface:NO];
        }

        _sapp.macos.window.contentView = _sapp.macos.view;
        [_sapp.macos.window makeFirstResponder:_sapp.macos.view];

        NSTimer* timer_obj = [NSTimer timerWithTimeInterval:0.001
            target:_sapp.macos.view
            selector:@selector(timerFired:)
            userInfo:nil
            repeats:YES];
        [[NSRunLoop currentRunLoop] addTimer:timer_obj forMode:NSDefaultRunLoopMode];
        timer_obj = nil;
    #endif
    [_sapp.macos.window center];
    _sapp.valid = true;
    if (_sapp.fullscreen) {
        /* ^^^ on GL, this already toggles a rendered frame, so set the valid flag before */
        [_sapp.macos.window toggleFullScreen:self];
    }
    NSApp.activationPolicy = NSApplicationActivationPolicyRegular;
    [NSApp activateIgnoringOtherApps:YES];
    [_sapp.macos.window makeKeyAndOrderFront:nil];
    _sapp_macos_update_dimensions();
    [NSEvent setMouseCoalescingEnabled:NO];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    _SOKOL_UNUSED(sender);
    return YES;
}

- (void)applicationWillTerminate:(NSNotification*)notification {
    _SOKOL_UNUSED(notification);
    _sapp_call_cleanup();
    _sapp_macos_discard_state();
    _sapp_discard_state();
}
@end

@implementation _sapp_macos_window_delegate
- (BOOL)windowShouldClose:(id)sender {
    _SOKOL_UNUSED(sender);
    /* only give user-code a chance to intervene when sapp_quit() wasn't already called */
    if (!_sapp.quit_ordered) {
        /* if window should be closed and event handling is enabled, give user code
           a chance to intervene via sapp_cancel_quit()
        */
        _sapp.quit_requested = true;
        _sapp_macos_app_event(SAPP_EVENTTYPE_QUIT_REQUESTED);
        /* user code hasn't intervened, quit the app */
        if (_sapp.quit_requested) {
            _sapp.quit_ordered = true;
        }
    }
    if (_sapp.quit_ordered) {
        return YES;
    }
    else {
        return NO;
    }
}

- (void)windowDidResize:(NSNotification*)notification {
    _SOKOL_UNUSED(notification);
    _sapp_macos_update_dimensions();
}

- (void)windowDidChangeScreen:(NSNotification*)notification {
    _SOKOL_UNUSED(notification);
    _sapp_timing_reset(&_sapp.timing);
    _sapp_macos_update_dimensions();
}

- (void)windowDidMiniaturize:(NSNotification*)notification {
    _SOKOL_UNUSED(notification);
    _sapp_macos_app_event(SAPP_EVENTTYPE_ICONIFIED);
}

- (void)windowDidDeminiaturize:(NSNotification*)notification {
    _SOKOL_UNUSED(notification);
    _sapp_macos_app_event(SAPP_EVENTTYPE_RESTORED);
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
    _SOKOL_UNUSED(notification);
    _sapp_macos_app_event(SAPP_EVENTTYPE_FOCUSED);
}

- (void)windowDidResignKey:(NSNotification*)notification {
    _SOKOL_UNUSED(notification);
    _sapp_macos_app_event(SAPP_EVENTTYPE_UNFOCUSED);
}

- (void)windowDidEnterFullScreen:(NSNotification*)notification {
    _SOKOL_UNUSED(notification);
    _sapp.fullscreen = true;
}

- (void)windowDidExitFullScreen:(NSNotification*)notification {
    _SOKOL_UNUSED(notification);
    _sapp.fullscreen = false;
}
@end

@implementation _sapp_macos_window
- (instancetype)initWithContentRect:(NSRect)contentRect
                          styleMask:(NSWindowStyleMask)style
                            backing:(NSBackingStoreType)backingStoreType
                              defer:(BOOL)flag {
    if (self = [super initWithContentRect:contentRect styleMask:style backing:backingStoreType defer:flag]) {
        #if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101300
            [self registerForDraggedTypes:[NSArray arrayWithObject:NSPasteboardTypeFileURL]];
        #endif
    }
    return self;
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
    return NSDragOperationCopy;
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
    return NSDragOperationCopy;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
    #if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101300
    NSPasteboard *pboard = [sender draggingPasteboard];
    if ([pboard.types containsObject:NSPasteboardTypeFileURL]) {
        _sapp_clear_drop_buffer();
        _sapp.drop.num_files = ((int)pboard.pasteboardItems.count > _sapp.drop.max_files) ? _sapp.drop.max_files : (int)pboard.pasteboardItems.count;
        bool drop_failed = false;
        for (int i = 0; i < _sapp.drop.num_files; i++) {
            NSURL *fileUrl = [NSURL fileURLWithPath:[pboard.pasteboardItems[(NSUInteger)i] stringForType:NSPasteboardTypeFileURL]];
            if (!_sapp_strcpy(fileUrl.standardizedURL.path.UTF8String, _sapp_dropped_file_path_ptr(i), _sapp.drop.max_path_length)) {
                _SAPP_ERROR(DROPPED_FILE_PATH_TOO_LONG);
                drop_failed = true;
                break;
            }
        }
        if (!drop_failed) {
            if (_sapp_events_enabled()) {
                _sapp_macos_mouse_update_from_nspoint(sender.draggingLocation, true);
                _sapp_init_event(SAPP_EVENTTYPE_FILES_DROPPED);
                _sapp.event.modifiers = _sapp_macos_mods(nil);
                _sapp_call_event(&_sapp.event);
            }
        }
        else {
            _sapp_clear_drop_buffer();
            _sapp.drop.num_files = 0;
        }
        return YES;
    }
    #endif
    return NO;
}
@end

@implementation _sapp_macos_view
#if defined(SOKOL_GLCORE33)
- (void)timerFired:(id)sender {
    _SOKOL_UNUSED(sender);
    [self setNeedsDisplay:YES];
}
- (void)prepareOpenGL {
    [super prepareOpenGL];
    GLint swapInt = 1;
    NSOpenGLContext* ctx = [_sapp.macos.view openGLContext];
    [ctx setValues:&swapInt forParameter:NSOpenGLContextParameterSwapInterval];
    [ctx makeCurrentContext];
}
#endif

_SOKOL_PRIVATE void _sapp_macos_poll_input_events() {
    /*

    NOTE: late event polling temporarily out-commented to check if this
    causes infrequent and almost impossible to reproduce problems with the
    window close events, see:
    https://github.com/floooh/sokol/pull/483#issuecomment-805148815


    const NSEventMask mask = NSEventMaskLeftMouseDown |
                             NSEventMaskLeftMouseUp|
                             NSEventMaskRightMouseDown |
                             NSEventMaskRightMouseUp |
                             NSEventMaskMouseMoved |
                             NSEventMaskLeftMouseDragged |
                             NSEventMaskRightMouseDragged |
                             NSEventMaskMouseEntered |
                             NSEventMaskMouseExited |
                             NSEventMaskKeyDown |
                             NSEventMaskKeyUp |
                             NSEventMaskCursorUpdate |
                             NSEventMaskScrollWheel |
                             NSEventMaskTabletPoint |
                             NSEventMaskTabletProximity |
                             NSEventMaskOtherMouseDown |
                             NSEventMaskOtherMouseUp |
                             NSEventMaskOtherMouseDragged |
                             NSEventMaskPressure |
                             NSEventMaskDirectTouch;
    @autoreleasepool {
        for (;;) {
            // NOTE: using NSDefaultRunLoopMode here causes stuttering in the GL backend,
            // see: https://github.com/floooh/sokol/issues/486
            NSEvent* event = [NSApp nextEventMatchingMask:mask untilDate:nil inMode:NSEventTrackingRunLoopMode dequeue:YES];
            if (event == nil) {
                break;
            }
            [NSApp sendEvent:event];
        }
    }
    */
}

- (void)drawRect:(NSRect)rect {
    _SOKOL_UNUSED(rect);
    _sapp_timing_measure(&_sapp.timing);
    /* Catch any last-moment input events */
    _sapp_macos_poll_input_events();
    @autoreleasepool {
        _sapp_macos_frame();
    }
    #if !defined(SOKOL_METAL)
    [[_sapp.macos.view openGLContext] flushBuffer];
    #endif
}

- (BOOL)isOpaque {
    return YES;
}
- (BOOL)canBecomeKeyView {
    return YES;
}
- (BOOL)acceptsFirstResponder {
    return YES;
}
- (void)updateTrackingAreas {
    if (_sapp.macos.tracking_area != nil) {
        [self removeTrackingArea:_sapp.macos.tracking_area];
        _SAPP_OBJC_RELEASE(_sapp.macos.tracking_area);
    }
    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
                                          NSTrackingActiveInKeyWindow |
                                          NSTrackingEnabledDuringMouseDrag |
                                          NSTrackingCursorUpdate |
                                          NSTrackingInVisibleRect |
                                          NSTrackingAssumeInside;
    _sapp.macos.tracking_area = [[NSTrackingArea alloc] initWithRect:[self bounds] options:options owner:self userInfo:nil];
    [self addTrackingArea:_sapp.macos.tracking_area];
    [super updateTrackingAreas];
}

// helper function to make GL context active
static void _sapp_gl_make_current(void) {
    #if defined(SOKOL_GLCORE33)
    [[_sapp.macos.view openGLContext] makeCurrentContext];
    #endif
}

- (void)mouseEntered:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, true);
    /* don't send mouse enter/leave while dragging (so that it behaves the same as
       on Windows while SetCapture is active
    */
    if (0 == _sapp.macos.mouse_buttons) {
        _sapp_macos_mouse_event(SAPP_EVENTTYPE_MOUSE_ENTER, SAPP_MOUSEBUTTON_INVALID, _sapp_macos_mods(event));
    }
}
- (void)mouseExited:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, true);
    if (0 == _sapp.macos.mouse_buttons) {
        _sapp_macos_mouse_event(SAPP_EVENTTYPE_MOUSE_LEAVE, SAPP_MOUSEBUTTON_INVALID, _sapp_macos_mods(event));
    }
}
- (void)mouseDown:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, false);
    _sapp_macos_mouse_event(SAPP_EVENTTYPE_MOUSE_DOWN, SAPP_MOUSEBUTTON_LEFT, _sapp_macos_mods(event));
    _sapp.macos.mouse_buttons |= (1<<SAPP_MOUSEBUTTON_LEFT);
}
- (void)mouseUp:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, false);
    _sapp_macos_mouse_event(SAPP_EVENTTYPE_MOUSE_UP, SAPP_MOUSEBUTTON_LEFT, _sapp_macos_mods(event));
    _sapp.macos.mouse_buttons &= ~(1<<SAPP_MOUSEBUTTON_LEFT);
}
- (void)rightMouseDown:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, false);
    _sapp_macos_mouse_event(SAPP_EVENTTYPE_MOUSE_DOWN, SAPP_MOUSEBUTTON_RIGHT, _sapp_macos_mods(event));
    _sapp.macos.mouse_buttons |= (1<<SAPP_MOUSEBUTTON_RIGHT);
}
- (void)rightMouseUp:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, false);
    _sapp_macos_mouse_event(SAPP_EVENTTYPE_MOUSE_UP, SAPP_MOUSEBUTTON_RIGHT, _sapp_macos_mods(event));
    _sapp.macos.mouse_buttons &= ~(1<<SAPP_MOUSEBUTTON_RIGHT);
}
- (void)otherMouseDown:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, false);
    if (2 == event.buttonNumber) {
        _sapp_macos_mouse_event(SAPP_EVENTTYPE_MOUSE_DOWN, SAPP_MOUSEBUTTON_MIDDLE, _sapp_macos_mods(event));
        _sapp.macos.mouse_buttons |= (1<<SAPP_MOUSEBUTTON_MIDDLE);
    }
}
- (void)otherMouseUp:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, false);
    if (2 == event.buttonNumber) {
        _sapp_macos_mouse_event(SAPP_EVENTTYPE_MOUSE_UP, SAPP_MOUSEBUTTON_MIDDLE, _sapp_macos_mods(event));
        _sapp.macos.mouse_buttons &= (1<<SAPP_MOUSEBUTTON_MIDDLE);
    }
}
- (void)otherMouseDragged:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, false);
    if (2 == event.buttonNumber) {
        if (_sapp.mouse.locked) {
            _sapp.mouse.dx = [event deltaX];
            _sapp.mouse.dy = [event deltaY];
        }
        _sapp_macos_mouse_event(SAPP_EVENTTYPE_MOUSE_MOVE, SAPP_MOUSEBUTTON_INVALID, _sapp_macos_mods(event));
    }
}
- (void)mouseMoved:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, false);
    if (_sapp.mouse.locked) {
        _sapp.mouse.dx = [event deltaX];
        _sapp.mouse.dy = [event deltaY];
    }
    _sapp_macos_mouse_event(SAPP_EVENTTYPE_MOUSE_MOVE, SAPP_MOUSEBUTTON_INVALID , _sapp_macos_mods(event));
}
- (void)mouseDragged:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, false);
    if (_sapp.mouse.locked) {
        _sapp.mouse.dx = [event deltaX];
        _sapp.mouse.dy = [event deltaY];
    }
    _sapp_macos_mouse_event(SAPP_EVENTTYPE_MOUSE_MOVE, SAPP_MOUSEBUTTON_INVALID , _sapp_macos_mods(event));
}
- (void)rightMouseDragged:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, false);
    if (_sapp.mouse.locked) {
        _sapp.mouse.dx = [event deltaX];
        _sapp.mouse.dy = [event deltaY];
    }
    _sapp_macos_mouse_event(SAPP_EVENTTYPE_MOUSE_MOVE, SAPP_MOUSEBUTTON_INVALID, _sapp_macos_mods(event));
}
- (void)scrollWheel:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_mouse_update_from_nsevent(event, true);
    if (_sapp_events_enabled()) {
        float dx = (float) event.scrollingDeltaX;
        float dy = (float) event.scrollingDeltaY;
        if (event.hasPreciseScrollingDeltas) {
            dx *= 0.1;
            dy *= 0.1;
        }
        if ((_sapp_absf(dx) > 0.0f) || (_sapp_absf(dy) > 0.0f)) {
            _sapp_init_event(SAPP_EVENTTYPE_MOUSE_SCROLL);
            _sapp.event.modifiers = _sapp_macos_mods(event);
            _sapp.event.scroll_x = dx;
            _sapp.event.scroll_y = dy;
            _sapp_call_event(&_sapp.event);
        }
    }
}
- (void)keyDown:(NSEvent*)event {
    if (_sapp_events_enabled()) {
        _sapp_gl_make_current();
        const uint32_t mods = _sapp_macos_mods(event);
        const sapp_keycode key_code = _sapp_translate_key(event.keyCode);
        _sapp_macos_key_event(SAPP_EVENTTYPE_KEY_DOWN, key_code, event.isARepeat, mods);
        const NSString* chars = event.characters;
        const NSUInteger len = chars.length;
        if (len > 0) {
            _sapp_init_event(SAPP_EVENTTYPE_CHAR);
            _sapp.event.modifiers = mods;
            for (NSUInteger i = 0; i < len; i++) {
                const unichar codepoint = [chars characterAtIndex:i];
                if ((codepoint & 0xFF00) == 0xF700) {
                    continue;
                }
                _sapp.event.char_code = codepoint;
                _sapp.event.key_repeat = event.isARepeat;
                _sapp_call_event(&_sapp.event);
            }
        }
        /* if this is a Cmd+V (paste), also send a CLIPBOARD_PASTE event */
        if (_sapp.clipboard.enabled && (mods == SAPP_MODIFIER_SUPER) && (key_code == SAPP_KEYCODE_V)) {
            _sapp_init_event(SAPP_EVENTTYPE_CLIPBOARD_PASTED);
            _sapp_call_event(&_sapp.event);
        }
    }
}
- (void)keyUp:(NSEvent*)event {
    _sapp_gl_make_current();
    _sapp_macos_key_event(SAPP_EVENTTYPE_KEY_UP,
        _sapp_translate_key(event.keyCode),
        event.isARepeat,
        _sapp_macos_mods(event));
}
- (void)flagsChanged:(NSEvent*)event {
    const uint32_t old_f = _sapp.macos.flags_changed_store;
    const uint32_t new_f = (uint32_t)event.modifierFlags;
    _sapp.macos.flags_changed_store = new_f;
    sapp_keycode key_code = SAPP_KEYCODE_INVALID;
    bool down = false;
    if ((new_f ^ old_f) & NSEventModifierFlagShift) {
        key_code = SAPP_KEYCODE_LEFT_SHIFT;
        down = 0 != (new_f & NSEventModifierFlagShift);
    }
    if ((new_f ^ old_f) & NSEventModifierFlagControl) {
        key_code = SAPP_KEYCODE_LEFT_CONTROL;
        down = 0 != (new_f & NSEventModifierFlagControl);
    }
    if ((new_f ^ old_f) & NSEventModifierFlagOption) {
        key_code = SAPP_KEYCODE_LEFT_ALT;
        down = 0 != (new_f & NSEventModifierFlagOption);
    }
    if ((new_f ^ old_f) & NSEventModifierFlagCommand) {
        key_code = SAPP_KEYCODE_LEFT_SUPER;
        down = 0 != (new_f & NSEventModifierFlagCommand);
    }
    if (key_code != SAPP_KEYCODE_INVALID) {
        _sapp_macos_key_event(down ? SAPP_EVENTTYPE_KEY_DOWN : SAPP_EVENTTYPE_KEY_UP,
            key_code,
            false,
            _sapp_macos_mods(event));
    }
}
@end

#endif // macOS

// ██  ██████  ███████
// ██ ██    ██ ██
// ██ ██    ██ ███████
// ██ ██    ██      ██
// ██  ██████  ███████
//
// >>ios
#if defined(_SAPP_IOS)

_SOKOL_PRIVATE void _sapp_ios_discard_state(void) {
    // NOTE: it's safe to call [release] on a nil object
    _SAPP_OBJC_RELEASE(_sapp.ios.textfield_dlg);
    _SAPP_OBJC_RELEASE(_sapp.ios.textfield);
    #if defined(SOKOL_METAL)
        _SAPP_OBJC_RELEASE(_sapp.ios.view_ctrl);
        _SAPP_OBJC_RELEASE(_sapp.ios.mtl_device);
    #else
        _SAPP_OBJC_RELEASE(_sapp.ios.view_ctrl);
        _SAPP_OBJC_RELEASE(_sapp.ios.eagl_ctx);
    #endif
    _SAPP_OBJC_RELEASE(_sapp.ios.view);
    _SAPP_OBJC_RELEASE(_sapp.ios.window);
}

_SOKOL_PRIVATE void _sapp_ios_run(const sapp_desc* desc) {
    _sapp_init_state(desc);
    static int argc = 1;
    static char* argv[] = { (char*)"sokol_app" };
    UIApplicationMain(argc, argv, nil, NSStringFromClass([_sapp_app_delegate class]));
}

/* iOS entry function */
#if !defined(SOKOL_NO_ENTRY)
int main(int argc, char* argv[]) {
    sapp_desc desc = sokol_main(argc, argv);
    _sapp_ios_run(&desc);
    return 0;
}
#endif /* SOKOL_NO_ENTRY */

_SOKOL_PRIVATE void _sapp_ios_app_event(sapp_event_type type) {
    if (_sapp_events_enabled()) {
        _sapp_init_event(type);
        _sapp_call_event(&_sapp.event);
    }
}

_SOKOL_PRIVATE void _sapp_ios_touch_event(sapp_event_type type, NSSet<UITouch *>* touches, UIEvent* event) {
    if (_sapp_events_enabled()) {
        _sapp_init_event(type);
        NSEnumerator* enumerator = event.allTouches.objectEnumerator;
        UITouch* ios_touch;
        while ((ios_touch = [enumerator nextObject])) {
            if ((_sapp.event.num_touches + 1) < SAPP_MAX_TOUCHPOINTS) {
                CGPoint ios_pos = [ios_touch locationInView:_sapp.ios.view];
                sapp_touchpoint* cur_point = &_sapp.event.touches[_sapp.event.num_touches++];
                cur_point->identifier = (uintptr_t) ios_touch;
                cur_point->pos_x = ios_pos.x * _sapp.dpi_scale;
                cur_point->pos_y = ios_pos.y * _sapp.dpi_scale;
                cur_point->changed = [touches containsObject:ios_touch];
            }
        }
        if (_sapp.event.num_touches > 0) {
            _sapp_call_event(&_sapp.event);
        }
    }
}

_SOKOL_PRIVATE void _sapp_ios_update_dimensions(void) {
    CGRect screen_rect = UIScreen.mainScreen.bounds;
    _sapp.framebuffer_width = (int)roundf(screen_rect.size.width * _sapp.dpi_scale);
    _sapp.framebuffer_height = (int)roundf(screen_rect.size.height * _sapp.dpi_scale);
    _sapp.window_width = (int)roundf(screen_rect.size.width);
    _sapp.window_height = (int)roundf(screen_rect.size.height);
    int cur_fb_width, cur_fb_height;
    #if defined(SOKOL_METAL)
        const CGSize fb_size = _sapp.ios.view.drawableSize;
        cur_fb_width = (int)roundf(fb_size.width);
        cur_fb_height = (int)roundf(fb_size.height);
    #else
        cur_fb_width = (int)roundf(_sapp.ios.view.drawableWidth);
        cur_fb_height = (int)roundf(_sapp.ios.view.drawableHeight);
    #endif
    const bool dim_changed = (_sapp.framebuffer_width != cur_fb_width) ||
                             (_sapp.framebuffer_height != cur_fb_height);
    if (dim_changed) {
        #if defined(SOKOL_METAL)
            const CGSize drawable_size = { (CGFloat) _sapp.framebuffer_width, (CGFloat) _sapp.framebuffer_height };
            _sapp.ios.view.drawableSize = drawable_size;
        #else
            // nothing to do here, GLKView correctly respects the view's contentScaleFactor
        #endif
        if (!_sapp.first_frame) {
            _sapp_ios_app_event(SAPP_EVENTTYPE_RESIZED);
        }
    }
}

_SOKOL_PRIVATE void _sapp_ios_frame(void) {
    _sapp_ios_update_dimensions();
    _sapp_frame();
}

_SOKOL_PRIVATE void _sapp_ios_show_keyboard(bool shown) {
    /* if not happened yet, create an invisible text field */
    if (nil == _sapp.ios.textfield) {
        _sapp.ios.textfield_dlg = [[_sapp_textfield_dlg alloc] init];
        _sapp.ios.textfield = [[UITextField alloc] initWithFrame:CGRectMake(10, 10, 100, 50)];
        _sapp.ios.textfield.keyboardType = UIKeyboardTypeDefault;
        _sapp.ios.textfield.returnKeyType = UIReturnKeyDefault;
        _sapp.ios.textfield.autocapitalizationType = UITextAutocapitalizationTypeNone;
        _sapp.ios.textfield.autocorrectionType = UITextAutocorrectionTypeNo;
        _sapp.ios.textfield.spellCheckingType = UITextSpellCheckingTypeNo;
        _sapp.ios.textfield.hidden = YES;
        _sapp.ios.textfield.text = @"x";
        _sapp.ios.textfield.delegate = _sapp.ios.textfield_dlg;
        [_sapp.ios.view_ctrl.view addSubview:_sapp.ios.textfield];

        [[NSNotificationCenter defaultCenter] addObserver:_sapp.ios.textfield_dlg
            selector:@selector(keyboardWasShown:)
            name:UIKeyboardDidShowNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:_sapp.ios.textfield_dlg
            selector:@selector(keyboardWillBeHidden:)
            name:UIKeyboardWillHideNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:_sapp.ios.textfield_dlg
            selector:@selector(keyboardDidChangeFrame:)
            name:UIKeyboardDidChangeFrameNotification object:nil];
    }
    if (shown) {
        /* setting the text field as first responder brings up the onscreen keyboard */
        [_sapp.ios.textfield becomeFirstResponder];
    }
    else {
        [_sapp.ios.textfield resignFirstResponder];
    }
}

@implementation _sapp_app_delegate
- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    CGRect screen_rect = UIScreen.mainScreen.bounds;
    _sapp.ios.window = [[UIWindow alloc] initWithFrame:screen_rect];
    _sapp.window_width = (int)roundf(screen_rect.size.width);
    _sapp.window_height = (int)roundf(screen_rect.size.height);
    if (_sapp.desc.high_dpi) {
        _sapp.dpi_scale = (float) UIScreen.mainScreen.nativeScale;
    }
    else {
        _sapp.dpi_scale = 1.0f;
    }
    _sapp.framebuffer_width = (int)roundf(_sapp.window_width * _sapp.dpi_scale);
    _sapp.framebuffer_height = (int)roundf(_sapp.window_height * _sapp.dpi_scale);
    NSInteger max_fps = UIScreen.mainScreen.maximumFramesPerSecond;
    #if defined(SOKOL_METAL)
        _sapp.ios.mtl_device = MTLCreateSystemDefaultDevice();
        _sapp.ios.view = [[_sapp_ios_view alloc] init];
        _sapp.ios.view.preferredFramesPerSecond = max_fps / _sapp.swap_interval;
        _sapp.ios.view.device = _sapp.ios.mtl_device;
        _sapp.ios.view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        _sapp.ios.view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
        _sapp.ios.view.sampleCount = (NSUInteger)_sapp.sample_count;
        /* NOTE: iOS MTKView seems to ignore thew view's contentScaleFactor
            and automatically renders at Retina resolution. We'll disable
            autoResize and instead do the resizing in _sapp_ios_update_dimensions()
        */
        _sapp.ios.view.autoResizeDrawable = false;
        _sapp.ios.view.userInteractionEnabled = YES;
        _sapp.ios.view.multipleTouchEnabled = YES;
        _sapp.ios.view_ctrl = [[UIViewController alloc] init];
        _sapp.ios.view_ctrl.modalPresentationStyle = UIModalPresentationFullScreen;
        _sapp.ios.view_ctrl.view = _sapp.ios.view;
        _sapp.ios.window.rootViewController = _sapp.ios.view_ctrl;
    #else
        _sapp.ios.eagl_ctx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        _sapp.ios.view = [[_sapp_ios_view alloc] initWithFrame:screen_rect];
        _sapp.ios.view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
        _sapp.ios.view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
        _sapp.ios.view.drawableStencilFormat = GLKViewDrawableStencilFormatNone;
        GLKViewDrawableMultisample msaa = _sapp.sample_count > 1 ? GLKViewDrawableMultisample4X : GLKViewDrawableMultisampleNone;
        _sapp.ios.view.drawableMultisample = msaa;
        _sapp.ios.view.context = _sapp.ios.eagl_ctx;
        _sapp.ios.view.enableSetNeedsDisplay = NO;
        _sapp.ios.view.userInteractionEnabled = YES;
        _sapp.ios.view.multipleTouchEnabled = YES;
        // on GLKView, contentScaleFactor appears to work just fine!
        if (_sapp.desc.high_dpi) {
            _sapp.ios.view.contentScaleFactor = _sapp.dpi_scale;
        }
        else {
            _sapp.ios.view.contentScaleFactor = 1.0;
        }
        _sapp.ios.view_ctrl = [[GLKViewController alloc] init];
        _sapp.ios.view_ctrl.view = _sapp.ios.view;
        _sapp.ios.view_ctrl.preferredFramesPerSecond = max_fps / _sapp.swap_interval;
        _sapp.ios.window.rootViewController = _sapp.ios.view_ctrl;
    #endif
    [_sapp.ios.window makeKeyAndVisible];

    _sapp.valid = true;
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    if (!_sapp.ios.suspended) {
        _sapp.ios.suspended = true;
        _sapp_ios_app_event(SAPP_EVENTTYPE_SUSPENDED);
    }
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    if (_sapp.ios.suspended) {
        _sapp.ios.suspended = false;
        _sapp_ios_app_event(SAPP_EVENTTYPE_RESUMED);
    }
}

/* NOTE: this method will rarely ever be called, iOS application
    which are terminated by the user are usually killed via signal 9
    by the operating system.
*/
- (void)applicationWillTerminate:(UIApplication *)application {
    _SOKOL_UNUSED(application);
    _sapp_call_cleanup();
    _sapp_ios_discard_state();
    _sapp_discard_state();
}
@end

@implementation _sapp_textfield_dlg
- (void)keyboardWasShown:(NSNotification*)notif {
    _sapp.onscreen_keyboard_shown = true;
    /* query the keyboard's size, and modify the content view's size */
    if (_sapp.desc.ios_keyboard_resizes_canvas) {
        NSDictionary* info = notif.userInfo;
        CGFloat kbd_h = [[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue].size.height;
        CGRect view_frame = UIScreen.mainScreen.bounds;
        view_frame.size.height -= kbd_h;
        _sapp.ios.view.frame = view_frame;
    }
}
- (void)keyboardWillBeHidden:(NSNotification*)notif {
    _sapp.onscreen_keyboard_shown = false;
    if (_sapp.desc.ios_keyboard_resizes_canvas) {
        _sapp.ios.view.frame = UIScreen.mainScreen.bounds;
    }
}
- (void)keyboardDidChangeFrame:(NSNotification*)notif {
    /* this is for the case when the screen rotation changes while the keyboard is open */
    if (_sapp.onscreen_keyboard_shown && _sapp.desc.ios_keyboard_resizes_canvas) {
        NSDictionary* info = notif.userInfo;
        CGFloat kbd_h = [[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue].size.height;
        CGRect view_frame = UIScreen.mainScreen.bounds;
        view_frame.size.height -= kbd_h;
        _sapp.ios.view.frame = view_frame;
    }
}
- (BOOL)textField:(UITextField*)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString*)string {
    if (_sapp_events_enabled()) {
        const NSUInteger len = string.length;
        if (len > 0) {
            for (NSUInteger i = 0; i < len; i++) {
                unichar c = [string characterAtIndex:i];
                if (c >= 32) {
                    /* ignore surrogates for now */
                    if ((c < 0xD800) || (c > 0xDFFF)) {
                        _sapp_init_event(SAPP_EVENTTYPE_CHAR);
                        _sapp.event.char_code = c;
                        _sapp_call_event(&_sapp.event);
                    }
                }
                if (c <= 32) {
                    sapp_keycode k = SAPP_KEYCODE_INVALID;
                    switch (c) {
                        case 10: k = SAPP_KEYCODE_ENTER; break;
                        case 32: k = SAPP_KEYCODE_SPACE; break;
                        default: break;
                    }
                    if (k != SAPP_KEYCODE_INVALID) {
                        _sapp_init_event(SAPP_EVENTTYPE_KEY_DOWN);
                        _sapp.event.key_code = k;
                        _sapp_call_event(&_sapp.event);
                        _sapp_init_event(SAPP_EVENTTYPE_KEY_UP);
                        _sapp.event.key_code = k;
                        _sapp_call_event(&_sapp.event);
                    }
                }
            }
        }
        else {
            /* this was a backspace */
            _sapp_init_event(SAPP_EVENTTYPE_KEY_DOWN);
            _sapp.event.key_code = SAPP_KEYCODE_BACKSPACE;
            _sapp_call_event(&_sapp.event);
            _sapp_init_event(SAPP_EVENTTYPE_KEY_UP);
            _sapp.event.key_code = SAPP_KEYCODE_BACKSPACE;
            _sapp_call_event(&_sapp.event);
        }
    }
    return NO;
}
@end

@implementation _sapp_ios_view
- (void)drawRect:(CGRect)rect {
    _SOKOL_UNUSED(rect);
    _sapp_timing_measure(&_sapp.timing);
    @autoreleasepool {
        _sapp_ios_frame();
    }
}
- (BOOL)isOpaque {
    return YES;
}
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent*)event {
    _sapp_ios_touch_event(SAPP_EVENTTYPE_TOUCHES_BEGAN, touches, event);
}
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent*)event {
    _sapp_ios_touch_event(SAPP_EVENTTYPE_TOUCHES_MOVED, touches, event);
}
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent*)event {
    _sapp_ios_touch_event(SAPP_EVENTTYPE_TOUCHES_ENDED, touches, event);
}
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent*)event {
    _sapp_ios_touch_event(SAPP_EVENTTYPE_TOUCHES_CANCELLED, touches, event);
}
@end
#endif /* TARGET_OS_IPHONE */

#endif /* _SAPP_APPLE */

//  ██████  ██          ██   ██ ███████ ██      ██████  ███████ ██████  ███████
// ██       ██          ██   ██ ██      ██      ██   ██ ██      ██   ██ ██
// ██   ███ ██          ███████ █████   ██      ██████  █████   ██████  ███████
// ██    ██ ██          ██   ██ ██      ██      ██      ██      ██   ██      ██
//  ██████  ███████     ██   ██ ███████ ███████ ██      ███████ ██   ██ ███████
//
// >>gl helpers
#if defined(SOKOL_GLCORE33)
typedef struct {
    int         red_bits;
    int         green_bits;
    int         blue_bits;
    int         alpha_bits;
    int         depth_bits;
    int         stencil_bits;
    int         samples;
    bool        doublebuffer;
    uintptr_t   handle;
} _sapp_gl_fbconfig;

_SOKOL_PRIVATE void _sapp_gl_init_fbconfig(_sapp_gl_fbconfig* fbconfig) {
    _sapp_clear(fbconfig, sizeof(_sapp_gl_fbconfig));
    /* -1 means "don't care" */
    fbconfig->red_bits = -1;
    fbconfig->green_bits = -1;
    fbconfig->blue_bits = -1;
    fbconfig->alpha_bits = -1;
    fbconfig->depth_bits = -1;
    fbconfig->stencil_bits = -1;
    fbconfig->samples = -1;
}

typedef struct {
    int least_missing;
    int least_color_diff;
    int least_extra_diff;
    bool best_match;
} _sapp_gl_fbselect;

_SOKOL_PRIVATE void _sapp_gl_init_fbselect(_sapp_gl_fbselect* fbselect) {
    _sapp_clear(fbselect, sizeof(_sapp_gl_fbselect));
    fbselect->least_missing = 1000000;
    fbselect->least_color_diff = 10000000;
    fbselect->least_extra_diff = 10000000;
    fbselect->best_match = false;
}

// NOTE: this is used only in the WGL code path
_SOKOL_PRIVATE bool _sapp_gl_select_fbconfig(_sapp_gl_fbselect* fbselect, const _sapp_gl_fbconfig* desired, const _sapp_gl_fbconfig* current) {
    int missing = 0;
    if (desired->doublebuffer != current->doublebuffer) {
        return false;
    }

    if ((desired->alpha_bits > 0) && (current->alpha_bits == 0)) {
        missing++;
    }
    if ((desired->depth_bits > 0) && (current->depth_bits == 0)) {
        missing++;
    }
    if ((desired->stencil_bits > 0) && (current->stencil_bits == 0)) {
        missing++;
    }
    if ((desired->samples > 0) && (current->samples == 0)) {
        /* Technically, several multisampling buffers could be
            involved, but that's a lower level implementation detail and
            not important to us here, so we count them as one
        */
        missing++;
    }

    /* These polynomials make many small channel size differences matter
        less than one large channel size difference
        Calculate color channel size difference value
    */
    int color_diff = 0;
    if (desired->red_bits != -1) {
        color_diff += (desired->red_bits - current->red_bits) * (desired->red_bits - current->red_bits);
    }
    if (desired->green_bits != -1) {
        color_diff += (desired->green_bits - current->green_bits) * (desired->green_bits - current->green_bits);
    }
    if (desired->blue_bits != -1) {
        color_diff += (desired->blue_bits - current->blue_bits) * (desired->blue_bits - current->blue_bits);
    }

    /* Calculate non-color channel size difference value */
    int extra_diff = 0;
    if (desired->alpha_bits != -1) {
        extra_diff += (desired->alpha_bits - current->alpha_bits) * (desired->alpha_bits - current->alpha_bits);
    }
    if (desired->depth_bits != -1) {
        extra_diff += (desired->depth_bits - current->depth_bits) * (desired->depth_bits - current->depth_bits);
    }
    if (desired->stencil_bits != -1) {
        extra_diff += (desired->stencil_bits - current->stencil_bits) * (desired->stencil_bits - current->stencil_bits);
    }
    if (desired->samples != -1) {
        extra_diff += (desired->samples - current->samples) * (desired->samples - current->samples);
    }

    /* Figure out if the current one is better than the best one found so far
        Least number of missing buffers is the most important heuristic,
        then color buffer size match and lastly size match for other buffers
    */
    bool new_closest = false;
    if (missing < fbselect->least_missing) {
        new_closest = true;
    } else if (missing == fbselect->least_missing) {
        if ((color_diff < fbselect->least_color_diff) ||
            ((color_diff == fbselect->least_color_diff) && (extra_diff < fbselect->least_extra_diff)))
        {
            new_closest = true;
        }
    }
    if (new_closest) {
        fbselect->least_missing = missing;
        fbselect->least_color_diff = color_diff;
        fbselect->least_extra_diff = extra_diff;
        fbselect->best_match = (missing | color_diff | extra_diff) == 0;
    }
    return new_closest;
}

// NOTE: this is used only in the GLX code path
_SOKOL_PRIVATE const _sapp_gl_fbconfig* _sapp_gl_choose_fbconfig(const _sapp_gl_fbconfig* desired, const _sapp_gl_fbconfig* alternatives, int count) {
    int missing, least_missing = 1000000;
    int color_diff, least_color_diff = 10000000;
    int extra_diff, least_extra_diff = 10000000;
    const _sapp_gl_fbconfig* current;
    const _sapp_gl_fbconfig* closest = 0;
    for (int i = 0;  i < count;  i++) {
        current = alternatives + i;
        if (desired->doublebuffer != current->doublebuffer) {
            continue;
        }
        missing = 0;
        if (desired->alpha_bits > 0 && current->alpha_bits == 0) {
            missing++;
        }
        if (desired->depth_bits > 0 && current->depth_bits == 0) {
            missing++;
        }
        if (desired->stencil_bits > 0 && current->stencil_bits == 0) {
            missing++;
        }
        if (desired->samples > 0 && current->samples == 0) {
            /* Technically, several multisampling buffers could be
                involved, but that's a lower level implementation detail and
                not important to us here, so we count them as one
            */
            missing++;
        }

        /* These polynomials make many small channel size differences matter
            less than one large channel size difference
            Calculate color channel size difference value
        */
        color_diff = 0;
        if (desired->red_bits != -1) {
            color_diff += (desired->red_bits - current->red_bits) * (desired->red_bits - current->red_bits);
        }
        if (desired->green_bits != -1) {
            color_diff += (desired->green_bits - current->green_bits) * (desired->green_bits - current->green_bits);
        }
        if (desired->blue_bits != -1) {
            color_diff += (desired->blue_bits - current->blue_bits) * (desired->blue_bits - current->blue_bits);
        }

        /* Calculate non-color channel size difference value */
        extra_diff = 0;
        if (desired->alpha_bits != -1) {
            extra_diff += (desired->alpha_bits - current->alpha_bits) * (desired->alpha_bits - current->alpha_bits);
        }
        if (desired->depth_bits != -1) {
            extra_diff += (desired->depth_bits - current->depth_bits) * (desired->depth_bits - current->depth_bits);
        }
        if (desired->stencil_bits != -1) {
            extra_diff += (desired->stencil_bits - current->stencil_bits) * (desired->stencil_bits - current->stencil_bits);
        }
        if (desired->samples != -1) {
            extra_diff += (desired->samples - current->samples) * (desired->samples - current->samples);
        }

        /* Figure out if the current one is better than the best one found so far
            Least number of missing buffers is the most important heuristic,
            then color buffer size match and lastly size match for other buffers
        */
        if (missing < least_missing) {
            closest = current;
        }
        else if (missing == least_missing) {
            if ((color_diff < least_color_diff) ||
                (color_diff == least_color_diff && extra_diff < least_extra_diff))
            {
                closest = current;
            }
        }
        if (current == closest) {
            least_missing = missing;
            least_color_diff = color_diff;
            least_extra_diff = extra_diff;
        }
    }
    return closest;
}
#endif

// ██     ██ ██ ███    ██ ██████   ██████  ██     ██ ███████
// ██     ██ ██ ████   ██ ██   ██ ██    ██ ██     ██ ██
// ██  █  ██ ██ ██ ██  ██ ██   ██ ██    ██ ██  █  ██ ███████
// ██ ███ ██ ██ ██  ██ ██ ██   ██ ██    ██ ██ ███ ██      ██
//  ███ ███  ██ ██   ████ ██████   ██████   ███ ███  ███████
//
// >>windows
#if defined(_SAPP_WIN32)
_SOKOL_PRIVATE bool _sapp_win32_utf8_to_wide(const char* src, wchar_t* dst, int dst_num_bytes) {
    SOKOL_ASSERT(src && dst && (dst_num_bytes > 1));
    _sapp_clear(dst, (size_t)dst_num_bytes);
    const int dst_chars = dst_num_bytes / (int)sizeof(wchar_t);
    const int dst_needed = MultiByteToWideChar(CP_UTF8, 0, src, -1, 0, 0);
    if ((dst_needed > 0) && (dst_needed < dst_chars)) {
        MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, dst_chars);
        return true;
    }
    else {
        /* input string doesn't fit into destination buffer */
        return false;
    }
}

_SOKOL_PRIVATE void _sapp_win32_app_event(sapp_event_type type) {
    if (_sapp_events_enabled()) {
        _sapp_init_event(type);
        _sapp_call_event(&_sapp.event);
    }
}

_SOKOL_PRIVATE void _sapp_win32_init_keytable(void) {
    /* same as GLFW */
    _sapp.keycodes[0x00B] = SAPP_KEYCODE_0;
    _sapp.keycodes[0x002] = SAPP_KEYCODE_1;
    _sapp.keycodes[0x003] = SAPP_KEYCODE_2;
    _sapp.keycodes[0x004] = SAPP_KEYCODE_3;
    _sapp.keycodes[0x005] = SAPP_KEYCODE_4;
    _sapp.keycodes[0x006] = SAPP_KEYCODE_5;
    _sapp.keycodes[0x007] = SAPP_KEYCODE_6;
    _sapp.keycodes[0x008] = SAPP_KEYCODE_7;
    _sapp.keycodes[0x009] = SAPP_KEYCODE_8;
    _sapp.keycodes[0x00A] = SAPP_KEYCODE_9;
    _sapp.keycodes[0x01E] = SAPP_KEYCODE_A;
    _sapp.keycodes[0x030] = SAPP_KEYCODE_B;
    _sapp.keycodes[0x02E] = SAPP_KEYCODE_C;
    _sapp.keycodes[0x020] = SAPP_KEYCODE_D;
    _sapp.keycodes[0x012] = SAPP_KEYCODE_E;
    _sapp.keycodes[0x021] = SAPP_KEYCODE_F;
    _sapp.keycodes[0x022] = SAPP_KEYCODE_G;
    _sapp.keycodes[0x023] = SAPP_KEYCODE_H;
    _sapp.keycodes[0x017] = SAPP_KEYCODE_I;
    _sapp.keycodes[0x024] = SAPP_KEYCODE_J;
    _sapp.keycodes[0x025] = SAPP_KEYCODE_K;
    _sapp.keycodes[0x026] = SAPP_KEYCODE_L;
    _sapp.keycodes[0x032] = SAPP_KEYCODE_M;
    _sapp.keycodes[0x031] = SAPP_KEYCODE_N;
    _sapp.keycodes[0x018] = SAPP_KEYCODE_O;
    _sapp.keycodes[0x019] = SAPP_KEYCODE_P;
    _sapp.keycodes[0x010] = SAPP_KEYCODE_Q;
    _sapp.keycodes[0x013] = SAPP_KEYCODE_R;
    _sapp.keycodes[0x01F] = SAPP_KEYCODE_S;
    _sapp.keycodes[0x014] = SAPP_KEYCODE_T;
    _sapp.keycodes[0x016] = SAPP_KEYCODE_U;
    _sapp.keycodes[0x02F] = SAPP_KEYCODE_V;
    _sapp.keycodes[0x011] = SAPP_KEYCODE_W;
    _sapp.keycodes[0x02D] = SAPP_KEYCODE_X;
    _sapp.keycodes[0x015] = SAPP_KEYCODE_Y;
    _sapp.keycodes[0x02C] = SAPP_KEYCODE_Z;
    _sapp.keycodes[0x028] = SAPP_KEYCODE_APOSTROPHE;
    _sapp.keycodes[0x02B] = SAPP_KEYCODE_BACKSLASH;
    _sapp.keycodes[0x033] = SAPP_KEYCODE_COMMA;
    _sapp.keycodes[0x00D] = SAPP_KEYCODE_EQUAL;
    _sapp.keycodes[0x029] = SAPP_KEYCODE_GRAVE_ACCENT;
    _sapp.keycodes[0x01A] = SAPP_KEYCODE_LEFT_BRACKET;
    _sapp.keycodes[0x00C] = SAPP_KEYCODE_MINUS;
    _sapp.keycodes[0x034] = SAPP_KEYCODE_PERIOD;
    _sapp.keycodes[0x01B] = SAPP_KEYCODE_RIGHT_BRACKET;
    _sapp.keycodes[0x027] = SAPP_KEYCODE_SEMICOLON;
    _sapp.keycodes[0x035] = SAPP_KEYCODE_SLASH;
    _sapp.keycodes[0x056] = SAPP_KEYCODE_WORLD_2;
    _sapp.keycodes[0x00E] = SAPP_KEYCODE_BACKSPACE;
    _sapp.keycodes[0x153] = SAPP_KEYCODE_DELETE;
    _sapp.keycodes[0x14F] = SAPP_KEYCODE_END;
    _sapp.keycodes[0x01C] = SAPP_KEYCODE_ENTER;
    _sapp.keycodes[0x001] = SAPP_KEYCODE_ESCAPE;
    _sapp.keycodes[0x147] = SAPP_KEYCODE_HOME;
    _sapp.keycodes[0x152] = SAPP_KEYCODE_INSERT;
    _sapp.keycodes[0x15D] = SAPP_KEYCODE_MENU;
    _sapp.keycodes[0x151] = SAPP_KEYCODE_PAGE_DOWN;
    _sapp.keycodes[0x149] = SAPP_KEYCODE_PAGE_UP;
    _sapp.keycodes[0x045] = SAPP_KEYCODE_PAUSE;
    _sapp.keycodes[0x146] = SAPP_KEYCODE_PAUSE;
    _sapp.keycodes[0x039] = SAPP_KEYCODE_SPACE;
    _sapp.keycodes[0x00F] = SAPP_KEYCODE_TAB;
    _sapp.keycodes[0x03A] = SAPP_KEYCODE_CAPS_LOCK;
    _sapp.keycodes[0x145] = SAPP_KEYCODE_NUM_LOCK;
    _sapp.keycodes[0x046] = SAPP_KEYCODE_SCROLL_LOCK;
    _sapp.keycodes[0x03B] = SAPP_KEYCODE_F1;
    _sapp.keycodes[0x03C] = SAPP_KEYCODE_F2;
    _sapp.keycodes[0x03D] = SAPP_KEYCODE_F3;
    _sapp.keycodes[0x03E] = SAPP_KEYCODE_F4;
    _sapp.keycodes[0x03F] = SAPP_KEYCODE_F5;
    _sapp.keycodes[0x040] = SAPP_KEYCODE_F6;
    _sapp.keycodes[0x041] = SAPP_KEYCODE_F7;
    _sapp.keycodes[0x042] = SAPP_KEYCODE_F8;
    _sapp.keycodes[0x043] = SAPP_KEYCODE_F9;
    _sapp.keycodes[0x044] = SAPP_KEYCODE_F10;
    _sapp.keycodes[0x057] = SAPP_KEYCODE_F11;
    _sapp.keycodes[0x058] = SAPP_KEYCODE_F12;
    _sapp.keycodes[0x064] = SAPP_KEYCODE_F13;
    _sapp.keycodes[0x065] = SAPP_KEYCODE_F14;
    _sapp.keycodes[0x066] = SAPP_KEYCODE_F15;
    _sapp.keycodes[0x067] = SAPP_KEYCODE_F16;
    _sapp.keycodes[0x068] = SAPP_KEYCODE_F17;
    _sapp.keycodes[0x069] = SAPP_KEYCODE_F18;
    _sapp.keycodes[0x06A] = SAPP_KEYCODE_F19;
    _sapp.keycodes[0x06B] = SAPP_KEYCODE_F20;
    _sapp.keycodes[0x06C] = SAPP_KEYCODE_F21;
    _sapp.keycodes[0x06D] = SAPP_KEYCODE_F22;
    _sapp.keycodes[0x06E] = SAPP_KEYCODE_F23;
    _sapp.keycodes[0x076] = SAPP_KEYCODE_F24;
    _sapp.keycodes[0x038] = SAPP_KEYCODE_LEFT_ALT;
    _sapp.keycodes[0x01D] = SAPP_KEYCODE_LEFT_CONTROL;
    _sapp.keycodes[0x02A] = SAPP_KEYCODE_LEFT_SHIFT;
    _sapp.keycodes[0x15B] = SAPP_KEYCODE_LEFT_SUPER;
    _sapp.keycodes[0x137] = SAPP_KEYCODE_PRINT_SCREEN;
    _sapp.keycodes[0x138] = SAPP_KEYCODE_RIGHT_ALT;
    _sapp.keycodes[0x11D] = SAPP_KEYCODE_RIGHT_CONTROL;
    _sapp.keycodes[0x036] = SAPP_KEYCODE_RIGHT_SHIFT;
    _sapp.keycodes[0x15C] = SAPP_KEYCODE_RIGHT_SUPER;
    _sapp.keycodes[0x150] = SAPP_KEYCODE_DOWN;
    _sapp.keycodes[0x14B] = SAPP_KEYCODE_LEFT;
    _sapp.keycodes[0x14D] = SAPP_KEYCODE_RIGHT;
    _sapp.keycodes[0x148] = SAPP_KEYCODE_UP;
    _sapp.keycodes[0x052] = SAPP_KEYCODE_KP_0;
    _sapp.keycodes[0x04F] = SAPP_KEYCODE_KP_1;
    _sapp.keycodes[0x050] = SAPP_KEYCODE_KP_2;
    _sapp.keycodes[0x051] = SAPP_KEYCODE_KP_3;
    _sapp.keycodes[0x04B] = SAPP_KEYCODE_KP_4;
    _sapp.keycodes[0x04C] = SAPP_KEYCODE_KP_5;
    _sapp.keycodes[0x04D] = SAPP_KEYCODE_KP_6;
    _sapp.keycodes[0x047] = SAPP_KEYCODE_KP_7;
    _sapp.keycodes[0x048] = SAPP_KEYCODE_KP_8;
    _sapp.keycodes[0x049] = SAPP_KEYCODE_KP_9;
    _sapp.keycodes[0x04E] = SAPP_KEYCODE_KP_ADD;
    _sapp.keycodes[0x053] = SAPP_KEYCODE_KP_DECIMAL;
    _sapp.keycodes[0x135] = SAPP_KEYCODE_KP_DIVIDE;
    _sapp.keycodes[0x11C] = SAPP_KEYCODE_KP_ENTER;
    _sapp.keycodes[0x037] = SAPP_KEYCODE_KP_MULTIPLY;
    _sapp.keycodes[0x04A] = SAPP_KEYCODE_KP_SUBTRACT;
}
#endif // _SAPP_WIN32

#if defined(_SAPP_WIN32)

#if defined(SOKOL_D3D11)

#if defined(__cplusplus)
#define _sapp_d3d11_Release(self) (self)->Release()
#define _sapp_win32_refiid(iid) iid
#else
#define _sapp_d3d11_Release(self) (self)->lpVtbl->Release(self)
#define _sapp_win32_refiid(iid) &iid
#endif

#define _SAPP_SAFE_RELEASE(obj) if (obj) { _sapp_d3d11_Release(obj); obj=0; }


static const IID _sapp_IID_ID3D11Texture2D = { 0x6f15aaf2,0xd208,0x4e89, {0x9a,0xb4,0x48,0x95,0x35,0xd3,0x4f,0x9c} };
static const IID _sapp_IID_IDXGIDevice1    = { 0x77db970f,0x6276,0x48ba, {0xba,0x28,0x07,0x01,0x43,0xb4,0x39,0x2c} };
static const IID _sapp_IID_IDXGIFactory    = { 0x7b7166ec,0x21c7,0x44ae, {0xb2,0x1a,0xc9,0xae,0x32,0x1a,0xe3,0x69} };

static inline HRESULT _sapp_dxgi_GetBuffer(IDXGISwapChain* self, UINT Buffer, REFIID riid, void** ppSurface) {
    #if defined(__cplusplus)
        return self->GetBuffer(Buffer, riid, ppSurface);
    #else
        return self->lpVtbl->GetBuffer(self, Buffer, riid, ppSurface);
    #endif
}

static inline HRESULT _sapp_d3d11_QueryInterface(ID3D11Device* self, REFIID riid, void** ppvObject) {
    #if defined(__cplusplus)
        return self->QueryInterface(riid, ppvObject);
    #else
        return self->lpVtbl->QueryInterface(self, riid, ppvObject);
    #endif
}

static inline HRESULT _sapp_d3d11_CreateRenderTargetView(ID3D11Device* self, ID3D11Resource *pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView) {
    #if defined(__cplusplus)
        return self->CreateRenderTargetView(pResource, pDesc, ppRTView);
    #else
        return self->lpVtbl->CreateRenderTargetView(self, pResource, pDesc, ppRTView);
    #endif
}

static inline HRESULT _sapp_d3d11_CreateTexture2D(ID3D11Device* self, const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D) {
    #if defined(__cplusplus)
        return self->CreateTexture2D(pDesc, pInitialData, ppTexture2D);
    #else
        return self->lpVtbl->CreateTexture2D(self, pDesc, pInitialData, ppTexture2D);
    #endif
}

static inline HRESULT _sapp_d3d11_CreateDepthStencilView(ID3D11Device* self, ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView) {
    #if defined(__cplusplus)
        return self->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView);
    #else
        return self->lpVtbl->CreateDepthStencilView(self, pResource, pDesc, ppDepthStencilView);
    #endif
}

static inline void _sapp_d3d11_ResolveSubresource(ID3D11DeviceContext* self, ID3D11Resource* pDstResource, UINT DstSubresource, ID3D11Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format) {
    #if defined(__cplusplus)
        self->ResolveSubresource(pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format);
    #else
        self->lpVtbl->ResolveSubresource(self, pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format);
    #endif
}

static inline HRESULT _sapp_dxgi_ResizeBuffers(IDXGISwapChain* self, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    #if defined(__cplusplus)
        return self->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
    #else
        return self->lpVtbl->ResizeBuffers(self, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    #endif
}

static inline HRESULT _sapp_dxgi_Present(IDXGISwapChain* self, UINT SyncInterval, UINT Flags) {
    #if defined(__cplusplus)
        return self->Present(SyncInterval, Flags);
    #else
        return self->lpVtbl->Present(self, SyncInterval, Flags);
    #endif
}

static inline HRESULT _sapp_dxgi_GetFrameStatistics(IDXGISwapChain* self, DXGI_FRAME_STATISTICS* pStats) {
    #if defined(__cplusplus)
        return self->GetFrameStatistics(pStats);
    #else
        return self->lpVtbl->GetFrameStatistics(self, pStats);
    #endif
}

static inline HRESULT _sapp_dxgi_SetMaximumFrameLatency(IDXGIDevice1* self, UINT MaxLatency) {
    #if defined(__cplusplus)
        return self->SetMaximumFrameLatency(MaxLatency);
    #else
        return self->lpVtbl->SetMaximumFrameLatency(self, MaxLatency);
    #endif
}

static inline HRESULT _sapp_dxgi_GetAdapter(IDXGIDevice1* self, IDXGIAdapter** pAdapter) {
    #if defined(__cplusplus)
        return self->GetAdapter(pAdapter);
    #else
        return self->lpVtbl->GetAdapter(self, pAdapter);
    #endif
}

static inline HRESULT _sapp_dxgi_GetParent(IDXGIObject* self, REFIID riid, void** ppParent) {
    #if defined(__cplusplus)
        return self->GetParent(riid, ppParent);
    #else
        return self->lpVtbl->GetParent(self, riid, ppParent);
    #endif
}

static inline HRESULT _sapp_dxgi_MakeWindowAssociation(IDXGIFactory* self, HWND WindowHandle, UINT Flags) {
    #if defined(__cplusplus)
        return self->MakeWindowAssociation(WindowHandle, Flags);
    #else
        return self->lpVtbl->MakeWindowAssociation(self, WindowHandle, Flags);
    #endif
}

_SOKOL_PRIVATE void _sapp_d3d11_create_device_and_swapchain(void) {
    DXGI_SWAP_CHAIN_DESC* sc_desc = &_sapp.d3d11.swap_chain_desc;
    sc_desc->BufferDesc.Width = (UINT)_sapp.framebuffer_width;
    sc_desc->BufferDesc.Height = (UINT)_sapp.framebuffer_height;
    sc_desc->BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sc_desc->BufferDesc.RefreshRate.Numerator = 60;
    sc_desc->BufferDesc.RefreshRate.Denominator = 1;
    sc_desc->OutputWindow = _sapp.win32.hwnd;
    sc_desc->Windowed = true;
    if (_sapp.win32.is_win10_or_greater) {
        sc_desc->BufferCount = 2;
        sc_desc->SwapEffect = (DXGI_SWAP_EFFECT) _SAPP_DXGI_SWAP_EFFECT_FLIP_DISCARD;
        _sapp.d3d11.use_dxgi_frame_stats = true;
    }
    else {
        sc_desc->BufferCount = 1;
        sc_desc->SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        _sapp.d3d11.use_dxgi_frame_stats = false;
    }
    sc_desc->SampleDesc.Count = 1;
    sc_desc->SampleDesc.Quality = 0;
    sc_desc->BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    UINT create_flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    #if defined(SOKOL_DEBUG)
        create_flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif
    D3D_FEATURE_LEVEL feature_level;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,                           /* pAdapter (use default) */
        D3D_DRIVER_TYPE_HARDWARE,       /* DriverType */
        NULL,                           /* Software */
        create_flags,                   /* Flags */
        NULL,                           /* pFeatureLevels */
        0,                              /* FeatureLevels */
        D3D11_SDK_VERSION,              /* SDKVersion */
        sc_desc,                        /* pSwapChainDesc */
        &_sapp.d3d11.swap_chain,        /* ppSwapChain */
        &_sapp.d3d11.device,            /* ppDevice */
        &feature_level,                 /* pFeatureLevel */
        &_sapp.d3d11.device_context);   /* ppImmediateContext */
    _SOKOL_UNUSED(hr);
    #if defined(SOKOL_DEBUG)
    if (!SUCCEEDED(hr)) {
        // if initialization with D3D11_CREATE_DEVICE_DEBUG fails, this could be because the
        // 'D3D11 debug layer' stopped working, indicated by the error message:
        // ===
        // D3D11CreateDevice: Flags (0x2) were specified which require the D3D11 SDK Layers for Windows 10, but they are not present on the system.
        // These flags must be removed, or the Windows 10 SDK must be installed.
        // Flags include: D3D11_CREATE_DEVICE_DEBUG
        // ===
        //
        // ...just retry with the DEBUG flag switched off
        _SAPP_ERROR(WIN32_D3D11_CREATE_DEVICE_AND_SWAPCHAIN_WITH_DEBUG_FAILED);
        create_flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        hr = D3D11CreateDeviceAndSwapChain(
            NULL,                           /* pAdapter (use default) */
            D3D_DRIVER_TYPE_HARDWARE,       /* DriverType */
            NULL,                           /* Software */
            create_flags,                   /* Flags */
            NULL,                           /* pFeatureLevels */
            0,                              /* FeatureLevels */
            D3D11_SDK_VERSION,              /* SDKVersion */
            sc_desc,                        /* pSwapChainDesc */
            &_sapp.d3d11.swap_chain,        /* ppSwapChain */
            &_sapp.d3d11.device,            /* ppDevice */
            &feature_level,                 /* pFeatureLevel */
            &_sapp.d3d11.device_context);   /* ppImmediateContext */
    }
    #endif
    SOKOL_ASSERT(SUCCEEDED(hr) && _sapp.d3d11.swap_chain && _sapp.d3d11.device && _sapp.d3d11.device_context);

    // minimize frame latency, disable Alt-Enter
    hr = _sapp_d3d11_QueryInterface(_sapp.d3d11.device, _sapp_win32_refiid(_sapp_IID_IDXGIDevice1), (void**)&_sapp.d3d11.dxgi_device);
    if (SUCCEEDED(hr) && _sapp.d3d11.dxgi_device) {
        _sapp_dxgi_SetMaximumFrameLatency(_sapp.d3d11.dxgi_device, 1);
        IDXGIAdapter* dxgi_adapter = 0;
        hr = _sapp_dxgi_GetAdapter(_sapp.d3d11.dxgi_device, &dxgi_adapter);
        if (SUCCEEDED(hr) && dxgi_adapter) {
            IDXGIFactory* dxgi_factory = 0;
            hr = _sapp_dxgi_GetParent((IDXGIObject*)dxgi_adapter, _sapp_win32_refiid(_sapp_IID_IDXGIFactory), (void**)&dxgi_factory);
            if (SUCCEEDED(hr)) {
                _sapp_dxgi_MakeWindowAssociation(dxgi_factory, _sapp.win32.hwnd, DXGI_MWA_NO_ALT_ENTER|DXGI_MWA_NO_PRINT_SCREEN);
                _SAPP_SAFE_RELEASE(dxgi_factory);
            }
            else {
                _SAPP_ERROR(WIN32_D3D11_GET_IDXGIFACTORY_FAILED);
            }
            _SAPP_SAFE_RELEASE(dxgi_adapter);
        }
        else {
            _SAPP_ERROR(WIN32_D3D11_GET_IDXGIADAPTER_FAILED);
        }
    }
    else {
        _SAPP_PANIC(WIN32_D3D11_QUERY_INTERFACE_IDXGIDEVICE1_FAILED);
    }
}

_SOKOL_PRIVATE void _sapp_d3d11_destroy_device_and_swapchain(void) {
    _SAPP_SAFE_RELEASE(_sapp.d3d11.swap_chain);
    _SAPP_SAFE_RELEASE(_sapp.d3d11.dxgi_device);
    _SAPP_SAFE_RELEASE(_sapp.d3d11.device_context);
    _SAPP_SAFE_RELEASE(_sapp.d3d11.device);
}

_SOKOL_PRIVATE void _sapp_d3d11_create_default_render_target(void) {
    SOKOL_ASSERT(0 == _sapp.d3d11.rt);
    SOKOL_ASSERT(0 == _sapp.d3d11.rtv);
    SOKOL_ASSERT(0 == _sapp.d3d11.msaa_rt);
    SOKOL_ASSERT(0 == _sapp.d3d11.msaa_rtv);
    SOKOL_ASSERT(0 == _sapp.d3d11.ds);
    SOKOL_ASSERT(0 == _sapp.d3d11.dsv);

    HRESULT hr;

    /* view for the swapchain-created framebuffer */
    hr = _sapp_dxgi_GetBuffer(_sapp.d3d11.swap_chain, 0, _sapp_win32_refiid(_sapp_IID_ID3D11Texture2D), (void**)&_sapp.d3d11.rt);
    SOKOL_ASSERT(SUCCEEDED(hr) && _sapp.d3d11.rt);
    hr = _sapp_d3d11_CreateRenderTargetView(_sapp.d3d11.device, (ID3D11Resource*)_sapp.d3d11.rt, NULL, &_sapp.d3d11.rtv);
    SOKOL_ASSERT(SUCCEEDED(hr) && _sapp.d3d11.rtv);

    /* common desc for MSAA and depth-stencil texture */
    D3D11_TEXTURE2D_DESC tex_desc;
    _sapp_clear(&tex_desc, sizeof(tex_desc));
    tex_desc.Width = (UINT)_sapp.framebuffer_width;
    tex_desc.Height = (UINT)_sapp.framebuffer_height;
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.Usage = D3D11_USAGE_DEFAULT;
    tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    tex_desc.SampleDesc.Count = (UINT) _sapp.sample_count;
    tex_desc.SampleDesc.Quality = (UINT) (_sapp.sample_count > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0);

    /* create MSAA texture and view if antialiasing requested */
    if (_sapp.sample_count > 1) {
        tex_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        hr = _sapp_d3d11_CreateTexture2D(_sapp.d3d11.device, &tex_desc, NULL, &_sapp.d3d11.msaa_rt);
        SOKOL_ASSERT(SUCCEEDED(hr) && _sapp.d3d11.msaa_rt);
        hr = _sapp_d3d11_CreateRenderTargetView(_sapp.d3d11.device, (ID3D11Resource*)_sapp.d3d11.msaa_rt, NULL, &_sapp.d3d11.msaa_rtv);
        SOKOL_ASSERT(SUCCEEDED(hr) && _sapp.d3d11.msaa_rtv);
    }

    /* texture and view for the depth-stencil-surface */
    tex_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    hr = _sapp_d3d11_CreateTexture2D(_sapp.d3d11.device, &tex_desc, NULL, &_sapp.d3d11.ds);
    SOKOL_ASSERT(SUCCEEDED(hr) && _sapp.d3d11.ds);
    hr = _sapp_d3d11_CreateDepthStencilView(_sapp.d3d11.device, (ID3D11Resource*)_sapp.d3d11.ds, NULL, &_sapp.d3d11.dsv);
    SOKOL_ASSERT(SUCCEEDED(hr) && _sapp.d3d11.dsv);
}

_SOKOL_PRIVATE void _sapp_d3d11_destroy_default_render_target(void) {
    _SAPP_SAFE_RELEASE(_sapp.d3d11.rt);
    _SAPP_SAFE_RELEASE(_sapp.d3d11.rtv);
    _SAPP_SAFE_RELEASE(_sapp.d3d11.msaa_rt);
    _SAPP_SAFE_RELEASE(_sapp.d3d11.msaa_rtv);
    _SAPP_SAFE_RELEASE(_sapp.d3d11.ds);
    _SAPP_SAFE_RELEASE(_sapp.d3d11.dsv);
}

_SOKOL_PRIVATE void _sapp_d3d11_resize_default_render_target(void) {
    if (_sapp.d3d11.swap_chain) {
        _sapp_d3d11_destroy_default_render_target();
        _sapp_dxgi_ResizeBuffers(_sapp.d3d11.swap_chain, _sapp.d3d11.swap_chain_desc.BufferCount, (UINT)_sapp.framebuffer_width, (UINT)_sapp.framebuffer_height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        _sapp_d3d11_create_default_render_target();
    }
}

_SOKOL_PRIVATE void _sapp_d3d11_present(bool do_not_wait) {
    /* do MSAA resolve if needed */
    if (_sapp.sample_count > 1) {
        SOKOL_ASSERT(_sapp.d3d11.rt);
        SOKOL_ASSERT(_sapp.d3d11.msaa_rt);
        _sapp_d3d11_ResolveSubresource(_sapp.d3d11.device_context, (ID3D11Resource*)_sapp.d3d11.rt, 0, (ID3D11Resource*)_sapp.d3d11.msaa_rt, 0, DXGI_FORMAT_B8G8R8A8_UNORM);
    }
    UINT flags = 0;
    if (_sapp.win32.is_win10_or_greater && do_not_wait) {
        /* this hack/workaround somewhat improves window-movement and -sizing
            responsiveness when rendering is controlled via WM_TIMER during window
            move and resize on NVIDIA cards on Win10 with recent drivers.
        */
        flags = DXGI_PRESENT_DO_NOT_WAIT;
    }
    _sapp_dxgi_Present(_sapp.d3d11.swap_chain, (UINT)_sapp.swap_interval, flags);
}

#endif /* SOKOL_D3D11 */

#if defined(SOKOL_GLCORE33)
_SOKOL_PRIVATE void _sapp_wgl_init(void) {
    _sapp.wgl.opengl32 = LoadLibraryA("opengl32.dll");
    if (!_sapp.wgl.opengl32) {
        _SAPP_PANIC(WIN32_LOAD_OPENGL32_DLL_FAILED);
    }
    SOKOL_ASSERT(_sapp.wgl.opengl32);
    _sapp.wgl.CreateContext = (PFN_wglCreateContext)(void*) GetProcAddress(_sapp.wgl.opengl32, "wglCreateContext");
    SOKOL_ASSERT(_sapp.wgl.CreateContext);
    _sapp.wgl.DeleteContext = (PFN_wglDeleteContext)(void*) GetProcAddress(_sapp.wgl.opengl32, "wglDeleteContext");
    SOKOL_ASSERT(_sapp.wgl.DeleteContext);
    _sapp.wgl.GetProcAddress = (PFN_wglGetProcAddress)(void*) GetProcAddress(_sapp.wgl.opengl32, "wglGetProcAddress");
    SOKOL_ASSERT(_sapp.wgl.GetProcAddress);
    _sapp.wgl.GetCurrentDC = (PFN_wglGetCurrentDC)(void*) GetProcAddress(_sapp.wgl.opengl32, "wglGetCurrentDC");
    SOKOL_ASSERT(_sapp.wgl.GetCurrentDC);
    _sapp.wgl.MakeCurrent = (PFN_wglMakeCurrent)(void*) GetProcAddress(_sapp.wgl.opengl32, "wglMakeCurrent");
    SOKOL_ASSERT(_sapp.wgl.MakeCurrent);

    _sapp.wgl.msg_hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
        L"SOKOLAPP",
        L"sokol-app message window",
        WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
        0, 0, 1, 1,
        NULL, NULL,
        GetModuleHandleW(NULL),
        NULL);
    if (!_sapp.wgl.msg_hwnd) {
        _SAPP_PANIC(WIN32_CREATE_HELPER_WINDOW_FAILED);
    }
    SOKOL_ASSERT(_sapp.wgl.msg_hwnd);
    ShowWindow(_sapp.wgl.msg_hwnd, SW_HIDE);
    MSG msg;
    while (PeekMessageW(&msg, _sapp.wgl.msg_hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    _sapp.wgl.msg_dc = GetDC(_sapp.wgl.msg_hwnd);
    if (!_sapp.wgl.msg_dc) {
        _SAPP_PANIC(WIN32_HELPER_WINDOW_GETDC_FAILED);
    }
}

_SOKOL_PRIVATE void _sapp_wgl_shutdown(void) {
    SOKOL_ASSERT(_sapp.wgl.opengl32 && _sapp.wgl.msg_hwnd);
    DestroyWindow(_sapp.wgl.msg_hwnd); _sapp.wgl.msg_hwnd = 0;
    FreeLibrary(_sapp.wgl.opengl32); _sapp.wgl.opengl32 = 0;
}

_SOKOL_PRIVATE bool _sapp_wgl_has_ext(const char* ext, const char* extensions) {
    SOKOL_ASSERT(ext && extensions);
    const char* start = extensions;
    while (true) {
        const char* where = strstr(start, ext);
        if (!where) {
            return false;
        }
        const char* terminator = where + strlen(ext);
        if ((where == start) || (*(where - 1) == ' ')) {
            if (*terminator == ' ' || *terminator == '\0') {
                break;
            }
        }
        start = terminator;
    }
    return true;
}

_SOKOL_PRIVATE bool _sapp_wgl_ext_supported(const char* ext) {
    SOKOL_ASSERT(ext);
    if (_sapp.wgl.GetExtensionsStringEXT) {
        const char* extensions = _sapp.wgl.GetExtensionsStringEXT();
        if (extensions) {
            if (_sapp_wgl_has_ext(ext, extensions)) {
                return true;
            }
        }
    }
    if (_sapp.wgl.GetExtensionsStringARB) {
        const char* extensions = _sapp.wgl.GetExtensionsStringARB(_sapp.wgl.GetCurrentDC());
        if (extensions) {
            if (_sapp_wgl_has_ext(ext, extensions)) {
                return true;
            }
        }
    }
    return false;
}

_SOKOL_PRIVATE void _sapp_wgl_load_extensions(void) {
    SOKOL_ASSERT(_sapp.wgl.msg_dc);
    PIXELFORMATDESCRIPTOR pfd;
    _sapp_clear(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    if (!SetPixelFormat(_sapp.wgl.msg_dc, ChoosePixelFormat(_sapp.wgl.msg_dc, &pfd), &pfd)) {
        _SAPP_PANIC(WIN32_DUMMY_CONTEXT_SET_PIXELFORMAT_FAILED);
    }
    HGLRC rc = _sapp.wgl.CreateContext(_sapp.wgl.msg_dc);
    if (!rc) {
        _SAPP_PANIC(WIN32_CREATE_DUMMY_CONTEXT_FAILED);
    }
    if (!_sapp.wgl.MakeCurrent(_sapp.wgl.msg_dc, rc)) {
        _SAPP_PANIC(WIN32_DUMMY_CONTEXT_MAKE_CURRENT_FAILED);
    }
    _sapp.wgl.GetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void*) _sapp.wgl.GetProcAddress("wglGetExtensionsStringEXT");
    _sapp.wgl.GetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)(void*) _sapp.wgl.GetProcAddress("wglGetExtensionsStringARB");
    _sapp.wgl.CreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)(void*) _sapp.wgl.GetProcAddress("wglCreateContextAttribsARB");
    _sapp.wgl.SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)(void*) _sapp.wgl.GetProcAddress("wglSwapIntervalEXT");
    _sapp.wgl.GetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)(void*) _sapp.wgl.GetProcAddress("wglGetPixelFormatAttribivARB");
    _sapp.wgl.arb_multisample = _sapp_wgl_ext_supported("WGL_ARB_multisample");
    _sapp.wgl.arb_create_context = _sapp_wgl_ext_supported("WGL_ARB_create_context");
    _sapp.wgl.arb_create_context_profile = _sapp_wgl_ext_supported("WGL_ARB_create_context_profile");
    _sapp.wgl.ext_swap_control = _sapp_wgl_ext_supported("WGL_EXT_swap_control");
    _sapp.wgl.arb_pixel_format = _sapp_wgl_ext_supported("WGL_ARB_pixel_format");
    _sapp.wgl.MakeCurrent(_sapp.wgl.msg_dc, 0);
    _sapp.wgl.DeleteContext(rc);
}

_SOKOL_PRIVATE int _sapp_wgl_attrib(int pixel_format, int attrib) {
    SOKOL_ASSERT(_sapp.wgl.arb_pixel_format);
    int value = 0;
    if (!_sapp.wgl.GetPixelFormatAttribivARB(_sapp.win32.dc, pixel_format, 0, 1, &attrib, &value)) {
        _SAPP_PANIC(WIN32_GET_PIXELFORMAT_ATTRIB_FAILED);
    }
    return value;
}

_SOKOL_PRIVATE void _sapp_wgl_attribiv(int pixel_format, int num_attribs, const int* attribs, int* results) {
    SOKOL_ASSERT(_sapp.wgl.arb_pixel_format);
    if (!_sapp.wgl.GetPixelFormatAttribivARB(_sapp.win32.dc, pixel_format, 0, num_attribs, attribs, results)) {
        _SAPP_PANIC(WIN32_GET_PIXELFORMAT_ATTRIB_FAILED);
    }
}

_SOKOL_PRIVATE int _sapp_wgl_find_pixel_format(void) {
    SOKOL_ASSERT(_sapp.win32.dc);
    SOKOL_ASSERT(_sapp.wgl.arb_pixel_format);

    #define _sapp_wgl_num_query_tags (12)
    const int query_tags[_sapp_wgl_num_query_tags] = {
        WGL_SUPPORT_OPENGL_ARB,
        WGL_DRAW_TO_WINDOW_ARB,
        WGL_PIXEL_TYPE_ARB,
        WGL_ACCELERATION_ARB,
        WGL_DOUBLE_BUFFER_ARB,
        WGL_RED_BITS_ARB,
        WGL_GREEN_BITS_ARB,
        WGL_BLUE_BITS_ARB,
        WGL_ALPHA_BITS_ARB,
        WGL_DEPTH_BITS_ARB,
        WGL_STENCIL_BITS_ARB,
        WGL_SAMPLES_ARB,
    };
    const int result_support_opengl_index = 0;
    const int result_draw_to_window_index = 1;
    const int result_pixel_type_index = 2;
    const int result_acceleration_index = 3;
    const int result_double_buffer_index = 4;
    const int result_red_bits_index = 5;
    const int result_green_bits_index = 6;
    const int result_blue_bits_index = 7;
    const int result_alpha_bits_index = 8;
    const int result_depth_bits_index = 9;
    const int result_stencil_bits_index = 10;
    const int result_samples_index = 11;

    int query_results[_sapp_wgl_num_query_tags] = {0};
    // Drop the last item if multisample extension is not supported.
    //  If in future querying with multiple extensions, will have to shuffle index values to have active extensions on the end.
    int query_count = _sapp_wgl_num_query_tags;
    if (!_sapp.wgl.arb_multisample) {
        query_count = _sapp_wgl_num_query_tags - 1;
    }

    int native_count = _sapp_wgl_attrib(1, WGL_NUMBER_PIXEL_FORMATS_ARB);

    _sapp_gl_fbconfig desired;
    _sapp_gl_init_fbconfig(&desired);
    desired.red_bits = 8;
    desired.green_bits = 8;
    desired.blue_bits = 8;
    desired.alpha_bits = 8;
    desired.depth_bits = 24;
    desired.stencil_bits = 8;
    desired.doublebuffer = true;
    desired.samples = (_sapp.sample_count > 1) ? _sapp.sample_count : 0;

    int pixel_format = 0;

    _sapp_gl_fbselect fbselect;
    _sapp_gl_init_fbselect(&fbselect);
    for (int i = 0; i < native_count; i++) {
        const int n = i + 1;
        _sapp_wgl_attribiv(n, query_count, query_tags, query_results);

        if (query_results[result_support_opengl_index] == 0
            || query_results[result_draw_to_window_index] == 0
            || query_results[result_pixel_type_index] != WGL_TYPE_RGBA_ARB
            || query_results[result_acceleration_index] == WGL_NO_ACCELERATION_ARB)
        {
            continue;
        }

        _sapp_gl_fbconfig u;
        _sapp_clear(&u, sizeof(u));
        u.red_bits     = query_results[result_red_bits_index];
        u.green_bits   = query_results[result_green_bits_index];
        u.blue_bits    = query_results[result_blue_bits_index];
        u.alpha_bits   = query_results[result_alpha_bits_index];
        u.depth_bits   = query_results[result_depth_bits_index];
        u.stencil_bits = query_results[result_stencil_bits_index];
        u.doublebuffer = 0 != query_results[result_double_buffer_index];
        u.samples = query_results[result_samples_index]; // NOTE: If arb_multisample is not supported  - just takes the default 0

        // Test if this pixel format is better than the previous one
        if (_sapp_gl_select_fbconfig(&fbselect, &desired, &u)) {
            pixel_format = (uintptr_t)n;

            // Early exit if matching as good as possible
            if (fbselect.best_match) {
                break;
            }
        }
    }

    return pixel_format;
}

_SOKOL_PRIVATE void _sapp_wgl_create_context(void) {
    int pixel_format = _sapp_wgl_find_pixel_format();
    if (0 == pixel_format) {
        _SAPP_PANIC(WIN32_WGL_FIND_PIXELFORMAT_FAILED);
    }
    PIXELFORMATDESCRIPTOR pfd;
    if (!DescribePixelFormat(_sapp.win32.dc, pixel_format, sizeof(pfd), &pfd)) {
        _SAPP_PANIC(WIN32_WGL_DESCRIBE_PIXELFORMAT_FAILED);
    }
    if (!SetPixelFormat(_sapp.win32.dc, pixel_format, &pfd)) {
        _SAPP_PANIC(WIN32_WGL_SET_PIXELFORMAT_FAILED);
    }
    if (!_sapp.wgl.arb_create_context) {
        _SAPP_PANIC(WIN32_WGL_ARB_CREATE_CONTEXT_REQUIRED);
    }
    if (!_sapp.wgl.arb_create_context_profile) {
        _SAPP_PANIC(WIN32_WGL_ARB_CREATE_CONTEXT_PROFILE_REQUIRED);
    }
    const int attrs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, _sapp.desc.gl_major_version,
        WGL_CONTEXT_MINOR_VERSION_ARB, _sapp.desc.gl_minor_version,
#if defined(SOKOL_DEBUG)
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
#else
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#endif
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0, 0
    };
    _sapp.wgl.gl_ctx = _sapp.wgl.CreateContextAttribsARB(_sapp.win32.dc, 0, attrs);
    if (!_sapp.wgl.gl_ctx) {
        const DWORD err = GetLastError();
        if (err == (0xc0070000 | ERROR_INVALID_VERSION_ARB)) {
            _SAPP_PANIC(WIN32_WGL_OPENGL_3_2_NOT_SUPPORTED);
        }
        else if (err == (0xc0070000 | ERROR_INVALID_PROFILE_ARB)) {
            _SAPP_PANIC(WIN32_WGL_OPENGL_PROFILE_NOT_SUPPORTED);
        }
        else if (err == (0xc0070000 | ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB)) {
            _SAPP_PANIC(WIN32_WGL_INCOMPATIBLE_DEVICE_CONTEXT);
        }
        else {
            _SAPP_PANIC(WIN32_WGL_CREATE_CONTEXT_ATTRIBS_FAILED_OTHER);
        }
    }
    _sapp.wgl.MakeCurrent(_sapp.win32.dc, _sapp.wgl.gl_ctx);
    if (_sapp.wgl.ext_swap_control) {
        /* FIXME: DwmIsCompositionEnabled() (see GLFW) */
        _sapp.wgl.SwapIntervalEXT(_sapp.swap_interval);
    }
}

_SOKOL_PRIVATE void _sapp_wgl_destroy_context(void) {
    SOKOL_ASSERT(_sapp.wgl.gl_ctx);
    _sapp.wgl.DeleteContext(_sapp.wgl.gl_ctx);
    _sapp.wgl.gl_ctx = 0;
}

_SOKOL_PRIVATE void _sapp_wgl_swap_buffers(void) {
    SOKOL_ASSERT(_sapp.win32.dc);
    /* FIXME: DwmIsCompositionEnabled? (see GLFW) */
    SwapBuffers(_sapp.win32.dc);
}
#endif /* SOKOL_GLCORE33 */

_SOKOL_PRIVATE bool _sapp_win32_wide_to_utf8(const wchar_t* src, char* dst, int dst_num_bytes) {
    SOKOL_ASSERT(src && dst && (dst_num_bytes > 1));
    _sapp_clear(dst, (size_t)dst_num_bytes);
    const int bytes_needed = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
    if (bytes_needed <= dst_num_bytes) {
        WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, dst_num_bytes, NULL, NULL);
        return true;
    }
    else {
        return false;
    }
}

/* updates current window and framebuffer size from the window's client rect, returns true if size has changed */
_SOKOL_PRIVATE bool _sapp_win32_update_dimensions(void) {
    RECT rect;
    if (GetClientRect(_sapp.win32.hwnd, &rect)) {
        float window_width = (float)(rect.right - rect.left) / _sapp.win32.dpi.window_scale;
        float window_height = (float)(rect.bottom - rect.top) / _sapp.win32.dpi.window_scale;
        _sapp.window_width = (int)roundf(window_width);
        _sapp.window_height = (int)roundf(window_height);
        int fb_width = (int)roundf(window_width * _sapp.win32.dpi.content_scale);
        int fb_height = (int)roundf(window_height * _sapp.win32.dpi.content_scale);
        /* prevent a framebuffer size of 0 when window is minimized */
        if (0 == fb_width) {
            fb_width = 1;
        }
        if (0 == fb_height) {
            fb_height = 1;
        }
        if ((fb_width != _sapp.framebuffer_width) || (fb_height != _sapp.framebuffer_height)) {
            _sapp.framebuffer_width = fb_width;
            _sapp.framebuffer_height = fb_height;
            return true;
        }
    }
    else {
        _sapp.window_width = _sapp.window_height = 1;
        _sapp.framebuffer_width = _sapp.framebuffer_height = 1;
    }
    return false;
}

_SOKOL_PRIVATE void _sapp_win32_set_fullscreen(bool fullscreen, UINT swp_flags) {
    HMONITOR monitor = MonitorFromWindow(_sapp.win32.hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO minfo;
    _sapp_clear(&minfo, sizeof(minfo));
    minfo.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &minfo);
    const RECT mr = minfo.rcMonitor;
    const int monitor_w = mr.right - mr.left;
    const int monitor_h = mr.bottom - mr.top;

    const DWORD win_ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    DWORD win_style;
    RECT rect = { 0, 0, 0, 0 };

    _sapp.fullscreen = fullscreen;
    if (!_sapp.fullscreen) {
        win_style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX;
        rect = _sapp.win32.stored_window_rect;
    }
    else {
        GetWindowRect(_sapp.win32.hwnd, &_sapp.win32.stored_window_rect);
        win_style = WS_POPUP | WS_SYSMENU | WS_VISIBLE;
        rect.left = mr.left;
        rect.top = mr.top;
        rect.right = rect.left + monitor_w;
        rect.bottom = rect.top + monitor_h;
        AdjustWindowRectEx(&rect, win_style, FALSE, win_ex_style);
    }
    const int win_w = rect.right - rect.left;
    const int win_h = rect.bottom - rect.top;
    const int win_x = rect.left;
    const int win_y = rect.top;
    SetWindowLongPtr(_sapp.win32.hwnd, GWL_STYLE, win_style);
    SetWindowPos(_sapp.win32.hwnd, HWND_TOP, win_x, win_y, win_w, win_h, swp_flags | SWP_FRAMECHANGED);
}

_SOKOL_PRIVATE void _sapp_win32_toggle_fullscreen(void) {
    _sapp_win32_set_fullscreen(!_sapp.fullscreen, SWP_SHOWWINDOW);
}

_SOKOL_PRIVATE void _sapp_win32_init_cursor(sapp_mouse_cursor cursor) {
    SOKOL_ASSERT((cursor >= 0) && (cursor < _SAPP_MOUSECURSOR_NUM));
    // NOTE: the OCR_* constants are only defined if OEMRESOURCE is defined
    // before windows.h is included, but we can't guarantee that because
    // the sokol_app.h implementation may be included with other implementations
    // in the same compilation unit
    int id = 0;
    switch (cursor) {
        case SAPP_MOUSECURSOR_ARROW:            id = 32512; break;  // OCR_NORMAL
        case SAPP_MOUSECURSOR_IBEAM:            id = 32513; break;  // OCR_IBEAM
        case SAPP_MOUSECURSOR_CROSSHAIR:        id = 32515; break;  // OCR_CROSS
        case SAPP_MOUSECURSOR_POINTING_HAND:    id = 32649; break;  // OCR_HAND
        case SAPP_MOUSECURSOR_RESIZE_EW:        id = 32644; break;  // OCR_SIZEWE
        case SAPP_MOUSECURSOR_RESIZE_NS:        id = 32645; break;  // OCR_SIZENS
        case SAPP_MOUSECURSOR_RESIZE_NWSE:      id = 32642; break;  // OCR_SIZENWSE
        case SAPP_MOUSECURSOR_RESIZE_NESW:      id = 32643; break;  // OCR_SIZENESW
        case SAPP_MOUSECURSOR_RESIZE_ALL:       id = 32646; break;  // OCR_SIZEALL
        case SAPP_MOUSECURSOR_NOT_ALLOWED:      id = 32648; break;  // OCR_NO
        default: break;
    }
    if (id != 0) {
        _sapp.win32.cursors[cursor] = (HCURSOR)LoadImageW(NULL, MAKEINTRESOURCEW(id), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE|LR_SHARED);
    }
    // fallback: default cursor
    if (0 == _sapp.win32.cursors[cursor]) {
        // 32512 => IDC_ARROW
        _sapp.win32.cursors[cursor] = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    }
    SOKOL_ASSERT(0 != _sapp.win32.cursors[cursor]);
}

_SOKOL_PRIVATE void _sapp_win32_init_cursors(void) {
    for (int i = 0; i < _SAPP_MOUSECURSOR_NUM; i++) {
        _sapp_win32_init_cursor((sapp_mouse_cursor)i);
    }
}

_SOKOL_PRIVATE bool _sapp_win32_cursor_in_content_area(void) {
    POINT pos;
    if (!GetCursorPos(&pos)) {
        return false;
    }
    if (WindowFromPoint(pos) != _sapp.win32.hwnd) {
        return false;
    }
    RECT area;
    GetClientRect(_sapp.win32.hwnd, &area);
    ClientToScreen(_sapp.win32.hwnd, (POINT*)&area.left);
    ClientToScreen(_sapp.win32.hwnd, (POINT*)&area.right);
    return PtInRect(&area, pos) == TRUE;
}

_SOKOL_PRIVATE void _sapp_win32_update_cursor(sapp_mouse_cursor cursor, bool shown, bool skip_area_test) {
    // NOTE: when called from WM_SETCURSOR, the area test would be redundant
    if (!skip_area_test) {
        if (!_sapp_win32_cursor_in_content_area()) {
            return;
        }
    }
    if (!shown) {
        SetCursor(NULL);
    }
    else {
        SOKOL_ASSERT((cursor >= 0) && (cursor < _SAPP_MOUSECURSOR_NUM));
        SOKOL_ASSERT(0 != _sapp.win32.cursors[cursor]);
        SetCursor(_sapp.win32.cursors[cursor]);
    }
}

_SOKOL_PRIVATE void _sapp_win32_capture_mouse(uint8_t btn_mask) {
    if (0 == _sapp.win32.mouse_capture_mask) {
        SetCapture(_sapp.win32.hwnd);
    }
    _sapp.win32.mouse_capture_mask |= btn_mask;
}

_SOKOL_PRIVATE void _sapp_win32_release_mouse(uint8_t btn_mask) {
    if (0 != _sapp.win32.mouse_capture_mask) {
        _sapp.win32.mouse_capture_mask &= ~btn_mask;
        if (0 == _sapp.win32.mouse_capture_mask) {
            ReleaseCapture();
        }
    }
}

_SOKOL_PRIVATE void _sapp_win32_lock_mouse(bool lock) {
    if (lock == _sapp.mouse.locked) {
        return;
    }
    _sapp.mouse.dx = 0.0f;
    _sapp.mouse.dy = 0.0f;
    _sapp.mouse.locked = lock;
    _sapp_win32_release_mouse(0xFF);
    if (_sapp.mouse.locked) {
        /* store the current mouse position, so it can be restored when unlocked */
        POINT pos;
        BOOL res = GetCursorPos(&pos);
        SOKOL_ASSERT(res); _SOKOL_UNUSED(res);
        _sapp.win32.mouse_locked_x = pos.x;
        _sapp.win32.mouse_locked_y = pos.y;

        /* while the mouse is locked, make the mouse cursor invisible and
           confine the mouse movement to a small rectangle inside our window
           (so that we don't miss any mouse up events)
        */
        RECT client_rect = {
            _sapp.win32.mouse_locked_x,
            _sapp.win32.mouse_locked_y,
            _sapp.win32.mouse_locked_x,
            _sapp.win32.mouse_locked_y
        };
        ClipCursor(&client_rect);

        /* make the mouse cursor invisible, this will stack with sapp_show_mouse() */
        ShowCursor(FALSE);

        /* enable raw input for mouse, starts sending WM_INPUT messages to WinProc (see GLFW) */
        const RAWINPUTDEVICE rid = {
            0x01,   // usUsagePage: HID_USAGE_PAGE_GENERIC
            0x02,   // usUsage: HID_USAGE_GENERIC_MOUSE
            0,      // dwFlags
            _sapp.win32.hwnd    // hwndTarget
        };
        if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
            _SAPP_ERROR(WIN32_REGISTER_RAW_INPUT_DEVICES_FAILED_MOUSE_LOCK);
        }
        /* in case the raw mouse device only supports absolute position reporting,
           we need to skip the dx/dy compution for the first WM_INPUT event
        */
        _sapp.win32.raw_input_mousepos_valid = false;
    }
    else {
        /* disable raw input for mouse */
        const RAWINPUTDEVICE rid = { 0x01, 0x02, RIDEV_REMOVE, NULL };
        if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
            _SAPP_ERROR(WIN32_REGISTER_RAW_INPUT_DEVICES_FAILED_MOUSE_UNLOCK);
        }

        /* let the mouse roam freely again */
        ClipCursor(NULL);
        ShowCursor(TRUE);

        /* restore the 'pre-locked' mouse position */
        BOOL res = SetCursorPos(_sapp.win32.mouse_locked_x, _sapp.win32.mouse_locked_y);
        SOKOL_ASSERT(res); _SOKOL_UNUSED(res);
    }
}

_SOKOL_PRIVATE bool _sapp_win32_update_monitor(void) {
    const HMONITOR cur_monitor = MonitorFromWindow(_sapp.win32.hwnd, MONITOR_DEFAULTTONULL);
    if (cur_monitor != _sapp.win32.hmonitor) {
        _sapp.win32.hmonitor = cur_monitor;
        return true;
    }
    else {
        return false;
    }
}

_SOKOL_PRIVATE uint32_t _sapp_win32_mods(void) {
    uint32_t mods = 0;
    if (GetKeyState(VK_SHIFT) & (1<<15)) {
        mods |= SAPP_MODIFIER_SHIFT;
    }
    if (GetKeyState(VK_CONTROL) & (1<<15)) {
        mods |= SAPP_MODIFIER_CTRL;
    }
    if (GetKeyState(VK_MENU) & (1<<15)) {
        mods |= SAPP_MODIFIER_ALT;
    }
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & (1<<15)) {
        mods |= SAPP_MODIFIER_SUPER;
    }
    const bool swapped = (TRUE == GetSystemMetrics(SM_SWAPBUTTON));
    if (GetAsyncKeyState(VK_LBUTTON)) {
        mods |= swapped ? SAPP_MODIFIER_RMB : SAPP_MODIFIER_LMB;
    }
    if (GetAsyncKeyState(VK_RBUTTON)) {
        mods |= swapped ? SAPP_MODIFIER_LMB : SAPP_MODIFIER_RMB;
    }
    if (GetAsyncKeyState(VK_MBUTTON)) {
        mods |= SAPP_MODIFIER_MMB;
    }
    return mods;
}

_SOKOL_PRIVATE void _sapp_win32_mouse_update(LPARAM lParam) {
    if (!_sapp.mouse.locked) {
        const float new_x  = (float)GET_X_LPARAM(lParam) * _sapp.win32.dpi.mouse_scale;
        const float new_y = (float)GET_Y_LPARAM(lParam) * _sapp.win32.dpi.mouse_scale;
        if (_sapp.mouse.pos_valid) {
            // don't update dx/dy in the very first event
            _sapp.mouse.dx = new_x - _sapp.mouse.x;
            _sapp.mouse.dy = new_y - _sapp.mouse.y;
        }
        _sapp.mouse.x = new_x;
        _sapp.mouse.y = new_y;
        _sapp.mouse.pos_valid = true;
    }
}

_SOKOL_PRIVATE void _sapp_win32_mouse_event(sapp_event_type type, sapp_mousebutton btn) {
    if (_sapp_events_enabled()) {
        _sapp_init_event(type);
        _sapp.event.modifiers = _sapp_win32_mods();
        _sapp.event.mouse_button = btn;
        _sapp_call_event(&_sapp.event);
    }
}

_SOKOL_PRIVATE void _sapp_win32_scroll_event(float x, float y) {
    if (_sapp_events_enabled()) {
        _sapp_init_event(SAPP_EVENTTYPE_MOUSE_SCROLL);
        _sapp.event.modifiers = _sapp_win32_mods();
        _sapp.event.scroll_x = -x / 30.0f;
        _sapp.event.scroll_y = y / 30.0f;
        _sapp_call_event(&_sapp.event);
    }
}

_SOKOL_PRIVATE void _sapp_win32_key_event(sapp_event_type type, int vk, bool repeat) {
    if (_sapp_events_enabled() && (vk < SAPP_MAX_KEYCODES)) {
        _sapp_init_event(type);
        _sapp.event.modifiers = _sapp_win32_mods();
        _sapp.event.key_code = _sapp.keycodes[vk];
        _sapp.event.key_repeat = repeat;
        _sapp_call_event(&_sapp.event);
        /* check if a CLIPBOARD_PASTED event must be sent too */
        if (_sapp.clipboard.enabled &&
            (type == SAPP_EVENTTYPE_KEY_DOWN) &&
            (_sapp.event.modifiers == SAPP_MODIFIER_CTRL) &&
            (_sapp.event.key_code == SAPP_KEYCODE_V))
        {
            _sapp_init_event(SAPP_EVENTTYPE_CLIPBOARD_PASTED);
            _sapp_call_event(&_sapp.event);
        }
    }
}

_SOKOL_PRIVATE void _sapp_win32_char_event(uint32_t c, bool repeat) {
    if (_sapp_events_enabled() && (c >= 32)) {
        _sapp_init_event(SAPP_EVENTTYPE_CHAR);
        _sapp.event.modifiers = _sapp_win32_mods();
        _sapp.event.char_code = c;
        _sapp.event.key_repeat = repeat;
        _sapp_call_event(&_sapp.event);
    }
}

_SOKOL_PRIVATE void _sapp_win32_dpi_changed(HWND hWnd, LPRECT proposed_win_rect) {
    /* called on WM_DPICHANGED, which will only be sent to the application
        if sapp_desc.high_dpi is true and the Windows version is recent enough
        to support DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
    */
    SOKOL_ASSERT(_sapp.desc.high_dpi);
    HINSTANCE user32 = LoadLibraryA("user32.dll");
    if (!user32) {
        return;
    }
    typedef UINT(WINAPI * GETDPIFORWINDOW_T)(HWND hwnd);
    GETDPIFORWINDOW_T fn_getdpiforwindow = (GETDPIFORWINDOW_T)(void*)GetProcAddress(user32, "GetDpiForWindow");
    if (fn_getdpiforwindow) {
        UINT dpix = fn_getdpiforwindow(_sapp.win32.hwnd);
        // NOTE: for high-dpi apps, mouse_scale remains one
        _sapp.win32.dpi.window_scale = (float)dpix / 96.0f;
        _sapp.win32.dpi.content_scale = _sapp.win32.dpi.window_scale;
        _sapp.dpi_scale = _sapp.win32.dpi.window_scale;
        SetWindowPos(hWnd, 0,
            proposed_win_rect->left,
            proposed_win_rect->top,
            proposed_win_rect->right - proposed_win_rect->left,
            proposed_win_rect->bottom - proposed_win_rect->top,
            SWP_NOZORDER | SWP_NOACTIVATE);
    }
    FreeLibrary(user32);
}

_SOKOL_PRIVATE void _sapp_win32_files_dropped(HDROP hdrop) {
    if (!_sapp.drop.enabled) {
        return;
    }
    _sapp_clear_drop_buffer();
    bool drop_failed = false;
    const int count = (int) DragQueryFileW(hdrop, 0xffffffff, NULL, 0);
    _sapp.drop.num_files = (count > _sapp.drop.max_files) ? _sapp.drop.max_files : count;
    for (UINT i = 0;  i < (UINT)_sapp.drop.num_files;  i++) {
        const UINT num_chars = DragQueryFileW(hdrop, i, NULL, 0) + 1;
        WCHAR* buffer = (WCHAR*) _sapp_malloc_clear(num_chars * sizeof(WCHAR));
        DragQueryFileW(hdrop, i, buffer, num_chars);
        if (!_sapp_win32_wide_to_utf8(buffer, _sapp_dropped_file_path_ptr((int)i), _sapp.drop.max_path_length)) {
            _SAPP_ERROR(DROPPED_FILE_PATH_TOO_LONG);
            drop_failed = true;
        }
        _sapp_free(buffer);
    }
    DragFinish(hdrop);
    if (!drop_failed) {
        if (_sapp_events_enabled()) {
            _sapp_init_event(SAPP_EVENTTYPE_FILES_DROPPED);
            _sapp.event.modifiers = _sapp_win32_mods();
            _sapp_call_event(&_sapp.event);
        }
    }
    else {
        _sapp_clear_drop_buffer();
        _sapp.drop.num_files = 0;
    }
}

_SOKOL_PRIVATE void _sapp_win32_timing_measure(void) {
    #if defined(SOKOL_D3D11)
        // on D3D11, use the more precise DXGI timestamp
        if (_sapp.d3d11.use_dxgi_frame_stats) {
            DXGI_FRAME_STATISTICS dxgi_stats;
            _sapp_clear(&dxgi_stats, sizeof(dxgi_stats));
            HRESULT hr = _sapp_dxgi_GetFrameStatistics(_sapp.d3d11.swap_chain, &dxgi_stats);
            if (SUCCEEDED(hr)) {
                if (dxgi_stats.SyncRefreshCount != _sapp.d3d11.sync_refresh_count) {
                    if ((_sapp.d3d11.sync_refresh_count + 1) != dxgi_stats.SyncRefreshCount) {
                        _sapp_timing_discontinuity(&_sapp.timing);
                    }
                    _sapp.d3d11.sync_refresh_count = dxgi_stats.SyncRefreshCount;
                    LARGE_INTEGER qpc = dxgi_stats.SyncQPCTime;
                    const uint64_t now = (uint64_t)_sapp_int64_muldiv(qpc.QuadPart - _sapp.timing.timestamp.win.start.QuadPart, 1000000000, _sapp.timing.timestamp.win.freq.QuadPart);
                    _sapp_timing_external(&_sapp.timing, (double)now / 1000000000.0);
                }
                return;
            }
        }
        // fallback if swap model isn't "flip-discard" or GetFrameStatistics failed for another reason
        _sapp_timing_measure(&_sapp.timing);
    #endif
    #if defined(SOKOL_GLCORE33)
        _sapp_timing_measure(&_sapp.timing);
    #endif
}

_SOKOL_PRIVATE LRESULT CALLBACK _sapp_win32_wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (!_sapp.win32.in_create_window) {
        switch (uMsg) {
            case WM_CLOSE:
                /* only give user a chance to intervene when sapp_quit() wasn't already called */
                if (!_sapp.quit_ordered) {
                    /* if window should be closed and event handling is enabled, give user code
                        a change to intervene via sapp_cancel_quit()
                    */
                    _sapp.quit_requested = true;
                    _sapp_win32_app_event(SAPP_EVENTTYPE_QUIT_REQUESTED);
                    /* if user code hasn't intervened, quit the app */
                    if (_sapp.quit_requested) {
                        _sapp.quit_ordered = true;
                    }
                }
                if (_sapp.quit_ordered) {
                    PostQuitMessage(0);
                }
                return 0;
            case WM_SYSCOMMAND:
                switch (wParam & 0xFFF0) {
                    case SC_SCREENSAVE:
                    case SC_MONITORPOWER:
                        if (_sapp.fullscreen) {
                            /* disable screen saver and blanking in fullscreen mode */
                            return 0;
                        }
                        break;
                    case SC_KEYMENU:
                        /* user trying to access menu via ALT */
                        return 0;
                }
                break;
            case WM_ERASEBKGND:
                return 1;
            case WM_SIZE:
                {
                    const bool iconified = wParam == SIZE_MINIMIZED;
                    if (iconified != _sapp.win32.iconified) {
                        _sapp.win32.iconified = iconified;
                        if (iconified) {
                            _sapp_win32_app_event(SAPP_EVENTTYPE_ICONIFIED);
                        }
                        else {
                            _sapp_win32_app_event(SAPP_EVENTTYPE_RESTORED);
                        }
                    }
                }
                break;
            case WM_SETFOCUS:
                _sapp_win32_app_event(SAPP_EVENTTYPE_FOCUSED);
                break;
            case WM_KILLFOCUS:
                /* if focus is lost for any reason, and we're in mouse locked mode, disable mouse lock */
                if (_sapp.mouse.locked) {
                    _sapp_win32_lock_mouse(false);
                }
                _sapp_win32_app_event(SAPP_EVENTTYPE_UNFOCUSED);
                break;
            case WM_SETCURSOR:
                if (LOWORD(lParam) == HTCLIENT) {
                    _sapp_win32_update_cursor(_sapp.mouse.current_cursor, _sapp.mouse.shown, true);
                    return TRUE;
                }
                break;
            case WM_DPICHANGED:
            {
                /* Update window's DPI and size if its moved to another monitor with a different DPI
                   Only sent if DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 is used.
                */
                _sapp_win32_dpi_changed(hWnd, (LPRECT)lParam);
                break;
            }
            case WM_LBUTTONDOWN:
                _sapp_win32_mouse_update(lParam);
                _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_DOWN, SAPP_MOUSEBUTTON_LEFT);
                _sapp_win32_capture_mouse(1<<SAPP_MOUSEBUTTON_LEFT);
                break;
            case WM_RBUTTONDOWN:
                _sapp_win32_mouse_update(lParam);
                _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_DOWN, SAPP_MOUSEBUTTON_RIGHT);
                _sapp_win32_capture_mouse(1<<SAPP_MOUSEBUTTON_RIGHT);
                break;
            case WM_MBUTTONDOWN:
                _sapp_win32_mouse_update(lParam);
                _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_DOWN, SAPP_MOUSEBUTTON_MIDDLE);
                _sapp_win32_capture_mouse(1<<SAPP_MOUSEBUTTON_MIDDLE);
                break;
            case WM_LBUTTONUP:
                _sapp_win32_mouse_update(lParam);
                _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_UP, SAPP_MOUSEBUTTON_LEFT);
                _sapp_win32_release_mouse(1<<SAPP_MOUSEBUTTON_LEFT);
                break;
            case WM_RBUTTONUP:
                _sapp_win32_mouse_update(lParam);
                _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_UP, SAPP_MOUSEBUTTON_RIGHT);
                _sapp_win32_release_mouse(1<<SAPP_MOUSEBUTTON_RIGHT);
                break;
            case WM_MBUTTONUP:
                _sapp_win32_mouse_update(lParam);
                _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_UP, SAPP_MOUSEBUTTON_MIDDLE);
                _sapp_win32_release_mouse(1<<SAPP_MOUSEBUTTON_MIDDLE);
                break;
            case WM_MOUSEMOVE:
                if (!_sapp.mouse.locked) {
                    _sapp_win32_mouse_update(lParam);
                    if (!_sapp.win32.mouse_tracked) {
                        _sapp.win32.mouse_tracked = true;
                        TRACKMOUSEEVENT tme;
                        _sapp_clear(&tme, sizeof(tme));
                        tme.cbSize = sizeof(tme);
                        tme.dwFlags = TME_LEAVE;
                        tme.hwndTrack = _sapp.win32.hwnd;
                        TrackMouseEvent(&tme);
                        _sapp.mouse.dx = 0.0f;
                        _sapp.mouse.dy = 0.0f;
                        _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_ENTER, SAPP_MOUSEBUTTON_INVALID);
                    }
                    _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_MOVE, SAPP_MOUSEBUTTON_INVALID);
                }
                break;
            case WM_INPUT:
                /* raw mouse input during mouse-lock */
                if (_sapp.mouse.locked) {
                    HRAWINPUT ri = (HRAWINPUT) lParam;
                    UINT size = sizeof(_sapp.win32.raw_input_data);
                    // see: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getrawinputdata
                    if ((UINT)-1 == GetRawInputData(ri, RID_INPUT, &_sapp.win32.raw_input_data, &size, sizeof(RAWINPUTHEADER))) {
                        _SAPP_ERROR(WIN32_GET_RAW_INPUT_DATA_FAILED);
                        break;
                    }
                    const RAWINPUT* raw_mouse_data = (const RAWINPUT*) &_sapp.win32.raw_input_data;
                    if (raw_mouse_data->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
                        /* mouse only reports absolute position
                           NOTE: This code is untested and will most likely behave wrong in Remote Desktop sessions.
                           (such remote desktop sessions are setting the MOUSE_MOVE_ABSOLUTE flag).
                           See: https://github.com/floooh/sokol/issues/806 and
                           https://github.com/microsoft/DirectXTK/commit/ef56b63f3739381e451f7a5a5bd2c9779d2a7555)
                        */
                        LONG new_x = raw_mouse_data->data.mouse.lLastX;
                        LONG new_y = raw_mouse_data->data.mouse.lLastY;
                        if (_sapp.win32.raw_input_mousepos_valid) {
                            _sapp.mouse.dx = (float) (new_x - _sapp.win32.raw_input_mousepos_x);
                            _sapp.mouse.dy = (float) (new_y - _sapp.win32.raw_input_mousepos_y);
                        }
                        _sapp.win32.raw_input_mousepos_x = new_x;
                        _sapp.win32.raw_input_mousepos_y = new_y;
                        _sapp.win32.raw_input_mousepos_valid = true;
                    }
                    else {
                        /* mouse reports movement delta (this seems to be the common case) */
                        _sapp.mouse.dx = (float) raw_mouse_data->data.mouse.lLastX;
                        _sapp.mouse.dy = (float) raw_mouse_data->data.mouse.lLastY;
                    }
                    _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_MOVE, SAPP_MOUSEBUTTON_INVALID);
                }
                break;

            case WM_MOUSELEAVE:
                if (!_sapp.mouse.locked) {
                    _sapp.mouse.dx = 0.0f;
                    _sapp.mouse.dy = 0.0f;
                    _sapp.win32.mouse_tracked = false;
                    _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_LEAVE, SAPP_MOUSEBUTTON_INVALID);
                }
                break;
            case WM_MOUSEWHEEL:
                _sapp_win32_scroll_event(0.0f, (float)((SHORT)HIWORD(wParam)));
                break;
            case WM_MOUSEHWHEEL:
                _sapp_win32_scroll_event((float)((SHORT)HIWORD(wParam)), 0.0f);
                break;
            case WM_CHAR:
                _sapp_win32_char_event((uint32_t)wParam, !!(lParam&0x40000000));
                break;
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                _sapp_win32_key_event(SAPP_EVENTTYPE_KEY_DOWN, (int)(HIWORD(lParam)&0x1FF), !!(lParam&0x40000000));
                break;
            case WM_KEYUP:
            case WM_SYSKEYUP:
                _sapp_win32_key_event(SAPP_EVENTTYPE_KEY_UP, (int)(HIWORD(lParam)&0x1FF), false);
                break;
            case WM_ENTERSIZEMOVE:
                SetTimer(_sapp.win32.hwnd, 1, USER_TIMER_MINIMUM, NULL);
                break;
            case WM_EXITSIZEMOVE:
                KillTimer(_sapp.win32.hwnd, 1);
                break;
            case WM_TIMER:
                _sapp_win32_timing_measure();
                _sapp_frame();
                #if defined(SOKOL_D3D11)
                    // present with DXGI_PRESENT_DO_NOT_WAIT
                    _sapp_d3d11_present(true);
                #endif
                #if defined(SOKOL_GLCORE33)
                    _sapp_wgl_swap_buffers();
                #endif
                /* NOTE: resizing the swap-chain during resize leads to a substantial
                   memory spike (hundreds of megabytes for a few seconds).

                if (_sapp_win32_update_dimensions()) {
                    #if defined(SOKOL_D3D11)
                    _sapp_d3d11_resize_default_render_target();
                    #endif
                    _sapp_win32_app_event(SAPP_EVENTTYPE_RESIZED);
                }
                */
                break;
            case WM_NCLBUTTONDOWN:
                /* workaround for half-second pause when starting to move window
                    see: https://gamedev.net/forums/topic/672094-keeping-things-moving-during-win32-moveresize-events/5254386/
                */
                if (SendMessage(_sapp.win32.hwnd, WM_NCHITTEST, wParam, lParam) == HTCAPTION) {
                    POINT point;
                    GetCursorPos(&point);
                    ScreenToClient(_sapp.win32.hwnd, &point);
                    PostMessage(_sapp.win32.hwnd, WM_MOUSEMOVE, 0, ((uint32_t)point.x)|(((uint32_t)point.y) << 16));
                }
                break;
            case WM_DROPFILES:
                _sapp_win32_files_dropped((HDROP)wParam);
                break;
            case WM_DISPLAYCHANGE:
                // refresh rate might have changed
                _sapp_timing_reset(&_sapp.timing);
                break;

            default:
                break;
        }
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

_SOKOL_PRIVATE void _sapp_win32_create_window(void) {
    WNDCLASSW wndclassw;
    _sapp_clear(&wndclassw, sizeof(wndclassw));
    wndclassw.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclassw.lpfnWndProc = (WNDPROC) _sapp_win32_wndproc;
    wndclassw.hInstance = GetModuleHandleW(NULL);
    wndclassw.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclassw.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wndclassw.lpszClassName = L"SOKOLAPP";
    RegisterClassW(&wndclassw);

    /* NOTE: regardless whether fullscreen is requested or not, a regular
       windowed-mode window will always be created first (however in hidden
       mode, so that no windowed-mode window pops up before the fullscreen window)
    */
    const DWORD win_ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    RECT rect = { 0, 0, 0, 0 };
    DWORD win_style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX;
    rect.right = (int) ((float)_sapp.window_width * _sapp.win32.dpi.window_scale);
    rect.bottom = (int) ((float)_sapp.window_height * _sapp.win32.dpi.window_scale);
    const bool use_default_width = 0 == _sapp.window_width;
    const bool use_default_height = 0 == _sapp.window_height;
    AdjustWindowRectEx(&rect, win_style, FALSE, win_ex_style);
    const int win_width = rect.right - rect.left;
    const int win_height = rect.bottom - rect.top;
    _sapp.win32.in_create_window = true;
    _sapp.win32.hwnd = CreateWindowExW(
        win_ex_style,               // dwExStyle
        L"SOKOLAPP",                // lpClassName
        _sapp.window_title_wide,    // lpWindowName
        win_style,                  // dwStyle
        CW_USEDEFAULT,              // X
        SW_HIDE,                    // Y (NOTE: CW_USEDEFAULT is not used for position here, but internally calls ShowWindow!
        use_default_width ? CW_USEDEFAULT : win_width, // nWidth
        use_default_height ? CW_USEDEFAULT : win_height, // nHeight (NOTE: if width is CW_USEDEFAULT, height is actually ignored)
        NULL,                       // hWndParent
        NULL,                       // hMenu
        GetModuleHandle(NULL),      // hInstance
        NULL);                      // lParam
    _sapp.win32.in_create_window = false;
    _sapp.win32.dc = GetDC(_sapp.win32.hwnd);
    _sapp.win32.hmonitor = MonitorFromWindow(_sapp.win32.hwnd, MONITOR_DEFAULTTONULL);
    SOKOL_ASSERT(_sapp.win32.dc);

    /* this will get the actual windowed-mode window size, if fullscreen
       is requested, the set_fullscreen function will then capture the
       current window rectangle, which then might be used later to
       restore the window position when switching back to windowed
    */
    _sapp_win32_update_dimensions();
    if (_sapp.fullscreen) {
        _sapp_win32_set_fullscreen(_sapp.fullscreen, SWP_HIDEWINDOW);
        _sapp_win32_update_dimensions();
    }
    ShowWindow(_sapp.win32.hwnd, SW_SHOW);
    DragAcceptFiles(_sapp.win32.hwnd, 1);
}

_SOKOL_PRIVATE void _sapp_win32_destroy_window(void) {
    DestroyWindow(_sapp.win32.hwnd); _sapp.win32.hwnd = 0;
    UnregisterClassW(L"SOKOLAPP", GetModuleHandleW(NULL));
}

_SOKOL_PRIVATE void _sapp_win32_destroy_icons(void) {
    if (_sapp.win32.big_icon) {
        DestroyIcon(_sapp.win32.big_icon);
        _sapp.win32.big_icon = 0;
    }
    if (_sapp.win32.small_icon) {
        DestroyIcon(_sapp.win32.small_icon);
        _sapp.win32.small_icon = 0;
    }
}

_SOKOL_PRIVATE void _sapp_win32_init_console(void) {
    if (_sapp.desc.win32_console_create || _sapp.desc.win32_console_attach) {
        BOOL con_valid = FALSE;
        if (_sapp.desc.win32_console_create) {
            con_valid = AllocConsole();
        }
        else if (_sapp.desc.win32_console_attach) {
            con_valid = AttachConsole(ATTACH_PARENT_PROCESS);
        }
        if (con_valid) {
            FILE* res_fp = 0;
            errno_t err;
            err = freopen_s(&res_fp, "CON", "w", stdout);
            (void)err;
            err = freopen_s(&res_fp, "CON", "w", stderr);
            (void)err;
        }
    }
    if (_sapp.desc.win32_console_utf8) {
        _sapp.win32.orig_codepage = GetConsoleOutputCP();
        SetConsoleOutputCP(CP_UTF8);
    }
}

_SOKOL_PRIVATE void _sapp_win32_restore_console(void) {
    if (_sapp.desc.win32_console_utf8) {
        SetConsoleOutputCP(_sapp.win32.orig_codepage);
    }
}

_SOKOL_PRIVATE void _sapp_win32_init_dpi(void) {

    DECLARE_HANDLE(DPI_AWARENESS_CONTEXT_T);
    typedef BOOL(WINAPI * SETPROCESSDPIAWARE_T)(void);
    typedef bool (WINAPI * SETPROCESSDPIAWARENESSCONTEXT_T)(DPI_AWARENESS_CONTEXT_T); // since Windows 10, version 1703
    typedef HRESULT(WINAPI * SETPROCESSDPIAWARENESS_T)(PROCESS_DPI_AWARENESS);
    typedef HRESULT(WINAPI * GETDPIFORMONITOR_T)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);

    SETPROCESSDPIAWARE_T fn_setprocessdpiaware = 0;
    SETPROCESSDPIAWARENESS_T fn_setprocessdpiawareness = 0;
    GETDPIFORMONITOR_T fn_getdpiformonitor = 0;
    SETPROCESSDPIAWARENESSCONTEXT_T fn_setprocessdpiawarenesscontext =0;

    HINSTANCE user32 = LoadLibraryA("user32.dll");
    if (user32) {
        fn_setprocessdpiaware = (SETPROCESSDPIAWARE_T)(void*) GetProcAddress(user32, "SetProcessDPIAware");
        fn_setprocessdpiawarenesscontext = (SETPROCESSDPIAWARENESSCONTEXT_T)(void*) GetProcAddress(user32, "SetProcessDpiAwarenessContext");
    }
    HINSTANCE shcore = LoadLibraryA("shcore.dll");
    if (shcore) {
        fn_setprocessdpiawareness = (SETPROCESSDPIAWARENESS_T)(void*) GetProcAddress(shcore, "SetProcessDpiAwareness");
        fn_getdpiformonitor = (GETDPIFORMONITOR_T)(void*) GetProcAddress(shcore, "GetDpiForMonitor");
    }
    /*
        NOTE on SetProcessDpiAware() vs SetProcessDpiAwareness() vs SetProcessDpiAwarenessContext():

        These are different attempts to get DPI handling on Windows right, from oldest
        to newest. SetProcessDpiAwarenessContext() is required for the new
        DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 method.
    */
    if (fn_setprocessdpiawareness) {
        if (_sapp.desc.high_dpi) {
            /* app requests HighDPI rendering, first try the Win10 Creator Update per-monitor-dpi awareness,
               if that fails, fall back to system-dpi-awareness
            */
            _sapp.win32.dpi.aware = true;
            DPI_AWARENESS_CONTEXT_T per_monitor_aware_v2 = (DPI_AWARENESS_CONTEXT_T)-4;
            if (!(fn_setprocessdpiawarenesscontext && fn_setprocessdpiawarenesscontext(per_monitor_aware_v2))) {
                // fallback to system-dpi-aware
                fn_setprocessdpiawareness(PROCESS_SYSTEM_DPI_AWARE);
            }
        }
        else {
            /* if the app didn't request HighDPI rendering, let Windows do the upscaling */
            _sapp.win32.dpi.aware = false;
            fn_setprocessdpiawareness(PROCESS_DPI_UNAWARE);
        }
    }
    else if (fn_setprocessdpiaware) {
        // fallback for Windows 7
        _sapp.win32.dpi.aware = true;
        fn_setprocessdpiaware();
    }
    /* get dpi scale factor for main monitor */
    if (fn_getdpiformonitor && _sapp.win32.dpi.aware) {
        POINT pt = { 1, 1 };
        HMONITOR hm = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
        UINT dpix, dpiy;
        HRESULT hr = fn_getdpiformonitor(hm, MDT_EFFECTIVE_DPI, &dpix, &dpiy);
        _SOKOL_UNUSED(hr);
        SOKOL_ASSERT(SUCCEEDED(hr));
        /* clamp window scale to an integer factor */
        _sapp.win32.dpi.window_scale = (float)dpix / 96.0f;
    }
    else {
        _sapp.win32.dpi.window_scale = 1.0f;
    }
    if (_sapp.desc.high_dpi) {
        _sapp.win32.dpi.content_scale = _sapp.win32.dpi.window_scale;
        _sapp.win32.dpi.mouse_scale = 1.0f;
    }
    else {
        _sapp.win32.dpi.content_scale = 1.0f;
        _sapp.win32.dpi.mouse_scale = 1.0f / _sapp.win32.dpi.window_scale;
    }
    _sapp.dpi_scale = _sapp.win32.dpi.content_scale;
    if (user32) {
        FreeLibrary(user32);
    }
    if (shcore) {
        FreeLibrary(shcore);
    }
}

_SOKOL_PRIVATE bool _sapp_win32_set_clipboard_string(const char* str) {
    SOKOL_ASSERT(str);
    SOKOL_ASSERT(_sapp.win32.hwnd);
    SOKOL_ASSERT(_sapp.clipboard.enabled && (_sapp.clipboard.buf_size > 0));

    if (!OpenClipboard(_sapp.win32.hwnd)) {
        return false;
    }

    HANDLE object = 0;
    wchar_t* wchar_buf = 0;

    const SIZE_T wchar_buf_size = (SIZE_T)_sapp.clipboard.buf_size * sizeof(wchar_t);
    object = GlobalAlloc(GMEM_MOVEABLE, wchar_buf_size);
    if (NULL == object) {
        goto error;
    }
    wchar_buf = (wchar_t*) GlobalLock(object);
    if (NULL == wchar_buf) {
        goto error;
    }
    if (!_sapp_win32_utf8_to_wide(str, wchar_buf, (int)wchar_buf_size)) {
        goto error;
    }
    GlobalUnlock(object);
    wchar_buf = 0;
    EmptyClipboard();
    // NOTE: when successful, SetClipboardData() takes ownership of memory object!
    if (NULL == SetClipboardData(CF_UNICODETEXT, object)) {
        goto error;
    }
    CloseClipboard();
    return true;

error:
    if (wchar_buf) {
        GlobalUnlock(object);
    }
    if (object) {
        GlobalFree(object);
    }
    CloseClipboard();
    return false;
}

_SOKOL_PRIVATE const char* _sapp_win32_get_clipboard_string(void) {
    SOKOL_ASSERT(_sapp.clipboard.enabled && _sapp.clipboard.buffer);
    SOKOL_ASSERT(_sapp.win32.hwnd);
    if (!OpenClipboard(_sapp.win32.hwnd)) {
        /* silently ignore any errors and just return the current
           content of the local clipboard buffer
        */
        return _sapp.clipboard.buffer;
    }
    HANDLE object = GetClipboardData(CF_UNICODETEXT);
    if (!object) {
        CloseClipboard();
        return _sapp.clipboard.buffer;
    }
    const wchar_t* wchar_buf = (const wchar_t*) GlobalLock(object);
    if (!wchar_buf) {
        CloseClipboard();
        return _sapp.clipboard.buffer;
    }
    if (!_sapp_win32_wide_to_utf8(wchar_buf, _sapp.clipboard.buffer, _sapp.clipboard.buf_size)) {
        _SAPP_ERROR(CLIPBOARD_STRING_TOO_BIG);
    }
    GlobalUnlock(object);
    CloseClipboard();
    return _sapp.clipboard.buffer;
}

_SOKOL_PRIVATE void _sapp_win32_update_window_title(void) {
    _sapp_win32_utf8_to_wide(_sapp.window_title, _sapp.window_title_wide, sizeof(_sapp.window_title_wide));
    SetWindowTextW(_sapp.win32.hwnd, _sapp.window_title_wide);
}

_SOKOL_PRIVATE HICON _sapp_win32_create_icon_from_image(const sapp_image_desc* desc) {
    BITMAPV5HEADER bi;
    _sapp_clear(&bi, sizeof(bi));
    bi.bV5Size = sizeof(bi);
    bi.bV5Width = desc->width;
    bi.bV5Height = -desc->height;   // NOTE the '-' here to indicate that origin is top-left
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;

    uint8_t* target = 0;
    const uint8_t* source = (const uint8_t*)desc->pixels.ptr;

    HDC dc = GetDC(NULL);
    HBITMAP color = CreateDIBSection(dc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&target, NULL, (DWORD)0);
    ReleaseDC(NULL, dc);
    if (0 == color) {
        return NULL;
    }
    SOKOL_ASSERT(target);

    HBITMAP mask = CreateBitmap(desc->width, desc->height, 1, 1, NULL);
    if (0 == mask) {
        DeleteObject(color);
        return NULL;
    }

    for (int i = 0; i < (desc->width*desc->height); i++) {
        target[0] = source[2];
        target[1] = source[1];
        target[2] = source[0];
        target[3] = source[3];
        target += 4;
        source += 4;
    }

    ICONINFO icon_info;
    _sapp_clear(&icon_info, sizeof(icon_info));
    icon_info.fIcon = true;
    icon_info.xHotspot = 0;
    icon_info.yHotspot = 0;
    icon_info.hbmMask = mask;
    icon_info.hbmColor = color;
    HICON icon_handle = CreateIconIndirect(&icon_info);
    DeleteObject(color);
    DeleteObject(mask);

    return icon_handle;
}

_SOKOL_PRIVATE void _sapp_win32_set_icon(const sapp_icon_desc* icon_desc, int num_images) {
    SOKOL_ASSERT((num_images > 0) && (num_images <= SAPP_MAX_ICONIMAGES));

    int big_img_index = _sapp_image_bestmatch(icon_desc->images, num_images, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
    int sml_img_index = _sapp_image_bestmatch(icon_desc->images, num_images, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));
    HICON big_icon = _sapp_win32_create_icon_from_image(&icon_desc->images[big_img_index]);
    HICON sml_icon = _sapp_win32_create_icon_from_image(&icon_desc->images[sml_img_index]);

    // if icon creation or lookup has failed for some reason, leave the currently set icon untouched
    if (0 != big_icon) {
        SendMessage(_sapp.win32.hwnd, WM_SETICON, ICON_BIG, (LPARAM) big_icon);
        if (0 != _sapp.win32.big_icon) {
            DestroyIcon(_sapp.win32.big_icon);
        }
        _sapp.win32.big_icon = big_icon;
    }
    if (0 != sml_icon) {
        SendMessage(_sapp.win32.hwnd, WM_SETICON, ICON_SMALL, (LPARAM) sml_icon);
        if (0 != _sapp.win32.small_icon) {
            DestroyIcon(_sapp.win32.small_icon);
        }
        _sapp.win32.small_icon = sml_icon;
    }
}

/* don't laugh, but this seems to be the easiest and most robust
   way to check if we're running on Win10

   From: https://github.com/videolan/vlc/blob/232fb13b0d6110c4d1b683cde24cf9a7f2c5c2ea/modules/video_output/win32/d3d11_swapchain.c#L263
*/
_SOKOL_PRIVATE bool _sapp_win32_is_win10_or_greater(void) {
    HMODULE h = GetModuleHandleW(L"kernel32.dll");
    if (NULL != h) {
        return (NULL != GetProcAddress(h, "GetSystemCpuSetInformation"));
    }
    else {
        return false;
    }
}

_SOKOL_PRIVATE void _sapp_win32_run(const sapp_desc* desc) {
    _sapp_init_state(desc);
    _sapp_win32_init_console();
    _sapp.win32.is_win10_or_greater = _sapp_win32_is_win10_or_greater();
    _sapp_win32_init_keytable();
    _sapp_win32_utf8_to_wide(_sapp.window_title, _sapp.window_title_wide, sizeof(_sapp.window_title_wide));
    _sapp_win32_init_dpi();
    _sapp_win32_init_cursors();
    _sapp_win32_create_window();
    sapp_set_icon(&desc->icon);
    #if defined(SOKOL_D3D11)
        _sapp_d3d11_create_device_and_swapchain();
        _sapp_d3d11_create_default_render_target();
    #endif
    #if defined(SOKOL_GLCORE33)
        _sapp_wgl_init();
        _sapp_wgl_load_extensions();
        _sapp_wgl_create_context();
    #endif
    _sapp.valid = true;

    bool done = false;
    while (!(done || _sapp.quit_ordered)) {
        _sapp_win32_timing_measure();
        MSG msg;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (WM_QUIT == msg.message) {
                done = true;
                continue;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
        _sapp_frame();
        #if defined(SOKOL_D3D11)
            _sapp_d3d11_present(false);
            if (IsIconic(_sapp.win32.hwnd)) {
                Sleep((DWORD)(16 * _sapp.swap_interval));
            }
        #endif
        #if defined(SOKOL_GLCORE33)
            _sapp_wgl_swap_buffers();
        #endif
        /* check for window resized, this cannot happen in WM_SIZE as it explodes memory usage */
        if (_sapp_win32_update_dimensions()) {
            #if defined(SOKOL_D3D11)
            _sapp_d3d11_resize_default_render_target();
            #endif
            _sapp_win32_app_event(SAPP_EVENTTYPE_RESIZED);
        }
        /* check if the window monitor has changed, need to reset timing because
           the new monitor might have a different refresh rate
        */
        if (_sapp_win32_update_monitor()) {
            _sapp_timing_reset(&_sapp.timing);
        }
        if (_sapp.quit_requested) {
            PostMessage(_sapp.win32.hwnd, WM_CLOSE, 0, 0);
        }
    }
    _sapp_call_cleanup();

    #if defined(SOKOL_D3D11)
        _sapp_d3d11_destroy_default_render_target();
        _sapp_d3d11_destroy_device_and_swapchain();
    #else
        _sapp_wgl_destroy_context();
        _sapp_wgl_shutdown();
    #endif
    _sapp_win32_destroy_window();
    _sapp_win32_destroy_icons();
    _sapp_win32_restore_console();
    _sapp_discard_state();
}

_SOKOL_PRIVATE char** _sapp_win32_command_line_to_utf8_argv(LPWSTR w_command_line, int* o_argc) {
    int argc = 0;
    char** argv = 0;
    char* args;

    LPWSTR* w_argv = CommandLineToArgvW(w_command_line, &argc);
    if (w_argv == NULL) {
        // FIXME: chicken egg problem, can't report errors before sokol_main() is called!
    } else {
        size_t size = wcslen(w_command_line) * 4;
        argv = (char**) _sapp_malloc_clear(((size_t)argc + 1) * sizeof(char*) + size);
        SOKOL_ASSERT(argv);
        args = (char*) &argv[argc + 1];
        int n;
        for (int i = 0; i < argc; ++i) {
            n = WideCharToMultiByte(CP_UTF8, 0, w_argv[i], -1, args, (int)size, NULL, NULL);
            if (n == 0) {
                // FIXME: chicken egg problem, can't report errors before sokol_main() is called!
                break;
            }
            argv[i] = args;
            size -= (size_t)n;
            args += n;
        }
        LocalFree(w_argv);
    }
    *o_argc = argc;
    return argv;
}

#if !defined(SOKOL_NO_ENTRY)
#if defined(SOKOL_WIN32_FORCE_MAIN)
int main(int argc, char* argv[]) {
    sapp_desc desc = sokol_main(argc, argv);
    _sapp_win32_run(&desc);
    return 0;
}
#else
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    _SOKOL_UNUSED(hInstance);
    _SOKOL_UNUSED(hPrevInstance);
    _SOKOL_UNUSED(lpCmdLine);
    _SOKOL_UNUSED(nCmdShow);
    int argc_utf8 = 0;
    char** argv_utf8 = _sapp_win32_command_line_to_utf8_argv(GetCommandLineW(), &argc_utf8);
    sapp_desc desc = sokol_main(argc_utf8, argv_utf8);
    _sapp_win32_run(&desc);
    _sapp_free(argv_utf8);
    return 0;
}
#endif /* SOKOL_WIN32_FORCE_MAIN */
#endif /* SOKOL_NO_ENTRY */

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#endif /* _SAPP_WIN32 */

// ██████  ██    ██ ██████  ██      ██  ██████
// ██   ██ ██    ██ ██   ██ ██      ██ ██
// ██████  ██    ██ ██████  ██      ██ ██
// ██      ██    ██ ██   ██ ██      ██ ██
// ██       ██████  ██████  ███████ ██  ██████
//
// >>public
#if defined(SOKOL_NO_ENTRY)
SOKOL_API_IMPL void sapp_run(const sapp_desc* desc) {
    SOKOL_ASSERT(desc);
    #if defined(_SAPP_MACOS)
        _sapp_macos_run(desc);
    #elif defined(_SAPP_IOS)
        _sapp_ios_run(desc);
    #elif defined(_SAPP_WIN32)
        _sapp_win32_run(desc);
    #else
    #error "sapp_run() not supported on this platform"
    #endif
}

/* this is just a stub so the linker doesn't complain */
sapp_desc sokol_main(int argc, char* argv[]) {
    _SOKOL_UNUSED(argc);
    _SOKOL_UNUSED(argv);
    sapp_desc desc;
    _sapp_clear(&desc, sizeof(desc));
    return desc;
}
#else
/* likewise, in normal mode, sapp_run() is just an empty stub */
SOKOL_API_IMPL void sapp_run(const sapp_desc* desc) {
    _SOKOL_UNUSED(desc);
}
#endif

SOKOL_API_IMPL bool sapp_isvalid(void) {
    return _sapp.valid;
}

SOKOL_API_IMPL void* sapp_userdata(void) {
    return _sapp.desc.user_data;
}

SOKOL_API_IMPL sapp_desc sapp_query_desc(void) {
    return _sapp.desc;
}

SOKOL_API_IMPL uint64_t sapp_frame_count(void) {
    return _sapp.frame_count;
}

SOKOL_API_IMPL double sapp_frame_duration(void) {
    return _sapp_timing_get_avg(&_sapp.timing);
}

SOKOL_API_IMPL int sapp_width(void) {
    return (_sapp.framebuffer_width > 0) ? _sapp.framebuffer_width : 1;
}

SOKOL_API_IMPL float sapp_widthf(void) {
    return (float)sapp_width();
}

SOKOL_API_IMPL int sapp_height(void) {
    return (_sapp.framebuffer_height > 0) ? _sapp.framebuffer_height : 1;
}

SOKOL_API_IMPL float sapp_heightf(void) {
    return (float)sapp_height();
}

SOKOL_API_IMPL int sapp_color_format(void) {
    #if defined(SOKOL_METAL) || defined(SOKOL_D3D11)
        return _SAPP_PIXELFORMAT_BGRA8;
    #else
        return _SAPP_PIXELFORMAT_RGBA8;
    #endif
}

SOKOL_API_IMPL int sapp_depth_format(void) {
    return _SAPP_PIXELFORMAT_DEPTH_STENCIL;
}

SOKOL_API_IMPL int sapp_sample_count(void) {
    return _sapp.sample_count;
}

SOKOL_API_IMPL bool sapp_high_dpi(void) {
    return _sapp.desc.high_dpi && (_sapp.dpi_scale >= 1.5f);
}

SOKOL_API_IMPL float sapp_dpi_scale(void) {
    return _sapp.dpi_scale;
}

SOKOL_API_IMPL const void* sapp_egl_get_display(void) {
    SOKOL_ASSERT(_sapp.valid);
    return 0;
}

SOKOL_API_IMPL const void* sapp_egl_get_context(void) {
    SOKOL_ASSERT(_sapp.valid);
    return 0;
}

SOKOL_API_IMPL void sapp_show_keyboard(bool show) {
    #if defined(_SAPP_IOS)
    _sapp_ios_show_keyboard(show);
    #else
    _SOKOL_UNUSED(show);
    #endif
}

SOKOL_API_IMPL bool sapp_keyboard_shown(void) {
    return _sapp.onscreen_keyboard_shown;
}

SOKOL_API_IMPL bool sapp_is_fullscreen(void) {
    return _sapp.fullscreen;
}

SOKOL_API_IMPL void sapp_toggle_fullscreen(void) {
    #if defined(_SAPP_MACOS)
    _sapp_macos_toggle_fullscreen();
    #elif defined(_SAPP_WIN32)
    _sapp_win32_toggle_fullscreen();
    #endif
}

/* NOTE that sapp_show_mouse() does not "stack" like the Win32 or macOS API functions! */
SOKOL_API_IMPL void sapp_show_mouse(bool show) {
    if (_sapp.mouse.shown != show) {
        #if defined(_SAPP_MACOS)
        _sapp_macos_update_cursor(_sapp.mouse.current_cursor, show);
        #elif defined(_SAPP_WIN32)
        _sapp_win32_update_cursor(_sapp.mouse.current_cursor, show, false);
        #endif
        _sapp.mouse.shown = show;
    }
}

SOKOL_API_IMPL bool sapp_mouse_shown(void) {
    return _sapp.mouse.shown;
}

SOKOL_API_IMPL void sapp_lock_mouse(bool lock) {
    #if defined(_SAPP_MACOS)
    _sapp_macos_lock_mouse(lock);
    #elif defined(_SAPP_WIN32)
    _sapp_win32_lock_mouse(lock);
    #else
    _sapp.mouse.locked = lock;
    #endif
}

SOKOL_API_IMPL bool sapp_mouse_locked(void) {
    return _sapp.mouse.locked;
}

SOKOL_API_IMPL void sapp_set_mouse_cursor(sapp_mouse_cursor cursor) {
    SOKOL_ASSERT((cursor >= 0) && (cursor < _SAPP_MOUSECURSOR_NUM));
    if (_sapp.mouse.current_cursor != cursor) {
        #if defined(_SAPP_MACOS)
        _sapp_macos_update_cursor(cursor, _sapp.mouse.shown);
        #elif defined(_SAPP_WIN32)
        _sapp_win32_update_cursor(cursor, _sapp.mouse.shown, false);
        #endif
        _sapp.mouse.current_cursor = cursor;
    }
}

SOKOL_API_IMPL sapp_mouse_cursor sapp_get_mouse_cursor(void) {
    return _sapp.mouse.current_cursor;
}

SOKOL_API_IMPL void sapp_request_quit(void) {
    _sapp.quit_requested = true;
}

SOKOL_API_IMPL void sapp_cancel_quit(void) {
    _sapp.quit_requested = false;
}

SOKOL_API_IMPL void sapp_quit(void) {
    _sapp.quit_ordered = true;
}

SOKOL_API_IMPL void sapp_consume_event(void) {
    _sapp.event_consumed = true;
}

/* NOTE: on HTML5, sapp_set_clipboard_string() must be called from within event handler! */
SOKOL_API_IMPL void sapp_set_clipboard_string(const char* str) {
    if (!_sapp.clipboard.enabled) {
        return;
    }
    SOKOL_ASSERT(str);
    #if defined(_SAPP_MACOS)
        _sapp_macos_set_clipboard_string(str);
    #elif defined(_SAPP_WIN32)
        _sapp_win32_set_clipboard_string(str);
    #else
        /* not implemented */
    #endif
    _sapp_strcpy(str, _sapp.clipboard.buffer, _sapp.clipboard.buf_size);
}

SOKOL_API_IMPL const char* sapp_get_clipboard_string(void) {
    if (!_sapp.clipboard.enabled) {
        return "";
    }
    #if defined(_SAPP_MACOS)
        return _sapp_macos_get_clipboard_string();
    #elif defined(_SAPP_WIN32)
        return _sapp_win32_get_clipboard_string();
    #else
        /* not implemented */
        return _sapp.clipboard.buffer;
    #endif
}

SOKOL_API_IMPL void sapp_set_window_title(const char* title) {
    SOKOL_ASSERT(title);
    _sapp_strcpy(title, _sapp.window_title, sizeof(_sapp.window_title));
    #if defined(_SAPP_MACOS)
        _sapp_macos_update_window_title();
    #elif defined(_SAPP_WIN32)
        _sapp_win32_update_window_title();
    #endif
}

SOKOL_API_IMPL void sapp_set_icon(const sapp_icon_desc* desc) {
    SOKOL_ASSERT(desc);
    if (desc->sokol_default) {
        if (0 == _sapp.default_icon_pixels) {
            _sapp_setup_default_icon();
        }
        SOKOL_ASSERT(0 != _sapp.default_icon_pixels);
        desc = &_sapp.default_icon_desc;
    }
    const int num_images = _sapp_icon_num_images(desc);
    if (num_images == 0) {
        return;
    }
    SOKOL_ASSERT((num_images > 0) && (num_images <= SAPP_MAX_ICONIMAGES));
    if (!_sapp_validate_icon_desc(desc, num_images)) {
        return;
    }
    #if defined(_SAPP_MACOS)
        _sapp_macos_set_icon(desc, num_images);
    #elif defined(_SAPP_WIN32)
        _sapp_win32_set_icon(desc, num_images);
    #endif
}

SOKOL_API_IMPL int sapp_get_num_dropped_files(void) {
    SOKOL_ASSERT(_sapp.drop.enabled);
    return _sapp.drop.num_files;
}

SOKOL_API_IMPL const char* sapp_get_dropped_file_path(int index) {
    SOKOL_ASSERT(_sapp.drop.enabled);
    SOKOL_ASSERT((index >= 0) && (index < _sapp.drop.num_files));
    SOKOL_ASSERT(_sapp.drop.buffer);
    if (!_sapp.drop.enabled) {
        return "";
    }
    if ((index < 0) || (index >= _sapp.drop.max_files)) {
        return "";
    }
    return (const char*) _sapp_dropped_file_path_ptr(index);
}

SOKOL_API_IMPL uint32_t sapp_html5_get_dropped_file_size(int index) {
    SOKOL_ASSERT(_sapp.drop.enabled);
    SOKOL_ASSERT((index >= 0) && (index < _sapp.drop.num_files));
    (void)index;
    return 0;
}

SOKOL_API_IMPL void sapp_html5_fetch_dropped_file(const sapp_html5_fetch_request* request) {
    SOKOL_ASSERT(_sapp.drop.enabled);
    SOKOL_ASSERT(request);
    SOKOL_ASSERT(request->callback);
    SOKOL_ASSERT(request->buffer.ptr);
    SOKOL_ASSERT(request->buffer.size > 0);
    (void)request;
}

SOKOL_API_IMPL const void* sapp_metal_get_device(void) {
    SOKOL_ASSERT(_sapp.valid);
    #if defined(SOKOL_METAL)
        #if defined(_SAPP_MACOS)
            const void* obj = (__bridge const void*) _sapp.macos.mtl_device;
        #else
            const void* obj = (__bridge const void*) _sapp.ios.mtl_device;
        #endif
        SOKOL_ASSERT(obj);
        return obj;
    #else
        return 0;
    #endif
}

SOKOL_API_IMPL const void* sapp_metal_get_renderpass_descriptor(void) {
    SOKOL_ASSERT(_sapp.valid);
    #if defined(SOKOL_METAL)
        #if defined(_SAPP_MACOS)
            const void* obj = (__bridge const void*) [_sapp.macos.view currentRenderPassDescriptor];
        #else
            const void* obj = (__bridge const void*) [_sapp.ios.view currentRenderPassDescriptor];
        #endif
        SOKOL_ASSERT(obj);
        return obj;
    #else
        return 0;
    #endif
}

SOKOL_API_IMPL const void* sapp_metal_get_drawable(void) {
    SOKOL_ASSERT(_sapp.valid);
    #if defined(SOKOL_METAL)
        #if defined(_SAPP_MACOS)
            const void* obj = (__bridge const void*) [_sapp.macos.view currentDrawable];
        #else
            const void* obj = (__bridge const void*) [_sapp.ios.view currentDrawable];
        #endif
        SOKOL_ASSERT(obj);
        return obj;
    #else
        return 0;
    #endif
}

SOKOL_API_IMPL const void* sapp_macos_get_window(void) {
    #if defined(_SAPP_MACOS)
        const void* obj = (__bridge const void*) _sapp.macos.window;
        SOKOL_ASSERT(obj);
        return obj;
    #else
        return 0;
    #endif
}

SOKOL_API_IMPL const void* sapp_ios_get_window(void) {
    #if defined(_SAPP_IOS)
        const void* obj = (__bridge const void*) _sapp.ios.window;
        SOKOL_ASSERT(obj);
        return obj;
    #else
        return 0;
    #endif
}

SOKOL_API_IMPL const void* sapp_d3d11_get_device(void) {
    SOKOL_ASSERT(_sapp.valid);
    #if defined(SOKOL_D3D11)
        return _sapp.d3d11.device;
    #else
        return 0;
    #endif
}

SOKOL_API_IMPL const void* sapp_d3d11_get_device_context(void) {
    SOKOL_ASSERT(_sapp.valid);
    #if defined(SOKOL_D3D11)
        return _sapp.d3d11.device_context;
    #else
        return 0;
    #endif
}

SOKOL_API_IMPL const void* sapp_d3d11_get_swap_chain(void) {
    SOKOL_ASSERT(_sapp.valid);
#if defined(SOKOL_D3D11)
    return _sapp.d3d11.swap_chain;
#else
    return 0;
#endif
}

SOKOL_API_IMPL const void* sapp_d3d11_get_render_target_view(void) {
    SOKOL_ASSERT(_sapp.valid);
    #if defined(SOKOL_D3D11)
        if (_sapp.d3d11.msaa_rtv) {
            return _sapp.d3d11.msaa_rtv;
        }
        else {
            return _sapp.d3d11.rtv;
        }
    #else
        return 0;
    #endif
}

SOKOL_API_IMPL const void* sapp_d3d11_get_depth_stencil_view(void) {
    SOKOL_ASSERT(_sapp.valid);
    #if defined(SOKOL_D3D11)
        return _sapp.d3d11.dsv;
    #else
        return 0;
    #endif
}

SOKOL_API_IMPL const void* sapp_win32_get_hwnd(void) {
    SOKOL_ASSERT(_sapp.valid);
    #if defined(_SAPP_WIN32)
        return _sapp.win32.hwnd;
    #else
        return 0;
    #endif
}

SOKOL_API_IMPL const void* sapp_wgpu_get_device(void) {
    SOKOL_ASSERT(_sapp.valid);
    return 0;
}

SOKOL_API_IMPL const void* sapp_wgpu_get_render_view(void) {
    SOKOL_ASSERT(_sapp.valid);
    return 0;
}

SOKOL_API_IMPL const void* sapp_wgpu_get_resolve_view(void) {
    SOKOL_ASSERT(_sapp.valid);
    return 0;
}

SOKOL_API_IMPL const void* sapp_wgpu_get_depth_stencil_view(void) {
    SOKOL_ASSERT(_sapp.valid);
    return 0;
}

SOKOL_API_IMPL void sapp_html5_ask_leave_site(bool ask) {
    _sapp.html5_ask_leave_site = ask;
}

#endif /* SOKOL_APP_IMPL */