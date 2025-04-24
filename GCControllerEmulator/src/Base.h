#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define GC_CONTROLLER_EMULATOR_EXPORT __declspec(dllexport)
    #define GC_CONTROLLER_EMULATOR_IMPORT __declspec(dllimport)
    #define GC_CONTROLLER_EMULATOR_LOCAL
#else
    #define GC_CONTROLLER_EMULATOR_EXPORT __attribute__((visibility("default")))
    #define GC_CONTROLLER_EMULATOR_IMPORT __attribute__((visibility("default")))
    #define GC_CONTROLLER_EMULATOR_LOCAL  __attribute__((visibility("hidden")))
#endif

#ifdef GC_CONTROLLER_EMULATOR_DLL
    #define GC_CONTROLLER_EMULATOR_PUBLIC GC_CONTROLLER_EMULATOR_EXPORT
#else
    #define GC_CONTROLLER_EMULATOR_PUBLIC GC_CONTROLLER_EMULATOR_IMPORT
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
    #include <stdio.h>

    #define PRINT_ERR_MSG(...) fprintf(stderr, __VA_ARGS__)
#else
    #define PRINT_ERR_MSG(...)
#endif
