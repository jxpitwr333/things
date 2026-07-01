#include <windows.h>

static HWND g_WindowHandle = NULL;
extern _Bool g_IsRunning;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CLOSE:
		case WM_DESTROY:
			g_IsRunning = 0;
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void platform_create_window(int width, int height) {
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {
        .lpfnWndProc   = WindowProc,
        .hInstance     = hInstance,
        .lpszClassName = "GameWindowClass",
        .hCursor       = LoadCursor(NULL, IDC_ARROW)
    };
    RegisterClass(&wc);

    g_WindowHandle = CreateWindowExA(
        0, "GameWindowClass", "Things",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, hInstance, NULL
    );
}

void platform_process_events(void) {
	MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void platform_present_buffer(const void *buffer, int width, int height) {
	HDC hdc = GetDC(g_WindowHandle);
    RECT clientRect;
    GetClientRect(g_WindowHandle, &clientRect);

    BITMAPINFO bmi = {
        .bmiHeader = {
            .biSize        = sizeof(BITMAPINFOHEADER),
            .biWidth       = width,
            .biHeight      = -height,
            .biPlanes      = 1,
            .biBitCount    = 32,
            .biCompression = BI_RGB,
        }
    };

    StretchDIBits(
        hdc, 
        0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top,
        0, 0, width, height,
        buffer, &bmi, DIB_RGB_COLORS, SRCCOPY
    );

    ReleaseDC(g_WindowHandle, hdc);

	Sleep(16);
}

void platform_destroy_window(void) {
    // win32 window cleanup is handled automatically via post-quit messages
    if (g_WindowHandle) DestroyWindow(g_WindowHandle);
}
