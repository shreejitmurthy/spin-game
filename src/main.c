/*
    Kept here as a reference for using Asura's dependencies, for now, not needed.
*/

#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define SOKOL_GFX_IMPL
#define SOKOL_GLCORE
#include <sokol_gfx.h>
#include <sokol_log.h>

#include <cglm/cglm.h>

typedef struct {
    mat4 u_model;
} params_t;

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

    const int screenWidth = 800;
    const int screenHeight = 600;

    state.window = SDL_CreateWindow("Window", 800, 600, SDL_WINDOW_OPENGL);
    state.ctx = SDL_GL_CreateContext(state.window);
    bool open = true;

    sg_desc desc;
    sg_setup(&desc);

    float vertices[] = {
        -0.5f,  0.5f, 0.0f,     1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, 0.0f,     0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,     1.0f, 1.0f, 0.0f, 1.0f,
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
        .vertex_func.source = read_file_into_char("../shaders/triangle.vert"),
        .fragment_func.source = read_file_into_char("../shaders/triangle.frag"),
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(params_t),
            .glsl_uniforms = {
                [0] = { .type = SG_UNIFORMTYPE_MAT4, .glsl_name = "u_model" }
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
        .width = screenWidth,
        .height = screenHeight,
        .color_format = SG_PIXELFORMAT_RGBA8,
    };
    
    params_t vs_params;
    glm_mat4_identity(vs_params.u_model);
    float angle = 0.0f;

    SDL_GL_SetSwapInterval(1);

    SDL_Event event;

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
                    if (event.key.key == SDLK_ESCAPE) {
                        open = false;
                    }
                    break;

                default:
                    break;
            }
        }

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
