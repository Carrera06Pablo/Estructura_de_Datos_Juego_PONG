#include <windows.h>
#include <string>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char* CLASS_NAME = "PongWindowClass";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Pong",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 850, 500,
        nullptr, nullptr, hInstance, nullptr
    );

    if (hwnd == nullptr) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // Aquí es donde se dibujan la bola y las paletas.
            Ellipse(hdc, 400, 200, 450, 250); // Dibuja la bola
            Rectangle(hdc, 800, 200, 825, 300); // Dibuja la paleta derecha
            Rectangle(hdc, 25, 200, 50, 300); // Dibuja la paleta izquierda
            EndPaint(hwnd, &ps);
        } return 0;

        case WM_KEYDOWN:
            // Aquí se manejan los eventos del teclado
            if (wParam == VK_UP) {
                // Mueve la paleta derecha hacia arriba
            }
            if (wParam == VK_DOWN) {
                // Mueve la paleta derecha hacia abajo
            }
            if (wParam == 'W') {
                // Mueve la paleta izquierda hacia arriba
            }
            if (wParam == 'S') {
                // Mueve la paleta izquierda hacia abajo
            }
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

