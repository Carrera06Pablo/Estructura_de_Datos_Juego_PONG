#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>
#include <cstdlib>  // Necesario para rand()


// Constantes de la pantalla
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
float velocidadPaletaJugador = 6.0f;
float velocidadPaletaMaquina = 6.0f;

// Constantes de la pelota
const int BALL_SIZE = 12;
int ballX, ballY;
int ballSpeedX = 6, ballSpeedY = 6;


// Constantes de las paletas
const int PADDLE_WIDTH = 15;
const int PADDLE_HEIGHT = 100;
int paddle1Y, paddle2Y;
int paddleSpeed = 10;

// Puntuaci�n
int score1 = 0, score2 = 0;
const int WINNING_SCORE = 7;

// Variables de audio
SDL_AudioSpec wavSpecPaddle, wavSpecScore, wavSpecImpact, wavSpecMenu;
Uint32 wavLengthPaddle, wavLengthScore, wavLengthImpact, wavLengthMenu;
Uint8 *wavBufferPaddle, *wavBufferScore, *wavBufferImpact, *wavBufferMenu;
SDL_AudioDeviceID deviceIdPaddle, deviceIdScore, deviceIdImpact, deviceIdMenu;

// Modo de juego
bool playAgainstAI = false;

// Declaración de funciones
void renderTextWithColor(SDL_Renderer* renderer, TTF_Font* font, std::string text, int x, int y, SDL_Color color);

bool powerUpActive = false;
bool powerUpUsed = false;
bool player1Frozen = false;
bool player2Frozen = false;
int powerUpX, powerUpY;
Uint32 powerUpStartTime = 0;
const int POWERUP_SIZE = 20;
const int FREEZE_DURATION = 3000; // Duración del congelamiento en milisegundos
int powerUpCount = 0;  // Contador para los power-ups
const int MAX_POWERUPS = 2;  // Número máximo de power-ups que aparecerán

extern int lastPlayerHit = 0;

// Inicializaci�n de SDL, la ventana, el renderizador y las fuentes
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

    *font = TTF_OpenFont("ka1.ttf", 20);  // Reducir el tama�o de la fuente
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

    // Cargar el archivo de sonido para el menú
    if (SDL_LoadWAV("menu.wav", &wavSpecMenu, &wavBufferMenu, &wavLengthMenu) == NULL) {
        std::cout << "No se pudo cargar menu.wav. SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    deviceIdMenu = SDL_OpenAudioDevice(NULL, 0, &wavSpecMenu, NULL, 0);

    if (IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG) == 0) {
    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
    // Manejo de errores
    }

    return true;
}
void initGame() {
    // Inicializar posiciones de la pelota y las paletas
    ballX = (SCREEN_WIDTH - BALL_SIZE) / 2;
    ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2;
    paddle1Y = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
    paddle2Y = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;

    // Restablecer las velocidades de la pelota
    ballSpeedX = 10;
    ballSpeedY = 10;

    powerUpX = -POWERUP_SIZE;
    powerUpY = -POWERUP_SIZE;

    // Restablecer las puntuaciones
    score1 = 0;
    score2 = 0;
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

// Actualizar la posici�n de la pelota
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
    if ((ballX <= PADDLE_WIDTH && ballY + BALL_SIZE >= paddle1Y && ballY <= paddle1Y + PADDLE_HEIGHT)) {
        ballSpeedX = -ballSpeedX;
        lastPlayerHit = 1; // La pelota fue golpeada por el jugador 1
        SDL_QueueAudio(deviceIdPaddle, wavBufferPaddle, wavLengthPaddle);
        SDL_PauseAudioDevice(deviceIdPaddle, 0);  // Reproducir sonido al golpear la paleta
    }else if (ballX + BALL_SIZE >= SCREEN_WIDTH - PADDLE_WIDTH && ballY + BALL_SIZE >= paddle2Y && ballY <= paddle2Y + PADDLE_HEIGHT) {
    ballSpeedX = -ballSpeedX;
    lastPlayerHit = 2; // La pelota fue golpeada por el jugador 2 o IA
    SDL_QueueAudio(deviceIdPaddle, wavBufferPaddle, wavLengthPaddle);
    SDL_PauseAudioDevice(deviceIdPaddle, 0);
    }

    // Colisión con el power-up
    if (powerUpActive && ballX + BALL_SIZE >= powerUpX && ballX <= powerUpX + POWERUP_SIZE && ballY + BALL_SIZE >= powerUpY && ballY <= powerUpY + POWERUP_SIZE) {
    if (lastPlayerHit == 1) {
        player2Frozen = true; // Congela al jugador 2 o IA
    } else if (lastPlayerHit == 2) {
        player1Frozen = true; // Congela al jugador 1
    }
    powerUpActive = false;
    powerUpStartTime = SDL_GetTicks();
    powerUpX = -POWERUP_SIZE;
    powerUpY = -POWERUP_SIZE;
}


    // Reiniciar la pelota y actualizar el marcador si sale de los l�mites
    if (ballX < 0) {
        score2++;
        ballX = (SCREEN_WIDTH - BALL_SIZE) / 2;
        ballY = 1;
        ballSpeedX = -ballSpeedX;
        SDL_QueueAudio(deviceIdScore, wavBufferScore, wavLengthScore);
        SDL_PauseAudioDevice(deviceIdScore, 0);  // Reproducir sonido al marcar un punto
    } else if (ballX > SCREEN_WIDTH) {
        score1++;
        ballX = (SCREEN_WIDTH - BALL_SIZE) / 2;
        ballY = 1;
        ballSpeedX = -ballSpeedX;
        SDL_QueueAudio(deviceIdScore, wavBufferScore, wavLengthScore);
        SDL_PauseAudioDevice(deviceIdScore, 0);  // Reproducir sonido al marcar un punto
    }
}

