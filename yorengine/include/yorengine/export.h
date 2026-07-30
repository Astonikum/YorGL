#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(YORENGINE_BUILDING_DLL)
#define YORENGINE_API __declspec(dllexport)
#else
#define YORENGINE_API __declspec(dllimport)
#endif
#else
#define YORENGINE_API __attribute__((visibility("default")))
#endif
