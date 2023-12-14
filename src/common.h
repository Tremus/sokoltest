#pragma once
#if defined(_WIN32)
#define SOKOL_D3D11
#elif defined(__APPLE__)
#define SOKOL_METAL
#endif