void renderPowerUp(SDL_Renderer* renderer) {
    if (powerUpActive) {
        SDL_Rect powerUpRect = {powerUpX, powerUpY, POWERUP_SIZE, POWERUP_SIZE};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Verde
        SDL_RenderFillRect(renderer, &powerUpRect);
    }
}

// Comprobar si alguien ha ganado
bool checkWinCondition(SDL_Renderer* renderer, TTF_Font* font) {
    if (score1 == WINNING_SCORE || score2 == WINNING_SCORE) {
        // Detener todos los sonidos en curso
        SDL_PauseAudioDevice(deviceIdPaddle, 1);
        SDL_PauseAudioDevice(deviceIdScore, 1);
        SDL_PauseAudioDevice(deviceIdImpact, 1);
        TTF_Font* smallFont = TTF_OpenFont("ka1.ttf", 30);
        SDL_Color Red = {0, 0, 0};
        std::string winnerText = (score1 == WINNING_SCORE) ? "Jugador 1 Gana!" : "Jugador 2 Gana!";
        renderTextWithColor(renderer, smallFont, winnerText, SCREEN_WIDTH / 4.5, SCREEN_HEIGHT / 2, Red);
        SDL_RenderPresent(renderer);
        SDL_Delay(3000); // Esperar 3 segundos antes de volver al menú
        return true;
    }
    return false;
}


void updateAIPaddle() {
    if (player2Frozen) {
        // Si el jugador 2 está congelado, no hacer nada
        return;
    }
    // Genera un número aleatorio entre 0 y 99
    int randomFailChance = rand() % 100;

    // Introducir un fallo si el número aleatorio está dentro de un rango (por ejemplo, 10% de fallos)
    if (randomFailChance < 10) {  // Ahora solo fallará el 10% de las veces
        // Fallo: la paleta se mueve en la dirección incorrecta pero de forma más suave
        if (ballY < paddle2Y + PADDLE_HEIGHT / 2) {
            paddle2Y += paddleSpeed / 2;  // Movimiento incorrecto, pero más suave
        } else if (ballY > paddle2Y + PADDLE_HEIGHT / 2) {
            paddle2Y -= paddleSpeed / 2;  // Movimiento incorrecto, pero más suave
        }
    } else {
        // Comportamiento normal
        if (ballY < paddle2Y + PADDLE_HEIGHT / 2) {
            paddle2Y -= paddleSpeed * 0.8;  // Movimiento más fluido (un poco más lento)
        } else if (ballY > paddle2Y + PADDLE_HEIGHT / 2) {
            paddle2Y += paddleSpeed * 0.8;  // Movimiento más fluido (un poco más lento)
        }
    }
}

