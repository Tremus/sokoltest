#include "common.h"

// this is needed for the Nuklear example code further down
#define _CRT_SECURE_NO_WARNINGS (1)

#define SOKOL_LOG_IMPL
#define SOKOL_GLUE_IMPL
#include "sokol_app.h"
#include "sokol_audio.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"


// include nuklear.h before the sokol_nuklear.h implementation
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS
#include "nuklear/nuklear.h"
#define SOKOL_NUKLEAR_IMPL
#include "sokol_nuklear.h"
#include "thread.h"
#define MINIMIDI_IMPL
#define MINIMIDI_USE_GLOBAL
#include "minimidi.h"

#ifdef _WIN32
// void Sleep(unsigned long ms);
#define SLEEP(ms) Sleep(ms)
#define print(str, ...) (printf(str, __VA_ARGS__), fflush(stdout))
#else
#include <unistd.h>
#define SLEEP(ms) usleep(ms * 1000)
#define print printf
#endif

#include <math.h>

static int draw_demo_ui(struct nk_context* ctx);

// Midi stuff
thread_atomic_int_t gExitThreads = {.i = 0};
thread_ptr_t        gMidiThread  = NULL;

enum MidiEventType
{
    MIDI_NOTE_OFF = 0x80,
    MIDI_NOTE_ON  = 0x90,
};

// Midi thread...
static int midi_cb(void* userdata)
{
    MiniMIDI*    mm;
    unsigned int numPorts;
    char         portName[128];
    int          err;

    mm = minimidi_get_global();

    if (! mm)
    {
        print("Failed to init RtMIDI! Exiting thread...\n");
        return 1;
    }

    // Check available ports.
    numPorts = minimidi_get_num_ports(mm);
    while (numPorts == 0)
    {
        print("No MIDI ports available! Please connect a device. Sleeping for 1sec\n");
        SLEEP(1000);

        if (thread_atomic_int_load(&gExitThreads) != 0)
            return 0;

        numPorts = minimidi_get_num_ports(mm);
    }
    err = minimidi_get_port_name(mm, 0, portName, sizeof(portName));
    if (err != 0)
    {
        print("Failed getting port name!\n");
        return 1;
    }
    err = minimidi_connect_port(mm, 0, "sokol-nuklear-midi");
    if (err != 0)
    {
        print("Failed connecting to port 0!\n");
        return 1;
    }

    // Periodically check input queue.
    print("Reading MIDI from port %s.\n", portName);
    while (thread_atomic_int_load(&gExitThreads) != 1)
    {
#ifdef _WIN32
        /* Hotplugging for windows. On MacOS it's automatic... */
        if (minimidi_should_reconnect(mm))
        {
            static const int HOTPLUG_TIMEOUT        = (1000 * 60 * 2); /* 2min */
            static const int HOTPLUG_SLEEP_INTERVAL = 100;             /* 100ms */
            int              msCounter              = 0;

            print("WARNING: Unknown device disconnected!\n");
            print("If this was your MIDI device, please plug it back in. This program will automatically reconnect.\n");

            while (msCounter < HOTPLUG_TIMEOUT)
            {
                if (minimidi_try_reconnect(mm, "sokol-nuklear-midi"))
                {
                    print("Successfully reconnected!\n");
                    break;
                }

                SLEEP(HOTPLUG_SLEEP_INTERVAL);

                if (thread_atomic_int_load(&gExitThreads) != 0)
                    return 0;

                msCounter += HOTPLUG_SLEEP_INTERVAL;
            }
        }
#endif

        SLEEP(100);
    }
    print("Disconnecting from MIDI port\n");
    minimidi_disconnect_port(mm);

    // OS will clear memory...
    // rtmidi_in_free(mm);
    return 0;
}

// Audio parameters
enum
{
    AUDIO_OFF,
    AUDIO_ON
};
static int gAudioBypass = AUDIO_ON;
// -60-0dB
static float gGaindB = -12.0f;
// oscillator phase, 0-1
static float gPhase = 0.0f;

static float gCrossover = 0.5f;

// If 0-127 if playing, 255 (0xff) if inactive
static uint8_t      gCurrentMidiNote = 0xff;
static inline float gain_to_db(float g) { return log10f(g) * 20; }
static inline float db_to_gain(float db) { return pow(10, db / 20); }

