#include <windows.h>
#include <string>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct Ball {
    float x, y;
    float velX, velY;
    int size;
};

struct Paddle {
    float x, y;
    float width, height;
    float speed;
};

Ball ball = { 425, 250, 0.5f, 0.5f, 20 };
Paddle paddleLeft = { 25, 200, 20, 100, 1 };
Paddle paddleRight = { 805, 200, 20, 100, 1 };

int scoreLeft = 0;
int scoreRight = 0;

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

        // Mover la bola
        ball.x += ball.velX;
        ball.y += ball.velY;

        // Colisiones con las paredes superior e inferior
        if (ball.y < 0 || ball.y > 480) {
            ball.velY *= -1;
        }

        // Colisiones con las paletas
        if (ball.x < paddleLeft.x + paddleLeft.width && ball.y > paddleLeft.y && ball.y < paddleLeft.y + paddleLeft.height) {
            ball.velX *= -1;
            // Ajustar la dirección de la bola según la posición de la colisión en la paleta
            float hitPos = (ball.y - paddleLeft.y) / paddleLeft.height - 0.5f;
            ball.velY = hitPos * 1.5f;
        }
        if (ball.x > paddleRight.x - ball.size && ball.y > paddleRight.y && ball.y < paddleRight.y + paddleRight.height) {
            ball.velX *= -1;
            // Ajustar la dirección de la bola según la posición de la colisión en la paleta
            float hitPos = (ball.y - paddleRight.y) / paddleRight.height - 0.5f;
            ball.velY = hitPos * 1.5f;
        }

        // Puntos
        if (ball.x < 0) {
            scoreRight++;
            ball.x = 425;
            ball.y = 250;
            ball.velX = 0.5f;
            ball.velY = 0.5f;
        }
        if (ball.x > 840) {
            scoreLeft++;
            ball.x = 425;
            ball.y = 250;
            ball.velX = -0.5f;
            ball.velY = 0.5f;
        }

        // Movimiento de las paletas
        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            if (paddleRight.y > 0) paddleRight.y -= paddleRight.speed;
        }
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            if (paddleRight.y < 400) paddleRight.y += paddleRight.speed;
        }
        if (GetAsyncKeyState('W') & 0x8000) {
            if (paddleLeft.y > 0) paddleLeft.y -= paddleLeft.speed;
        }
        if (GetAsyncKeyState('S') & 0x8000) {
            if (paddleLeft.y < 400) paddleLeft.y += paddleLeft.speed;
        }

        // Redibujar la ventana
        InvalidateRect(hwnd, nullptr, TRUE);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HDC hdcBackBuffer = nullptr;
    static HBITMAP hbmBackBuffer = nullptr;
    static HBITMAP hbmOldBuffer = nullptr;
    static int width, height;

    switch (uMsg) {
        case WM_SIZE: {
            if (hdcBackBuffer) {
                SelectObject(hdcBackBuffer, hbmOldBuffer);
                DeleteObject(hbmBackBuffer);
                DeleteDC(hdcBackBuffer);
            }

            width = LOWORD(lParam);
            height = HIWORD(lParam);

            HDC hdc = GetDC(hwnd);
            hdcBackBuffer = CreateCompatibleDC(hdc);
            hbmBackBuffer = CreateCompatibleBitmap(hdc, width, height);
            hbmOldBuffer = (HBITMAP)SelectObject(hdcBackBuffer, hbmBackBuffer);
            ReleaseDC(hwnd, hdc);
        } return 0;

        case WM_DESTROY: {
            if (hdcBackBuffer) {
                SelectObject(hdcBackBuffer, hbmOldBuffer);
                DeleteObject(hbmBackBuffer);
                DeleteDC(hdcBackBuffer);
            }
            PostQuitMessage(0);
        } return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Fondo blanco
            FillRect(hdcBackBuffer, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW+1));

            // Dibuja la bola
            Ellipse(hdcBackBuffer, (int)ball.x, (int)ball.y, (int)ball.x + ball.size, (int)ball.y + ball.size);

            // Dibuja las paletas
            Rectangle(hdcBackBuffer, (int)paddleLeft.x, (int)paddleLeft.y, (int)paddleLeft.x + (int)paddleLeft.width, (int)paddleLeft.y + (int)paddleLeft.height);
            Rectangle(hdcBackBuffer, (int)paddleRight.x, (int)paddleRight.y, (int)paddleRight.x + (int)paddleRight.width, (int)paddleRight.y + (int)paddleRight.height);

            // Dibuja el marcador
            std::string scoreText = std::to_string(scoreLeft) + " - " + std::to_string(scoreRight);
            TextOut(hdcBackBuffer, 400, 10, scoreText.c_str(), scoreText.length());

            // Copiar el buffer a la pantalla
            BitBlt(hdc, 0, 0, width, height, hdcBackBuffer, 0, 0, SRCCOPY);

            EndPaint(hwnd, &ps);
        } return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