// Renderizar el juego
void render(SDL_Renderer* renderer, TTF_Font* font) {

    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderClear(renderer);

    // Establece el color
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    int x = 320; // Centro horizontal de la ventana (640 / 2)
    int y1 = 0;  // Punto de inicio en la parte superior
    int y2 = 480; // Punto de fin en la parte inferior
    int thickness = 12; // Grosor de la línea

    // Dibuja la línea blanca
    for (int i = -thickness / 2; i <= thickness / 2; ++i) {
        SDL_RenderDrawLine(renderer, x + i, y1, x + i, y2);
    }

    // Renderizar la pelota
    SDL_Rect ballRect = {ballX, ballY, BALL_SIZE, BALL_SIZE};
    SDL_RenderFillRect(renderer, &ballRect);

    // Renderizar las paletas
    SDL_Rect paddle1Rect = {0, paddle1Y, PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_RenderFillRect(renderer, &paddle1Rect);

    SDL_Rect paddle2Rect = {SCREEN_WIDTH - PADDLE_WIDTH, paddle2Y, PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_RenderFillRect(renderer, &paddle2Rect);

    // Renderizar el marcador
    renderText(renderer, font, std::to_string(score1), SCREEN_WIDTH / 4, 20);
    renderText(renderer, font, std::to_string(score2), SCREEN_WIDTH * 3 / 4, 20);

    // Dibujar power-up
    renderPowerUp(renderer);

    SDL_RenderPresent(renderer);
}

// Mostrar las instrucciones del juego
void showInstructions(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Event e;
    bool waiting = true;

    while (waiting) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        TTF_Font* smallFont = TTF_OpenFont("ka1.ttf", 16);
        SDL_Color Red = {255, 0, 0};
        renderTextWithColor(renderer, smallFont, "Instrucciones:", 35, 50,Red);
        renderTextWithColor(renderer, smallFont, "Modo 1 Jugador:", 35, 80, Red);
        renderText(renderer, smallFont, "Controla la paleta izquierda con W y S.", 50, 110);
        renderText(renderer, smallFont, "La paleta derecha es controlada por la", 50, 140);
        renderText(renderer, smallFont, "computadora.", 50, 170);

        renderTextWithColor(renderer, smallFont, "Modo 2 Jugadores:", 35, 200, Red);
        renderText(renderer, smallFont, "Jugador 1: Usa W y S para mover la paleta", 50, 230);
        renderText(renderer, smallFont, "izquierda.", 50, 260);
        renderText(renderer, smallFont, "Jugador 2: Usa UP y DOWN para mover la paleta", 50, 290);
        renderText(renderer, smallFont, "derecha.", 50, 320);

        renderTextWithColor(renderer, smallFont, "El primer jugador en alcanzar 7 puntos", 35, 350, Red);
        renderTextWithColor(renderer, smallFont, "gana el juego.", 35, 380, Red);
        renderText(renderer, smallFont, "Presione cualquier tecla para regresar al menu.", 35, 410);

        TTF_CloseFont(smallFont);

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

// Definir niveles de dificultad
enum Dificultad { FACIL, MEDIO, DIFICIL };
Dificultad dificultadSeleccionada = DIFICIL;
bool showDifficultyMenu(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Event e;
    bool selected = false;

    // Definir las áreas de las opciones del submenú de dificultad
    SDL_Rect easyRect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3, 400, 30};
    SDL_Rect mediumRect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3 + 40, 400, 30};
    SDL_Rect hardRect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3 + 80, 400, 30};

    while (!selected) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Obtener la posición actual del mouse
        int x, y;
        SDL_GetMouseState(&x, &y);
        SDL_Point mousePoint = {x, y};

        // Comprobar si el mouse está sobre alguna opción y cambiar su color
        SDL_Color easyColor = SDL_PointInRect(&mousePoint, &easyRect) ? SDL_Color{255, 0, 0} : SDL_Color{255, 255, 255};
        SDL_Color mediumColor = SDL_PointInRect(&mousePoint, &mediumRect) ? SDL_Color{255, 0, 0} : SDL_Color{255, 255, 255};
        SDL_Color hardColor = SDL_PointInRect(&mousePoint, &hardRect) ? SDL_Color{255, 0, 0} : SDL_Color{255, 255, 255};

        // Renderizar las opciones con el color correspondiente
        renderTextWithColor(renderer, font, "Seleccione la dificultad", SCREEN_WIDTH / 4.5, SCREEN_HEIGHT / 5, SDL_Color{255, 0, 0});
        renderTextWithColor(renderer, font, "                      Facil", easyRect.x, easyRect.y, easyColor);
        renderTextWithColor(renderer, font, "                      Medio", mediumRect.x, mediumRect.y, mediumColor);
        renderTextWithColor(renderer, font, "                     Dificil", hardRect.x, hardRect.y, hardColor);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                return false;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (SDL_PointInRect(&mousePoint, &easyRect)) {
                    dificultadSeleccionada = FACIL;
                    selected = true;
                } else if (SDL_PointInRect(&mousePoint, &mediumRect)) {
                    dificultadSeleccionada = MEDIO;
                    selected = true;
                } else if (SDL_PointInRect(&mousePoint, &hardRect)) {
                    dificultadSeleccionada = DIFICIL;
                    selected = true;
                }
            }
        }
    }
    return true;
}

