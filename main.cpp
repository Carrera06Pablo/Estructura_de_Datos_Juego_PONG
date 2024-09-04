#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include "pong.h"



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
        showCountdown(renderer, font);

        bool gameRunning = true;

        while (gameRunning) {
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    gameRunning = false;
                    quit = true;
                }
            }

            // Verifica si han pasado los 3 segundos de congelamiento para descongelar
            if (player1Frozen && SDL_GetTicks() - powerUpStartTime >= FREEZE_DURATION) {
                player1Frozen = false;
            }
            if (player2Frozen && SDL_GetTicks() - powerUpStartTime >= FREEZE_DURATION) {
                player2Frozen = false;
            }

            // Controla la aparición del power-up solo una vez
            if (SDL_GetTicks() >= 6000 && powerUpCount < MAX_POWERUPS && !powerUpActive) {
                powerUpX = rand() % (SCREEN_WIDTH - POWERUP_SIZE);
                powerUpY = rand() % (SCREEN_HEIGHT - POWERUP_SIZE);
                powerUpActive = true;
                powerUpCount++; // Marca el power-up como usado
            }

            const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
            if (!player1Frozen) {
            if (currentKeyStates[SDL_SCANCODE_W]) {
                paddle1Y -= paddleSpeed;
                if (paddle1Y < 0) paddle1Y = 0;
            }
            if (currentKeyStates[SDL_SCANCODE_S]) {
                paddle1Y += paddleSpeed;
                if (paddle1Y > SCREEN_HEIGHT - PADDLE_HEIGHT) paddle1Y = SCREEN_HEIGHT - PADDLE_HEIGHT;
            }
            }
            if (!playAgainstAI) {
                    if (!player2Frozen) {
                if (currentKeyStates[SDL_SCANCODE_UP]) {
                    paddle2Y -= paddleSpeed;
                    if (paddle2Y < 0) paddle2Y = 0;
                }
                if (currentKeyStates[SDL_SCANCODE_DOWN]) {
                    paddle2Y += paddleSpeed;
                    if (paddle2Y > SCREEN_HEIGHT - PADDLE_HEIGHT) paddle2Y = SCREEN_HEIGHT - PADDLE_HEIGHT;
                }
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


