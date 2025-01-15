#include "controls.h"
#include "keyboard.h"

void init_controls(controls_t *controls, control_bind_callback_t callback) {
    // Initialize controls with emtpy values
    controls->forward = (action_t){{}, 0, 0};
    controls->back = (action_t){{}, 0, 0};
    controls->left = (action_t){{}, 0, 0};
    controls->right = (action_t){{}, 0, 0};

    // Let the user-defined callback customize the controls
    if (callback) {
        callback(controls);
    }
}

bool action_key_down(action_t action) {
    for (int i = 0; i < action.numKeys; i++) {
        if (keyboard_down(action.keys[i])) {
            return true;
        }
    }
    return false;
}