// Función para mostrar el menú
bool showMenu(SDL_Renderer* renderer, TTF_Font* font) {

    // Reproducir el sonido de fondo del menú
    SDL_ClearQueuedAudio(deviceIdMenu);
    SDL_QueueAudio(deviceIdMenu, wavBufferMenu, wavLengthMenu);
    SDL_PauseAudioDevice(deviceIdMenu, 0);

    // Pausar todos los sonidos en curso
    SDL_PauseAudioDevice(deviceIdPaddle, 1);
    SDL_PauseAudioDevice(deviceIdScore, 1);
    SDL_PauseAudioDevice(deviceIdImpact, 1);

    SDL_Event e;
    bool selected = false;

    // Definir el ancho y alto de cada opción
    int optionWidth = 400;
    int optionHeight = 30;

    // Calcular la posición X centrada
    int centerX = (640 - optionWidth) / 2;

    // Establecer la posición Y inicial a 230 píxeles desde la parte superior
    int startY = 235;

    // Definir las áreas de las opciones del menú
    SDL_Rect option1Rect = {centerX, startY, optionWidth, optionHeight};
    SDL_Rect option2Rect = {centerX, startY + 40, optionWidth, optionHeight};
    SDL_Rect option3Rect = {centerX, startY + 80, optionWidth, optionHeight};
    SDL_Rect option4Rect = {centerX, startY + 120, optionWidth, optionHeight};

    // Cargar la imagen de fondo
    SDL_Surface* bgSurface = IMG_Load("fondo_menu.png");

        if (!bgSurface) {
            printf("Error loading background image: %s\n", IMG_GetError());
            IMG_Quit(); // Finalizar SDL_image si ocurre un error
        return false;
        }
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

        if (!bgTexture) {
            printf("Error creating texture from surface: %s\n", SDL_GetError());
            IMG_Quit(); // Finalizar SDL_image si ocurre un error
            return false;
        }

    while (!selected) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Renderizar la imagen de fondo
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

        // Obtener la posición actual del mouse
        int x, y;
        SDL_GetMouseState(&x, &y);
        SDL_Point mousePoint = {x, y};

        // Comprobar si el mouse está sobre alguna opción y cambiar su color
        SDL_Color option1Color = SDL_PointInRect(&mousePoint, &option1Rect) ? SDL_Color{255, 0, 0} : SDL_Color{255, 255, 255};
        SDL_Color option2Color = SDL_PointInRect(&mousePoint, &option2Rect) ? SDL_Color{255, 0, 0} : SDL_Color{255, 255, 255};
        SDL_Color option3Color = SDL_PointInRect(&mousePoint, &option3Rect) ? SDL_Color{255, 0, 0} : SDL_Color{255, 255, 255};
        SDL_Color option4Color = SDL_PointInRect(&mousePoint, &option4Rect) ? SDL_Color{255, 0, 0} : SDL_Color{255, 255, 255};

        // Renderizar las opciones con el color correspondiente
        renderTextWithColor(renderer, font, "                         Jugar", option1Rect.x, option1Rect.y, option1Color);
        renderTextWithColor(renderer, font, "                Multijugador", option2Rect.x, option2Rect.y, option2Color);
        renderTextWithColor(renderer, font, "                Instrucciones", option3Rect.x, option3Rect.y, option3Color);
        renderTextWithColor(renderer, font, "                          Salir", option4Rect.x, option4Rect.y, option4Color);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                SDL_DestroyTexture(bgTexture); // Liberar recursos
                return false;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (SDL_PointInRect(&mousePoint, &option2Rect)) {
                    playAgainstAI = false;
                    selected = true;
                } else if (SDL_PointInRect(&mousePoint, &option1Rect)) {
                    playAgainstAI = true;
                    selected = showDifficultyMenu(renderer, font); // Mostrar submenú de dificultad
                    // Ajustar la velocidad de la paleta según la dificultad seleccionada
                    if (selected) {
                        switch (dificultadSeleccionada) {
                            case FACIL:
                                paddleSpeed = 7;  // Velocidad baja para fácil
                                break;
                            case MEDIO:
                                paddleSpeed = 8;  // Velocidad media para medio
                                break;
                            case DIFICIL:
                                paddleSpeed = 9;  // Velocidad alta para difícil
                                break;
                        }
                    }
                } else if (SDL_PointInRect(&mousePoint, &option3Rect)) {
                    showInstructions(renderer, font);
                } else if (SDL_PointInRect(&mousePoint, &option4Rect)) {
                    SDL_DestroyTexture(bgTexture); // Liberar recursos
                    SDL_PauseAudioDevice(deviceIdMenu, 1); // Pausar el audio del menú

                    return false;
                } else {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderClear(renderer);
                    renderText(renderer, font, "Opción incorrecta, intente de nuevo.", SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2);
                    SDL_RenderPresent(renderer);
                }
            }
        }
    }
    SDL_DestroyTexture(bgTexture); // Liberar recursos
    SDL_PauseAudioDevice(deviceIdMenu, 1); // Pausar el audio del menú

    return true;
}