// Audio thread...
static void audio_cb(float* buffer, int num_frames, int num_channels)
{
    assert(1 == num_channels);
    if (thread_atomic_int_load(&gExitThreads) == 1)
        return;

    MiniMIDI*       mm  = minimidi_get_global();
    MiniMIDIMessage msg = minimidi_read_message(mm);
    while (msg.timestampMs != 0)
    {
        if ((msg.status & 0xf0) == MIDI_NOTE_ON)
        {
            uint8_t channel  = msg.status & 0x0f;
            uint8_t midiNote = msg.data1;
            uint8_t velocity = msg.data2;
            // ignore channel & velocity
            gCurrentMidiNote = midiNote;
        }
        else if ((msg.status & 0xf0) == MIDI_NOTE_OFF)
        {
            uint8_t channel  = msg.status & 0x0f;
            uint8_t midiNote = msg.data1;
            uint8_t velocity = msg.data2;
            // ignore channel & velocity
            if (midiNote == gCurrentMidiNote)
                gCurrentMidiNote = 0xff;
        }
        msg = minimidi_read_message(mm);
    }

    // Check if playing
    if (gCurrentMidiNote == 0xff || gAudioBypass == AUDIO_OFF)
    {
        memset(buffer, 0, num_frames * sizeof(*buffer));
        return;
    }

    float Hz    = exp2f(((float)gCurrentMidiNote - 69.0f) * 0.0833333f) * 440.0f;
    float phase = gPhase;
    float inc   = Hz / (float)saudio_sample_rate();
    float vol   = db_to_gain(gGaindB);

    for (int i = 0; i < num_frames; i++)
    {
        float sample = phase >= 0.5f ? 1.0f : -1.0f;
        // float sample = sinf(phase * 2 * 3.14159265f);
        // buffer[i] = vol * sinf(phase * 2 * 3.14159265f);
        buffer[i] = vol * sample;
        phase     += inc;
        phase     -= (int)phase;
    }
    gPhase = phase;
}

// App stuff

void init(void)
{
    // init midi thread
    minimidi_init(minimidi_get_global());
    // start midi thread
    gMidiThread = thread_create(midi_cb, NULL, 0);

    // init sokol-audio with default params (monophonic)
    saudio_setup(&(saudio_desc){
        .stream_cb   = audio_cb,
        .logger.func = slog_func,
    });

    // setup sokol-gfx, sokol-time and sokol-nuklear
    sg_setup(&(sg_desc){
        .context     = sapp_sgcontext(),
        .logger.func = slog_func,
    });
    // __dbgui_setup(sapp_sample_count());

    // use sokol-nuklear with all default-options (we're not doing
    // multi-sampled rendering or using non-default pixel formats)
    snk_setup(&(snk_desc_t){
        .dpi_scale   = sapp_dpi_scale(),
        .logger.func = slog_func,
    });
}

void frame(void)
{
    struct nk_context* ctx = snk_new_frame();

    // see big function at end of file
    draw_demo_ui(ctx);

    // the sokol_gfx draw pass
    const sg_pass_action pass_action = {
        .colors[0] = {.load_action = SG_LOADACTION_CLEAR, .clear_value = {0.25f, 0.5f, 0.7f, 1.0f}}};
    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
    snk_render(sapp_width(), sapp_height());
    // __dbgui_draw();
    sg_end_pass();
    sg_commit();
}

void cleanup(void)
{
    thread_atomic_int_store(&gExitThreads, 1);
    saudio_shutdown();
    thread_join(gMidiThread);

    // __dbgui_shutdown();
    snk_shutdown();
    sg_shutdown();

    // The OS automatically frees memory when finishing process...
    // thread_destroy(gMidiThread);
}

void input(const sapp_event* event)
{
    // if (!__dbgui_event_with_retval(event)) {
    snk_handle_event(event);
    // }
}

struct sapp_desc create_sapp_desc()
{
    return (sapp_desc){
        .init_cb                     = init,
        .frame_cb                    = frame,
        .cleanup_cb                  = cleanup,
        .event_cb                    = input,
        .enable_clipboard            = true,
        .width                       = 720,
        .height                      = 480,
        .window_title                = "Sine Synthesiser (Mono)",
        .ios_keyboard_resizes_canvas = true,
        .icon.sokol_default          = true,
        .logger.func                 = slog_func,
    };
}

static int draw_demo_ui(struct nk_context* ctx)
{
    if (nk_begin(ctx, "Show", nk_rect(50, 50, 380, 220), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE))
    {
        /* fixed widget pixel width */
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "button"))
        {
            /* event handling */
        }

        /* fixed widget window ratio width */
        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "Audio Off", gAudioBypass == AUDIO_OFF))
            gAudioBypass = AUDIO_OFF;
        if (nk_option_label(ctx, "Audio On", gAudioBypass == AUDIO_ON))
            gAudioBypass = AUDIO_ON;

        /* custom widget pixel width */
        nk_layout_row_begin(ctx, NK_STATIC, 30, 3);
        {
            nk_layout_row_push(ctx, 70);
            nk_label(ctx, "Volume:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 200);
            nk_slider_float(ctx, -60, &gGaindB, 0.0f, 0.00000001f);

            char text[16];
            snprintf(text, sizeof(text), "%.2fdB", gGaindB);
            nk_layout_row_push(ctx, 70);
            nk_label(ctx, text, NK_TEXT_LEFT);
        }
        nk_layout_row_end(ctx);

        nk_layout_row_begin(ctx, NK_STATIC, 30, 1);
        {
            static const char* midiLetters[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
            char               text[16];
            int                midi   = gCurrentMidiNote;
            int                octave = (midi / 12) - 3;
            const char*        letter = midiLetters[midi % 12];

            if (midi != 0xff)
                snprintf(text, sizeof(text), "Note: %s%d", letter, octave);
            else
                snprintf(text, sizeof(text), "Note: off");

            nk_layout_row_push(ctx, 70);
            nk_label(ctx, text, NK_TEXT_LEFT);
        }
        nk_layout_row_end(ctx);
    }
    nk_end(ctx);

    return ! nk_window_is_closed(ctx, "Overview");
}