//------------------------------------------------------------------------------
//  nuklear-sapp.c
//
//  Demonstrates Nuklear UI rendering in C via
//  sokol_gfx.h + sokol_nuklear.h + nuklear.h
//
//  Nuklear UI on github: https://github.com/Immediate-Mode-UI/Nuklear
//------------------------------------------------------------------------------
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

#include <math.h>

static int draw_demo_ui(struct nk_context* ctx);

enum {AUDIO_OFF, AUDIO_ON};
static int gAudioBypass;
// -60-0dB
static float gGaindB;
// normalised logarithmic range 20Hz-20kHz
static float gFrequencyNormalised;
// oscillator phase, 0-1
static float gPhase;

static inline float denormalise_hz(float n) { return 20 * exp2f(n * 10); }
static inline float normalise_hz(float Hz) { return logf(Hz * 0.05) * 0.14426950408889633f; }
static inline float gain_to_db(float g) { return log10f(g) * 20; }
static inline float db_to_gain(float db) { return pow(10, db / 20); }

static void audio_cb(float* buffer, int num_frames, int num_channels) {
    assert(1 == num_channels);

    if (gAudioBypass == AUDIO_OFF)
        return;

    float sample_rate = (float)saudio_sample_rate();

    float phase = gPhase;
    float freq = denormalise_hz(gFrequencyNormalised);
    float vol = db_to_gain(gGaindB);

    float inc = freq / (float)sample_rate;

    static uint32_t count = 0;
    for (int i = 0; i < num_frames; i++) {
        buffer[i] = vol * sinf(phase * 2 * 3.14159265f);
        phase += inc;
        phase -= (int)phase;
    }

    gPhase = phase;
}


void init(void) {
    // initialise audio state
    gAudioBypass = AUDIO_OFF;
    gGaindB = -12.0f;
    gFrequencyNormalised = normalise_hz(500.0f);
    gPhase = 0.0f;

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

    // init sokol-audio with default params
    saudio_setup(&(saudio_desc){
        .stream_cb = audio_cb,
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
    // shutdown sokol-audio
    saudio_shutdown();
    // __dbgui_shutdown();
    snk_shutdown();
    sg_shutdown();
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
        .window_title = "nuklear (sokol-app)",
        .ios_keyboard_resizes_canvas = true,
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}

/* copied from: https://github.com/Immediate-Mode-UI/Nuklear/blob/master/demo/overview.c */
#if defined(__GNUC__)
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#endif
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

static int
draw_demo_ui(struct nk_context *ctx)
{
    if (nk_begin(ctx, "Show", nk_rect(50, 50, 300, 220),
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
        nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
        {
            nk_layout_row_push(ctx, 60);
            nk_label(ctx, "Volume:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 200);
            nk_slider_float(ctx, -60, &gGaindB, 0.0f, 0.00000001f);
        }
        nk_layout_row_end(ctx);

        nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
        {
            nk_layout_row_push(ctx, 60);
            nk_label(ctx, "Frequency:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 200);
            nk_slider_float(ctx, 0, &gFrequencyNormalised, 1.0f, 0.00000001f);
        }
        nk_layout_row_end(ctx);
    }
    nk_end(ctx);

    return !nk_window_is_closed(ctx, "Overview");
}