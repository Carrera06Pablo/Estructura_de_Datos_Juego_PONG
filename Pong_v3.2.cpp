#include <SDL2/SDL.h>
#include <iostream>

// Constantes del juego
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PADDLE_WIDTH = 20;
const int PADDLE_HEIGHT = 100;
const int BALL_SIZE = 20;

// Variables de la paleta
int paddle1Y = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
int paddle2Y = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
int paddleSpeed = 10;

// Variables de la pelota
int ballX = (SCREEN_WIDTH - BALL_SIZE) / 2;
int ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2;
int ballSpeedX = 5;
int ballSpeedY = 5;

bool init(SDL_Window** window, SDL_Renderer** renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL no se pudo inicializar. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    *window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (*window == NULL) {
        std::cout << "La ventana no se pudo crear. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) {
        std::cout << "El renderizador no se pudo crear. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void close(SDL_Window* window, SDL_Renderer* renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void handleInput(bool& quit, const Uint8* keystates) {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        }
    }

    // Movimiento de la paleta 1
    if (keystates[SDL_SCANCODE_W] && paddle1Y > 0) {
        paddle1Y -= paddleSpeed;
    }
    if (keystates[SDL_SCANCODE_S] && paddle1Y < SCREEN_HEIGHT - PADDLE_HEIGHT) {
        paddle1Y += paddleSpeed;
    }

    // Movimiento de la paleta 2
    if (keystates[SDL_SCANCODE_UP] && paddle2Y > 0) {
        paddle2Y -= paddleSpeed;
    }
    if (keystates[SDL_SCANCODE_DOWN] && paddle2Y < SCREEN_HEIGHT - PADDLE_HEIGHT) {
        paddle2Y += paddleSpeed;
    }
}

void updateBall() {
    ballX += ballSpeedX;
    ballY += ballSpeedY;

    // Colisiones con los bordes superior e inferior
    if (ballY <= 0 || ballY >= SCREEN_HEIGHT - BALL_SIZE) {
        ballSpeedY = -ballSpeedY;
    }

    // Colisiones con las paletas
    if ((ballX <= PADDLE_WIDTH && ballY + BALL_SIZE >= paddle1Y && ballY <= paddle1Y + PADDLE_HEIGHT) ||
        (ballX + BALL_SIZE >= SCREEN_WIDTH - PADDLE_WIDTH && ballY + BALL_SIZE >= paddle2Y && ballY <= paddle2Y + PADDLE_HEIGHT)) {
        ballSpeedX = -ballSpeedX;
    }

    // Reiniciar la pelota si sale de los límites
    if (ballX < 0 || ballX > SCREEN_WIDTH) {
        ballX = (SCREEN_WIDTH - BALL_SIZE) / 2;
        ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2;
        ballSpeedX = -ballSpeedX;
    }
}

void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Renderizar paletas
    SDL_Rect paddle1 = { 0, paddle1Y, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_Rect paddle2 = { SCREEN_WIDTH - PADDLE_WIDTH, paddle2Y, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_RenderFillRect(renderer, &paddle1);
    SDL_RenderFillRect(renderer, &paddle2);

    // Renderizar pelota
    SDL_Rect ball = { ballX, ballY, BALL_SIZE, BALL_SIZE };
    SDL_RenderFillRect(renderer, &ball);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* args[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if (!init(&window, &renderer)) {
        std::cout << "Fallo en la inicialización." << std::endl;
        return -1;
    }

    bool quit = false;
    const Uint8* keystates = SDL_GetKeyboardState(NULL);

    while (!quit) {
        handleInput(quit, keystates);
        updateBall();
        render(renderer);

        SDL_Delay(16);  // Aproximadamente 60 FPS
    }

    close(window, renderer);
    return 0;
}
