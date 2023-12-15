#include "common.h"

// this is needed for the Nuklear example code further down
#define _CRT_SECURE_NO_WARNINGS (1)

#define SOKOL_LOG_IMPL
#define SOKOL_GLUE_IMPL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_audio.h"
#include "sokol_glue.h"

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
#define INCLUDED_RINGBUF_IMPL
#include "ringbuffer.h"
// TODO: replace rtmidi
#include "rtmidi/rtmidi_c.h"

#ifdef _WIN32
// void Sleep(unsigned long ms);
#define SLEEP(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP(ms) usleep(ms * 1000)
#endif

#include <math.h>

static int draw_demo_ui(struct nk_context* ctx);

// Midi stuff
ringbuf_t gMidiRingBuffer;
thread_atomic_int_t gExitThreads;
thread_ptr_t gMidiThread;

enum MidiEventType {
    MIDI_NOTE_OFF         = 0x80,
    MIDI_NOTE_ON          = 0x90,
};

// Midi thread...
static int midi_cb(void* userdata)
{
    unsigned char messages[1024];
    size_t nBytes;
    struct RtMidiWrapper *midiin;
    double stamp;
    char portName[128];
    unsigned int nPorts;
    int portNameLen;
    midiin = rtmidi_in_create_default();

    if (!midiin) {
        printf("Failed to init RtMIDI! Exiting thread...\n");
        return 1;
    }

    // Check available ports.
    nPorts = rtmidi_get_port_count(midiin);
    while (nPorts == 0) {
        printf("No MIDI ports available! Sleeping for 1sec\n");
        SLEEP(1000);
    }
    rtmidi_get_port_name(midiin, 0, portName, &portNameLen);
    rtmidi_open_port(midiin, 0, "RtMidi");

    // Periodically check input queue.
    printf("Reading MIDI from port %s.\n", portName);
    while (midiin->ok) {
        do {
            nBytes = sizeof(messages);
            stamp = rtmidi_in_get_message(midiin, messages, &nBytes);
            // ignore timestamp. this is quick and dirty
            ringbuf_memcpy_into(gMidiRingBuffer, messages, nBytes);
        }
        while (nBytes != 0 && midiin->ok);

        if (thread_atomic_int_load(&gExitThreads) == 1)
            break;

        SLEEP(10);
        // thread_yield();
    }
    rtmidi_close_port(midiin);

    // OS will clear memory...
    // rtmidi_in_free(midiin);
    return 0;
}

// Audio parameters
enum {AUDIO_OFF, AUDIO_ON};
static int gAudioBypass;
// -60-0dB
static float gGaindB;
// oscillator phase, 0-1
static float gPhase;
// If 0-127 if playing, 255 (0xff) if inactive
static uint8_t gCurrentMidiNote;
static inline float gain_to_db(float g) { return log10f(g) * 20; }
static inline float db_to_gain(float db) { return pow(10, db / 20); }

// Audio thread...
static void audio_cb(float* buffer, int num_frames, int num_channels) {
    assert(1 == num_channels);
    if (thread_atomic_int_load(&gExitThreads) == 1)
        return;

    // (poorly) process incoming MIDI
    uint8_t *bufend = ringbuf_end(gMidiRingBuffer);
    uint8_t *head = ringbuf_head(gMidiRingBuffer);
    uint8_t *tail = ringbuf_tail(gMidiRingBuffer);
    while (tail != head) {
        uint8_t status = *tail;
        if (++tail == bufend) tail = gMidiRingBuffer->buf;

        if ((status & 0xf0) == MIDI_NOTE_ON) {
            unsigned channel  = status & 0x0f;
            unsigned midiNote = *tail;
            if (++tail == bufend) tail = gMidiRingBuffer->buf;
            unsigned velocity = *tail;
            if (++tail == bufend) tail = gMidiRingBuffer->buf;
            // ignore channel & velocity
            gCurrentMidiNote = midiNote;

        } else if ((status & 0xf0) == MIDI_NOTE_OFF) {
            uint8_t  channel = status & 0x0f;
            uint8_t  midiNote = *tail;
            if (++tail == bufend) tail = gMidiRingBuffer->buf;
            uint8_t  velocity = *tail;
            if (++tail == bufend) tail = gMidiRingBuffer->buf;
            // ignore channel & velocity
            if (midiNote == gCurrentMidiNote)
                gCurrentMidiNote = 0xff;
        }
    }
    gMidiRingBuffer->tail = tail;

    // Check if playing 
    if (gCurrentMidiNote == 0xff || gAudioBypass == AUDIO_OFF)
    {
        memset(buffer, 0, num_frames * sizeof(*buffer));
        return;
    }

    float Hz = exp2f(((float)gCurrentMidiNote - 69.0f) * 0.0833333f) * 440.0f;
    float phase = gPhase;
    float inc = Hz / (float)saudio_sample_rate();
    float vol = db_to_gain(gGaindB);

    for (int i = 0; i < num_frames; i++) {
        buffer[i] = vol * sinf(phase * 2 * 3.14159265f);
        phase += inc;
        phase -= (int)phase;
    }
    gPhase = phase;
}

