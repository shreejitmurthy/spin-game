#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define SOKOL_IMPL
#define SOKOL_GLCORE
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_time.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cglm/cglm.h>

bool KEYS[322];

uint64_t last_time = 0;

void keyboard_init() {
    for(int i = 0; i < 322; i++) {  // init them all to false
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

typedef struct camera_t {
    vec3 position;
    vec3 target;
    vec3 up;
    vec3 front;
    mat4 view;
    mat4 projection;
    float speed;
    float yaw;   // Horizontal rotation
    float pitch; // Vertical rotation
    float sensitivity; // Mouse sensitivity
} camera_t;

typedef struct camera_desc_t {
    vec3 position;
    vec3 target;
    vec3 up;

} camera_desc_t;

camera_t camera_init(vec3 position, vec3 target, vec3 up) {
    camera_t camera;
    glm_vec3_copy(position, camera.position);
    glm_vec3_copy(target, camera.target);
    glm_vec3_copy(up, camera.up);

    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, camera.front);

    // Calculate the view matrix
    glm_lookat(camera.position, camera.target, camera.up, camera.view);
    return camera;
}

void camera_move(camera_t* camera, vec3 movement) {
    vec3 delta;
    glm_vec3_scale(movement, camera->speed * (float)stm_sec(stm_laptime(&last_time)), delta);
    glm_vec3_add(camera->position, delta, camera->position);
    glm_vec3_add(camera->target, delta, camera->target);
}

void camera_handle_input(camera_t* camera) {
    vec3 forward, right, movement = {0.0f, 0.0f, 0.0f};

    // Calculate direction vectors
    glm_vec3_sub(camera->target, camera->position, forward);
    glm_vec3_normalize(forward);
    glm_vec3_crossn(forward, camera->up, right);

    // Handle input
    if (keyboard_down(SDLK_W)) glm_vec3_add(movement, forward, movement);  // Move forward
    if (keyboard_down(SDLK_S)) glm_vec3_sub(movement, forward, movement);  // Move backward
    if (keyboard_down(SDLK_A)) glm_vec3_sub(movement, right, movement);    // Strafe left
    if (keyboard_down(SDLK_D)) glm_vec3_add(movement, right, movement);    // Strafe right

    // Normalize movement
    if (!glm_vec3_eqv(movement, (vec3){0.0f, 0.0f, 0.0f})) {
        glm_vec3_normalize(movement);
    }

    // Move the camera
    camera_move(camera, movement);
}

typedef struct {
    mat4 u_model;
    mat4 u_projection;
    mat4 u_view;
} transform_params_t;

typedef struct {
    vec4 tint;
} colour_params_t;

char* read_file_into_char(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("failed to open file");
        return NULL;
    }
 
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
 
    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        perror("memory alloc failed");
        fclose(file);
        return NULL;
    }
 
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        perror("error reading file");
        free(buffer);
        fclose(file);
        return NULL;
    }
 
    buffer[bytes_read] = '\0';
 
    fclose(file);
    return buffer;
}

