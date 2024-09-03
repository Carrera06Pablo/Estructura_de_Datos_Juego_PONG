#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>

// Constantes de la pantalla
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Constantes de la pelota
const int BALL_SIZE = 15;

int ballX, ballY;
int ballSpeedX = 6, ballSpeedY = 6;

// Constantes de las paletas
const int PADDLE_WIDTH = 15;
const int PADDLE_HEIGHT = 100;
int paddle1Y, paddle2Y;
int paddleSpeed = 5;

// Puntuación


int score1 = 0, score2 = 0;
const int WINNING_SCORE = 10;

// Variables de audio


SDL_AudioSpec wavSpecPaddle, wavSpecScore, wavSpecImpact;
Uint32 wavLengthPaddle, wavLengthScore, wavLengthImpact;
Uint8 *wavBufferPaddle, *wavBufferScore, *wavBufferImpact;
SDL_AudioDeviceID deviceIdPaddle, deviceIdScore, deviceIdImpact;

// Modo de juego
bool playAgainstAI = false;

// Inicialización de SDL, la ventana, el renderizador y las fuentes
bool init(SDL_Window** window, SDL_Renderer** renderer, TTF_Font** font) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cout << "SDL no pudo inicializarse. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    *window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (*window == NULL) {
        std::cout << "La ventana no pudo ser creada. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);

    if (*renderer == NULL) {
        std::cout << "El renderizador no pudo ser creado. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cout << "SDL_ttf no pudo inicializarse. SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    *font = TTF_OpenFont("arial.ttf", 28);
    if (*font == NULL) {
        std::cout << "No se pudo cargar la fuente. SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    // Cargar el archivo de sonido para el golpe de la paleta
    if (SDL_LoadWAV("paddle.wav", &wavSpecPaddle, &wavBufferPaddle, &wavLengthPaddle) == NULL) {
        std::cout << "No se pudo cargar paddle.wav. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    deviceIdPaddle = SDL_OpenAudioDevice(NULL, 0, &wavSpecPaddle, NULL, 0);

    // Cargar el archivo de sonido para marcar un punto
    if (SDL_LoadWAV("score.wav", &wavSpecScore, &wavBufferScore, &wavLengthScore) == NULL) {
        std::cout << "No se pudo cargar score.wav. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    deviceIdScore = SDL_OpenAudioDevice(NULL, 0, &wavSpecScore, NULL, 0);

    // Cargar el archivo de sonido para el impacto con las bandas
    if (SDL_LoadWAV("impact.wav", &wavSpecImpact, &wavBufferImpact, &wavLengthImpact) == NULL) {
        std::cout << "No se pudo cargar impact.wav. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    deviceIdImpact = SDL_OpenAudioDevice(NULL, 0, &wavSpecImpact, NULL, 0);

    return true;
}

// Liberar los recursos


void close(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font) {
    // Liberar sonidos
    SDL_CloseAudioDevice(deviceIdPaddle);
    SDL_FreeWAV(wavBufferPaddle);

    SDL_CloseAudioDevice(deviceIdScore);
    SDL_FreeWAV(wavBufferScore);

    SDL_CloseAudioDevice(deviceIdImpact);
    SDL_FreeWAV(wavBufferImpact);

    // Liberar otros recursos
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

// Renderizar texto en pantalla

void renderText(SDL_Renderer* renderer, TTF_Font* font, std::string text, int x, int y) {
    SDL_Color color = {255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect destRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Actualizar la posición de la pelota

void updateBall() {
    ballX += ballSpeedX;
    ballY += ballSpeedY;

    // Colisiones con los bordes superior e inferior

    if (ballY <= 0 || ballY >= SCREEN_HEIGHT - BALL_SIZE) {
        ballSpeedY = -ballSpeedY;
        SDL_QueueAudio(deviceIdImpact, wavBufferImpact, wavLengthImpact);
        SDL_PauseAudioDevice(deviceIdImpact, 0);  // Reproducir sonido al golpear las bandas
    }

    // Colisiones con las paletas
    if ((ballX <= PADDLE_WIDTH && ballY + BALL_SIZE >= paddle1Y && ballY <= paddle1Y + PADDLE_HEIGHT) ||
        (ballX + BALL_SIZE >= SCREEN_WIDTH - PADDLE_WIDTH && ballY + BALL_SIZE >= paddle2Y && ballY <= paddle2Y + PADDLE_HEIGHT)) {
        ballSpeedX = -ballSpeedX;
        SDL_QueueAudio(deviceIdPaddle, wavBufferPaddle, wavLengthPaddle);
        SDL_PauseAudioDevice(deviceIdPaddle, 0);  // Reproducir sonido al golpear la paleta


    }

    // Reiniciar la pelota y actualizar el marcador si sale de los límites

    if (ballX < 0) {
        score2++;
        ballX = (SCREEN_WIDTH - BALL_SIZE) / 2;
        ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2;
        ballSpeedX = -ballSpeedX;
        SDL_QueueAudio(deviceIdScore, wavBufferScore, wavLengthScore);
        SDL_PauseAudioDevice(deviceIdScore, 0);  // Reproducir sonido al marcar un punto
    } else if (ballX > SCREEN_WIDTH) {
        score1++;
        ballX = (SCREEN_WIDTH - BALL_SIZE) / 2;
        ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2;
        ballSpeedX = -ballSpeedX;
        SDL_QueueAudio(deviceIdScore, wavBufferScore, wavLengthScore);
        SDL_PauseAudioDevice(deviceIdScore, 0);  // Reproducir sonido al marcar un punto
    }
}

// Comprobar si alguien ha ganadosadsa


bool checkWinCondition(SDL_Renderer* renderer, TTF_Font* font) {
    if (score1 == WINNING_SCORE || score2 == WINNING_SCORE) {
        std::string winnerText = (score1 == WINNING_SCORE) ? "Jugador 1 Gana!" : "Jugador 2 Gana!";
        renderText(renderer, font, winnerText, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2);
        SDL_RenderPresent(renderer);
        SDL_Delay(3000); // Esperar 3 segundos antes de volver al menú
        return true;
    }
    return false;
}

// Actualizar la posición de la paleta controlada por la computadora
void updateAIPaddle() {
    if (ballY < paddle2Y + PADDLE_HEIGHT / 2) {
        paddle2Y -= paddleSpeed;
    } else if (ballY > paddle2Y + PADDLE_HEIGHT / 2) {
        paddle2Y += paddleSpeed;
    }
}

// Renderizar el juego
void render(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);



    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);


    // Renderizar la pelota
    SDL_Rect ballRect = {ballX, ballY, BALL_SIZE, BALL_SIZE};
    SDL_RenderFillRect(renderer, &ballRect);

    // Renderizar las paletas
    SDL_Rect paddle1Rect = {0, paddle1Y, PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_RenderFillRect(renderer, &paddle1Rect);

    SDL_Rect paddle2Rect = {SCREEN_WIDTH - PADDLE_WIDTH, paddle2Y, PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_RenderFillRect(renderer, &paddle2Rect);

    // Renderizar la puntuación
    renderText(renderer, font, std::to_string(score1), SCREEN_WIDTH / 4, 20);
    renderText(renderer, font, std::to_string(score2), SCREEN_WIDTH * 3 / 4, 20);

    SDL_RenderPresent(renderer);
}

// Mostrar las instrucciones del juego
void showInstructions(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Event e;
    bool waiting = true;

    while (waiting) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderText(renderer, font, "Instrucciones:", 50, 50);

        renderText(renderer, font, "Lógica del Juego:", 50, 100);

        renderText(renderer, font, "Contra otro Jugador: Ambos jugadores", 50, 140);

        renderText(renderer, font, "controlan las paletas usando W/S para el", 50, 180);

        renderText(renderer, font, "jugador 1 y UP/DOWN para el jugador 2.", 50, 220);

        renderText(renderer, font, "Contra la Computadora: El jugador 1", 50, 260);

        renderText(renderer, font, "controla la paleta izquierda, mientras que", 50, 300);

        renderText(renderer, font, "la paleta derecha es controlada", 50, 340);

        renderText(renderer, font, "automaticamente.", 50, 380);
        renderText(renderer, font, "Presione cualquier tecla para volver al menu.", 50, 420);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e) != 0) {

            if (e.type == SDL_QUIT) {

                waiting = false;
                break;
            }
            if (e.type == SDL_KEYDOWN) {
                waiting = false;
            }
        }
    }
}

// Mostrar el menú principal
bool showMenu(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Event e;
    bool selected = false;

    while (!selected) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderText(renderer, font, "Bienvenido a Pong", SCREEN_WIDTH / 4, SCREEN_HEIGHT / 6);
        renderText(renderer, font, "1. Jugar contra otro jugador", SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3);
        renderText(renderer, font, "2. Jugar contra la computadora", SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3 + 40);
        renderText(renderer, font, "3. Instrucciones", SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3 + 80);
        renderText(renderer, font, "4. Salir", SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3 + 120);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                return false;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_1) {
                    playAgainstAI = false;
                    selected = true;
                } else if (e.key.keysym.sym == SDLK_2) {
                    playAgainstAI = true;
                    selected = true;
                } else if (e.key.keysym.sym == SDLK_3) {
                    showInstructions(renderer, font);
                } else if (e.key.keysym.sym == SDLK_4) {
                    return false;
                } else {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderClear(renderer);
                    renderText(renderer, font, "Opción incorrecta, ingrese de nuevo.", SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2);
                    SDL_RenderPresent(renderer);
                    SDL_Delay(1000);
                }
            }
        }
    }
    return true;
}

