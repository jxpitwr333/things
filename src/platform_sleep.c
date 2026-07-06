#include "platform_sleep.h"

#if defined(_WIN32) || defined(_WIN64)
    // Tell windows.h to skip the bloated GDI and User sub-libraries 
    // that conflict with Raylib's Rectangle/CloseWindow/ShowCursor
    #define WIN32_LEAN_AND_MEAN
    #define NOGDI
    #define NOUSER
    #include <windows.h>
    
    void sys_sleep(unsigned int milliseconds) {
        Sleep(milliseconds);
    }
#elif defined(__unix__) || defined(__APPLE__) || defined(__linux__)
    #define _POSIX_C_SOURCE 199309L
    #include <time.h>
    
    void sys_sleep(unsigned int milliseconds) {
        struct timespec ts;
        ts.tv_sec = milliseconds / 1000;
        ts.tv_nsec = (milliseconds % 1000) * 1000000L;
        while (nanosleep(&ts, &ts) == -1) { }
    }
#else
    #error "Unsupported platform"
#endif