// App stuff

void init(void) {
    // init midi thread
    gMidiRingBuffer = ringbuf_new(2048);
    thread_atomic_int_store(&gExitThreads,  0);
    // start midi thread
    gMidiThread = thread_create(midi_cb, NULL, 0);

    // initialise audio state
    gAudioBypass = AUDIO_ON;
    gGaindB = -12.0f;
    gPhase = 0.0f;
    gCurrentMidiNote = 0xff;

    // init sokol-audio with default params (monophonic)
    saudio_setup(&(saudio_desc){
        .stream_cb = audio_cb,
        .logger.func = slog_func,
    });

    // setup sokol-gfx, sokol-time and sokol-nuklear
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext(),
        .logger.func = slog_func,
    });
    // __dbgui_setup(sapp_sample_count());

    // use sokol-nuklear with all default-options (we're not doing
    // multi-sampled rendering or using non-default pixel formats)
    snk_setup(&(snk_desc_t){
        .dpi_scale = sapp_dpi_scale(),
        .logger.func = slog_func,
    });
}

void frame(void) {
    struct nk_context *ctx = snk_new_frame();

    // see big function at end of file
    draw_demo_ui(ctx);

    // the sokol_gfx draw pass
    const sg_pass_action pass_action = {
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.25f, 0.5f, 0.7f, 1.0f }
        }
    };
    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
    snk_render(sapp_width(), sapp_height());
    // __dbgui_draw();
    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    thread_atomic_int_store(&gExitThreads, 1);
    thread_join(gMidiThread);

    saudio_shutdown();
    // __dbgui_shutdown();
    snk_shutdown();
    sg_shutdown();

    // The OS automatically frees memory when finishing process...
    // thread_destroy(gMidiThread);
    // ringbuf_free(gMidiRingBuffer);
}

void input(const sapp_event* event) {
    // if (!__dbgui_event_with_retval(event)) {
        snk_handle_event(event);
    // }
}

struct sapp_desc create_sapp_desc()
{
    return (sapp_desc) {
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = input,
        .enable_clipboard = true,
        .width = 720,
        .height = 480,
        .window_title = "Sine Synthesiser (Mono)",
        .ios_keyboard_resizes_canvas = true,
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}

static int draw_demo_ui(struct nk_context *ctx) {
    if (nk_begin(ctx, "Show", nk_rect(50, 50, 380, 220),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
        /* fixed widget pixel width */
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "button")) {
            /* event handling */
        }

        /* fixed widget window ratio width */
        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "Audio Off", gAudioBypass == AUDIO_OFF)) gAudioBypass = AUDIO_OFF;
        if (nk_option_label(ctx, "Audio On",  gAudioBypass == AUDIO_ON))  gAudioBypass = AUDIO_ON;

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
            static const char* midiLetters[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
            char text[16];
            int  midi = gCurrentMidiNote;
            int octave = (midi / 12) - 3;
            const char* letter = midiLetters[midi % 12];
            
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

    return !nk_window_is_closed(ctx, "Overview");
}