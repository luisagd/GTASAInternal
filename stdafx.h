// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <vector>
#include <array>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <atomic>
#include <windowsx.h>
#include <d3d9.h>
#include <D3dx9core.h>
#include <d3dcommon.h>

//#define FMT_HEADER_ONLY 1
#include <fmt/core.h>
#include <fmt/format.h>
//#include <iostream>

constexpr float PI = 3.1415927f;

#endif //PCH_H
