#define _POSIX_C_SOURCE 199309L
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern bool g_IsRunning;

static Display *g_Display       = NULL;
static Window   g_Window;
static GC       g_GC;
static XImage  *g_XImage       = NULL;
static XShmSegmentInfo g_ShmInfo;

void platform_create_window(int width, int height) {
    g_Display = XOpenDisplay(NULL);
    if (!g_Display) {
        fprintf(stderr, "Failed to open X Display\n");
        exit(1);
    }

    int screen = DefaultScreen(g_Display);
    Visual *visual = DefaultVisual(g_Display, screen);
    int depth = DefaultDepth(g_Display, screen);

    if (!XShmQueryExtension(g_Display)) {
        fprintf(stderr, "XShm extension not available!\n");
        exit(1);
    }

    XSetWindowAttributes swa = {
        .event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask
    };

    g_Window = XCreateWindow(
        g_Display, RootWindow(g_Display, screen),
        0, 0, width, height, 0,
        depth, InputOutput, visual,
        CWEventMask, &swa
    );

    XStoreName(g_Display, g_Window, "Things");
    XMapWindow(g_Display, g_Window);
    g_GC = XCreateGC(g_Display, g_Window, 0, NULL);

    Atom wmDeleteMessage = XInternAtom(g_Display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(g_Display, g_Window, &wmDeleteMessage, 1);

    g_XImage = XShmCreateImage(
        g_Display, visual, depth, ZPixmap, 
        NULL, &g_ShmInfo, width, height
    );

    if (!g_XImage) {
        fprintf(stderr, "Failed to create XShm Image\n");
        exit(1);
    }

    g_ShmInfo.shmid = shmget(
        IPC_PRIVATE, 
        g_XImage->bytes_per_line * g_XImage->height, 
        IPC_CREAT | 0777
    );
    
    if (g_ShmInfo.shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    g_ShmInfo.shmaddr = (char *)shmat(g_ShmInfo.shmid, NULL, 0);
    g_XImage->data = g_ShmInfo.shmaddr;
    g_ShmInfo.readOnly = False;

    if (g_ShmInfo.shmaddr == (char *)-1) {
        perror("shmat failed");
        exit(1);
    }

    if (!XShmAttach(g_Display, &g_ShmInfo)) {
        fprintf(stderr, "XShmAttach failed\n");
        exit(1);
    }

    XSync(g_Display, False);
}

void platform_process_events(void) {
    while (XPending(g_Display)) {
        XEvent event;
        XNextEvent(g_Display, &event);

        switch (event.type) {
            case ClientMessage: {
                Atom wmDeleteMessage = XInternAtom(g_Display, "WM_DELETE_WINDOW", False);
                if ((Atom)event.xclient.data.l[0] == wmDeleteMessage) {
                    g_IsRunning = false;
                }
                break;
            }
            case DestroyNotify:
                g_IsRunning = false;
                break;
        }
    }
}


void platform_sleep(uint32_t ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    
    while (nanosleep(&ts, &ts) == -1) {}
}

void platform_present_buffer(const void *buffer, int width, int height) {
    uint8_t *dest_bytes = (uint8_t *)g_XImage->data;
    const uint32_t *src = (const uint32_t *)buffer;

    int dest_stride = g_XImage->bytes_per_line;

    int win_w = 800; 
    int win_h = 800;

    for (int y = 0; y < win_h; ++y) {
        int src_y = (y * height) / win_h;
        uint32_t *dest_row = (uint32_t *)(dest_bytes + (y * dest_stride));
        const uint32_t *src_row = src + (src_y * width);

        for (int x = 0; x < win_w; ++x) {
            int src_x = (x * width) / win_w;
            dest_row[x] = src_row[src_x];
        }
    }

    XShmPutImage(
        g_Display, g_Window, g_GC, g_XImage, 
        0, 0, 0, 0, win_w, win_h, False
    );

    XFlush(g_Display);
    platform_sleep(16);
}

void platform_destroy_window(void) {
    if (g_Display) {
        if (g_XImage) {
            XShmDetach(g_Display, &g_ShmInfo);
            XDestroyImage(g_XImage);
            shmdt(g_ShmInfo.shmaddr);
            shmctl(g_ShmInfo.shmid, IPC_RMID, 0);
        }
        XFreeGC(g_Display, g_GC);
        XDestroyWindow(g_Display, g_Window);
        XCloseDisplay(g_Display);
    }
}
