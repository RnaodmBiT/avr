#include <SDL.h>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <unistd.h>

int main(int argc, char** argv) {

    SDL_Init(SDL_INIT_EVERYTHING);
    atexit(SDL_Quit);


    SDL_Window* window = SDL_CreateWindow("Remote GUI",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          1280, 720, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* buffer = SDL_CreateTexture(renderer,
                                            SDL_PIXELFORMAT_RGB888,
                                            SDL_TEXTUREACCESS_TARGET,
                                            128, 72);


    SDL_Point direction = { 0, 0 };

    FILE* remote = fopen("/dev/ttyACM0", "w");
    if (remote) {
        printf("Found remote\n");
    } else {
        printf("No remote attached\n");
    }

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }

            if (event.type == SDL_JOYDEVICEADDED) {
                SDL_JoystickOpen(event.jdevice.which);
            }

            if (event.type == SDL_JOYDEVICEREMOVED) {
                direction = { 0, 0 };
            }

            if (event.type == SDL_JOYAXISMOTION) {
                if (event.jaxis.axis == 1) {
                    direction.y = event.jaxis.value / 256;
                }
                if (event.jaxis.axis == 3) {
                    direction.x = event.jaxis.value / 256;
                }
            }
        }

        // clamp the direction values
        int left = -direction.y + direction.x;
        int right = -direction.y - direction.x;

        if (SDL_abs(left) < 30) left = 0;
        if (SDL_abs(right) < 30) right = 0;

        left = SDL_max(SDL_min(left, 127), -127);
        right = SDL_max(SDL_min(right, 127), -127);

        // write the direction out to the remote
        if (remote) {
            fprintf(remote, "%hhi,%hhi\n", left, right);
        }

        // Draw to the buffer texture
        SDL_SetRenderTarget(renderer, buffer);
        SDL_SetRenderDrawColor(renderer, 0x64, 0x95, 0xed, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawLine(renderer, 64 - 32, 36 - 32, 64 + 32, 36 - 32);
        SDL_RenderDrawLine(renderer, 64 + 32, 36 - 32, 64 + 32, 36 + 32);
        SDL_RenderDrawLine(renderer, 64 + 32, 36 + 32, 64 - 31, 36 + 32);
        SDL_RenderDrawLine(renderer, 64 - 32, 36 + 32, 64 - 32, 36 - 31);
        SDL_RenderDrawLine(renderer, 64, 36, 64 + direction.x / 4, 36 + direction.y / 4);

        // Blit the buffer over the screen
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderCopy(renderer, buffer, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        usleep(10000);
    }

    return 0;
}
