#pragma once
#if defined(_WIN32)
#define SOKOL_D3D11
#define SOKOL_NO_ENTRY
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#elif defined(__APPLE__)
#define SOKOL_METAL
#endif