// Reiniciar variables
void resetGame() {
    paddle1Y = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
    paddle2Y = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
    ballX = (SCREEN_WIDTH - BALL_SIZE) / 2;
    ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2;
    ballSpeedX = 6;
    ballSpeedY = 6;
    score1 = 0;
    score2 = 0;
}

int main(int argc, char* args[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    TTF_Font* font = NULL;

    if (!init(&window, &renderer, &font)) {
        std::cout << "Fallo en la inicialización." << std::endl;
        return -1;
    }

    bool quit = false;
    while (!quit) {
        if (!showMenu(renderer, font)) {
            break;
        }

        resetGame();
        bool gameRunning = true;

        while (gameRunning) {
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    gameRunning = false;
                    quit = true;
                }
            }

            const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
            if (currentKeyStates[SDL_SCANCODE_W]) {
                paddle1Y -= paddleSpeed;
                if (paddle1Y < 0) paddle1Y = 0;
            }
            if (currentKeyStates[SDL_SCANCODE_S]) {
                paddle1Y += paddleSpeed;
                if (paddle1Y > SCREEN_HEIGHT - PADDLE_HEIGHT) paddle1Y = SCREEN_HEIGHT - PADDLE_HEIGHT;
            }
            if (!playAgainstAI) {
                if (currentKeyStates[SDL_SCANCODE_UP]) {
                    paddle2Y -= paddleSpeed;
                    if (paddle2Y < 0) paddle2Y = 0;
                }
                if (currentKeyStates[SDL_SCANCODE_DOWN]) {
                    paddle2Y += paddleSpeed;
                    if (paddle2Y > SCREEN_HEIGHT - PADDLE_HEIGHT) paddle2Y = SCREEN_HEIGHT - PADDLE_HEIGHT;
                }
            } else {
                updateAIPaddle();
            }

            updateBall();

            if (checkWinCondition(renderer, font)) {
                gameRunning = false;
            }

            render(renderer, font);
            SDL_Delay(10);
        }
    }


    close(window, renderer, font);

    return 0;
}