// Función para renderizar texto con un color específico
void renderTextWithColor(SDL_Renderer* renderer, TTF_Font* font, std::string text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect destRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}


// Reiniciar variables
void resetGame() {
    paddle1Y = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
    paddle2Y = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
    ballX = (SCREEN_WIDTH - BALL_SIZE) / 2;
    ballY = 0;
    ballSpeedX = 6;
    ballSpeedY = 6;
    score1 = 0;
    score2 = 0;

    // Reiniciar los dispositivos de audio
    SDL_ClearQueuedAudio(deviceIdPaddle);
    SDL_ClearQueuedAudio(deviceIdScore);
    SDL_ClearQueuedAudio(deviceIdImpact);

    powerUpActive = false;
    powerUpUsed = false;
    player1Frozen = false;
    player2Frozen = false;
    powerUpX = -POWERUP_SIZE;
    powerUpY = -POWERUP_SIZE;
}


void showCountdown(SDL_Renderer* renderer, TTF_Font* font) {
    const int countdownTime = 1000; // 1000 ms = 1 segundo
    for (int i = 3; i > 0; --i) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        TTF_Font* smallFont = TTF_OpenFont("ka1.ttf", 60);

        renderText(renderer, smallFont, std::to_string(i) , SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT / 2 - 20);

        SDL_RenderPresent(renderer);
        SDL_Delay(countdownTime);
    }
}
