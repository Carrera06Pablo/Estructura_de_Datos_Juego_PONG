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

Ball ball = { 425, 250, 1.5f, 1.5f, 20 };
Paddle paddleLeft = { 25, 200, 20, 100, 2 };
Paddle paddleRight = { 805, 200, 20, 100, 2 };

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
            ball.velY = hitPos * 4;
        }
        if (ball.x > paddleRight.x - ball.size && ball.y > paddleRight.y && ball.y < paddleRight.y + paddleRight.height) {
            ball.velX *= -1;
            // Ajustar la dirección de la bola según la posición de la colisión en la paleta
            float hitPos = (ball.y - paddleRight.y) / paddleRight.height - 0.5f;
            ball.velY = hitPos * 4;
        }

        // Puntos
        if (ball.x < 0) {
            scoreRight++;
            ball.x = 425;
            ball.y = 250;
            ball.velX = 1.5f;
            ball.velY = 1.5f;
        }
        if (ball.x > 840) {
            scoreLeft++;
            ball.x = 425;
            ball.y = 250;
            ball.velX = -1.5f;
            ball.velY = 1.5f;
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
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Fondo blanco
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW+1));

            // Dibuja la bola
            Ellipse(hdc, (int)ball.x, (int)ball.y, (int)ball.x + ball.size, (int)ball.y + ball.size);

            // Dibuja las paletas
            Rectangle(hdc, (int)paddleLeft.x, (int)paddleLeft.y, (int)paddleLeft.x + (int)paddleLeft.width, (int)paddleLeft.y + (int)paddleLeft.height);
            Rectangle(hdc, (int)paddleRight.x, (int)paddleRight.y, (int)paddleRight.x + (int)paddleRight.width, (int)paddleRight.y + (int)paddleRight.height);

            // Dibuja el marcador
            std::string scoreText = std::to_string(scoreLeft) + " - " + std::to_string(scoreRight);
            TextOut(hdc, 400, 10, scoreText.c_str(), scoreText.length());

            EndPaint(hwnd, &ps);
        } return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

