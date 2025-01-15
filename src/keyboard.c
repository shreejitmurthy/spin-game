#include "keyboard.h"

void keyboard_init() {
    for (int i = 0; i < 322; i++) {
        KEYS[i] = false;
    }
}

void keyboard_process(SDL_Event event) {
    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
        if (event.key.scancode < SDL_SCANCODE_COUNT) {
            KEYS[event.key.scancode] = (event.type == SDL_EVENT_KEY_DOWN);
        }
    }
}

bool keyboard_down(uint32_t key) {
    SDL_Scancode scancode = SDL_GetScancodeFromKey(key, 0);
    return (scancode < SDL_SCANCODE_COUNT && KEYS[scancode]);
}