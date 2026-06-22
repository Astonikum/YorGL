#pragma once

#if defined(_WIN32)
#  if defined(YORGL_BUILDING_DLL)
#    define YORGL_API __declspec(dllexport)
#  else
#    define YORGL_API __declspec(dllimport)
#  endif
#else
#  define YORGL_API __attribute__((visibility("default")))
#endif