struct  {
    SDL_Window* window;
    SDL_GLContext ctx;
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
    sg_swapchain swapchain;
} state;

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        perror("sdl init failed!");
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    const int SCR_WIDTH = 800;
    const int SCR_HEIGHT = 600;

    state.window = SDL_CreateWindow("Window", 800, 600, SDL_WINDOW_OPENGL);
    state.ctx = SDL_GL_CreateContext(state.window);
    bool open = true;

    camera_t camera = camera_init((vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
    camera.speed = 5.f;
    camera.sensitivity = 0.1f;

    // Calculate the view matrix
    glm_lookat(camera.position, camera.target, camera.up, camera.view);

    // Set up a perspective projection matrix
    glm_perspective(glm_rad(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f, camera.projection);

    stm_setup();
    sg_desc desc;
    sg_setup(&desc);

    float vertices[] = {
        -0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 0.0f, 1.0f,
    };
    state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices),
        .label = "triangle-vertices"
    });

    uint16_t indices[] = {
        0, 1, 2,
        0, 2, 3,
    };
    state.bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(indices),
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .label = "triangle-indices"
    });

    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .vertex_func.source = read_file_into_char("../shaders/quad.vert"),
        .fragment_func.source = read_file_into_char("../shaders/quad.frag"),
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(transform_params_t),
            .glsl_uniforms = {
                [0] = { .type = SG_UNIFORMTYPE_MAT4, .glsl_name = "u_model" },
                [1] = { .type = SG_UNIFORMTYPE_MAT4, .glsl_name = "u_projection" },
                [2] = { .type = SG_UNIFORMTYPE_MAT4, .glsl_name = "u_view" },
            }
        }
    });

    sg_shader shd1 = sg_make_shader(&(sg_shader_desc){
        .vertex_func.source = read_file_into_char("../shaders/texture.vert"),
        .fragment_func.source = read_file_into_char("../shaders/texture.frag"),
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(transform_params_t),
            .glsl_uniforms = {
                [0] = { .type = SG_UNIFORMTYPE_MAT4, .glsl_name = "u_model" },
                [1] = { .type = SG_UNIFORMTYPE_MAT4, .glsl_name = "u_projection" },
                [2] = { .type = SG_UNIFORMTYPE_MAT4, .glsl_name = "u_view" },
            }
        },
        .uniform_blocks[1] = {
            .stage = SG_SHADERSTAGE_FRAGMENT,
            .size = sizeof(colour_params_t),
            .glsl_uniforms = {
                [0] = {.type = SG_UNIFORMTYPE_FLOAT4, .glsl_name = "u_tint"}
            }
        }
    });

    state.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .layout = {
            .attrs = {
                [0] = { .offset = 0, .format = SG_VERTEXFORMAT_FLOAT3 },
                [1] = { .offset = 12, .format = SG_VERTEXFORMAT_FLOAT4 }
            }
        },
        .label = "epic-sigma"
    });

    state.pass_action = (sg_pass_action) {
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.2f, 0.3f, 0.3f, 1.0f }
        },
    };

    state.swapchain = (sg_swapchain){
        .width = SCR_WIDTH,
        .height = SCR_HEIGHT,
        .color_format = SG_PIXELFORMAT_RGBA8,
    };
    
    transform_params_t vs_params;
    glm_mat4_identity(vs_params.u_model);
    glm_mat4_copy(camera.view, vs_params.u_view);
    glm_mat4_copy(camera.projection, vs_params.u_projection);
    float angle = 0.0f;

    colour_params_t fs_params;
    glm_vec4_copy((vec4){1.f, 1.f, 1.f, 1.f}, fs_params.tint);

    SDL_GL_SetSwapInterval(1);

    SDL_Event event;

    // SDL_HideCursor();
    // SDL_SetWindowRelativeMouseMode(state.window, true);

    float delta_time;

    while (open) {
        angle += 0.1f;
        glm_mat4_identity(vs_params.u_model);
        glm_rotate_y(vs_params.u_model, angle, vs_params.u_model);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    open = false;
                    break;

                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP:
                    keyboard_process(event);
                    if (event.key.key == SDLK_ESCAPE) {
                        open = false;
                    }
                    break;

                default:
                    break;
            }
        }

        int xrel, yrel;
        SDL_GetRelativeMouseState(&xrel, &yrel);

        camera_handle_input(&camera);

        glm_mat4_copy(camera.view, vs_params.u_view);
        glm_mat4_copy(camera.projection, vs_params.u_projection);

        glm_lookat(camera.position, camera.target, camera.up, camera.view);
        glm_mat4_copy(camera.view, vs_params.u_view);
        glm_mat4_copy(camera.projection, vs_params.u_projection);

        sg_begin_pass(&(sg_pass){ .action = state.pass_action, .swapchain = state.swapchain });
        sg_apply_pipeline(state.pip);
        sg_apply_bindings(&state.bind);
        sg_apply_uniforms(0, &SG_RANGE(vs_params));
        sg_draw(0, 6, 1);
        sg_end_pass();
        sg_commit();

        SDL_GL_SwapWindow(state.window);
    }


    sg_shutdown();
    SDL_DestroyWindow(state.window);
    SDL_GL_DestroyContext(state.ctx);
    SDL_Quit();

    return 0;
}
