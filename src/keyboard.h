#pragma once

#include <SDL3/SDL.h>
#include <stdbool.h>

bool KEYS[322];

void keyboard_init();
void keyboard_process(SDL_Event event);
bool keyboard_down(uint32_t key);