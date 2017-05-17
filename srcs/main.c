#define SDL_MAIN_HANDLED
#define GL_GLEXT_PROTOTYPES
#define STB_IMAGE_IMPLEMENTATION
#include "linmath.h"
#include "stb_image.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdbool.h>

#define APPLICATION_STACK_SIZE_BYTES 50000

struct {
    uint32_t top;
    uint8_t  pool[APPLICATION_STACK_SIZE_BYTES];
} application_stack_memory;

static void* memoria_stack_alloc(uint32_t size) {
    uint8_t* top_of_stack = &application_stack_memory.pool[application_stack_memory.top];
    application_stack_memory.top += size;

    // SDL_Log("Stack stats { current: %u, capacity: %u }", internal_stack->stack_ptr, internal_stack->capacity);
    SDL_assert(application_stack_memory.top <= APPLICATION_STACK_SIZE_BYTES);

    return top_of_stack;
}
static void memoria_stack_free(uint32_t size) {
    SDL_assert(size <= application_stack_memory.top);
    application_stack_memory.top -= size;
    // SDL_Log("Stack stats { current: %u, capacity: %u }", internal_stack->stack_ptr, internal_stack->capacity);
}

static float deg_to_rad(float deg) {
    float pi = 3.1415926535897f;
    return (pi * deg) / 180.0f;
}

struct TrianglesVertex {
    float position[3];
    float normal[3];
    float tex_coord[2];
};

static void saya_cube_indices(uint32_t* count, uint32_t* output) {
    uint32_t triangles_per_side   = 2;
    uint32_t indices_per_triangle = 3;
    uint32_t cube_sides           = 6;
    uint32_t total_index_count    = cube_sides * triangles_per_side * indices_per_triangle;

    if (!output) {
        *count = total_index_count;
        return;
    }

    for (uint32_t i = 0; i < total_index_count; ++i) {
        uint32_t cube_side_index_offsets[] = {0, 1, 2, 2, 3, 0};
        uint32_t start_index               = (i / 6) * 4;
        uint32_t cube_side_index_offset    = cube_side_index_offsets[i % 6];
        output[i]                          = start_index + cube_side_index_offset;
    }
}
static void saya_cube_vertices(uint32_t* count, struct TrianglesVertex* output) {
    uint32_t vertex_per_side = 4;
    uint32_t cube_sides      = 6;
    uint32_t vertex_count    = vertex_per_side * cube_sides;

    if (!output) {
        *count = vertex_count;
        return;
    }

    for (uint32_t i = 0; i < vertex_count; ++i) {
        struct {
            vec4 position;
            vec4 normal;
            vec2 tex_coord;
        } references[] = {
            {{-1.0f, -1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f, -1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        };

        uint32_t current_reference = i % 4;
        uint32_t current_side      = i / vertex_per_side;
        mat4x4   rotation          = {0};
        vec4     position          = {0};
        vec4     normal            = {0};

        mat4x4_identity(rotation);

        switch (current_side) {
        default: // sides
            mat4x4_rotate_Y(rotation, rotation, deg_to_rad(90.0f) * (i / 4));
            break;
        case 4: // top
            mat4x4_rotate_X(rotation, rotation, deg_to_rad(90.0f));
            break;
        case 5: // bottom
            mat4x4_rotate_X(rotation, rotation, deg_to_rad(-90.0f));
            break;
        }

        mat4x4_mul_vec4(position, rotation, references[current_reference].position);
        mat4x4_mul_vec4(normal, rotation, references[current_reference].normal);

        memcpy(output[i].position, position, 3 * sizeof(float));
        memcpy(output[i].normal, normal, 3 * sizeof(float));
        memcpy(output[i].tex_coord, references[current_reference].tex_coord, 2 * sizeof(float));
    }
}

#define SAFELOADGLPROC(name)                                                                                           \
    proc_##name = SDL_GL_GetProcAddress(#name);                                                                        \
    SDL_assert(NULL != proc_##name);

typedef void(APIENTRYP PFNGLCREATEBUFFERSPROC)(GLsizei n, GLuint* buffers);

PFNGLGENBUFFERSPROC                 proc_glGenBuffers              = NULL;
PFNGLBINDBUFFERPROC                 proc_glBindBuffer              = NULL;
PFNGLDELETEBUFFERSPROC              proc_glDeleteBuffers           = NULL;
PFNGLNAMEDBUFFERDATAEXTPROC         proc_glNamedBufferData         = NULL;
PFNGLGENERATEMIPMAPPROC             proc_glGenerateMipmap          = NULL;
PFNGLCOMPILESHADERPROC              proc_glCompileShader           = NULL;
PFNGLCREATESHADERPROC               proc_glCreateShader            = NULL;
PFNGLDELETESHADERPROC               proc_glDeleteShader            = NULL;
PFNGLGETSHADERIVPROC                proc_glGetShaderiv             = NULL;
PFNGLGETSHADERINFOLOGPROC           proc_glGetShaderInfoLog        = NULL;
PFNGLSHADERSOURCEPROC               proc_glShaderSource            = NULL;
PFNGLCREATEPROGRAMPROC              proc_glCreateProgram           = NULL;
PFNGLATTACHSHADERPROC               proc_glAttachShader            = NULL;
PFNGLBINDFRAGDATALOCATIONPROC       proc_glBindFragDataLocation    = NULL;
PFNGLLINKPROGRAMPROC                proc_glLinkProgram             = NULL;
PFNGLUSEPROGRAMPROC                 proc_glUseProgram              = NULL;
PFNGLDELETEPROGRAMPROC              proc_glDeleteProgram           = NULL;
PFNGLGETUNIFORMLOCATIONPROC         proc_glGetUniformLocation      = NULL;
PFNGLBINDBUFFERBASEPROC             proc_glBindBufferBase          = NULL;
PFNGLNAMEDBUFFERSUBDATAEXTPROC      proc_glNamedBufferSubData      = NULL;
PFNGLENABLEVERTEXARRAYATTRIBEXTPROC proc_glEnableVertexArrayAttrib = NULL;
PFNGLGETATTRIBLOCATIONPROC          proc_glGetAttribLocation       = NULL;
PFNGLVERTEXATTRIBPOINTERPROC        proc_glVertexAttribPointer     = NULL;
PFNGLBINDVERTEXARRAYPROC            proc_glBindVertexArray         = NULL;
PFNGLGENVERTEXARRAYSPROC            proc_glGenVertexArrays         = NULL;
PFNGLDRAWELEMENTSINSTANCEDPROC      proc_glDrawElementsInstanced   = NULL;
PFNGLACTIVETEXTUREPROC              proc_glActiveTexture           = NULL;
PFNGLCREATEBUFFERSPROC              proc_glCreateBuffers           = NULL;
PFNGLVALIDATEPROGRAMPROC            proc_glValidateProgram         = NULL;
PFNGLGETPROGRAMINFOLOGPROC          proc_glGetProgramInfoLog       = NULL;
PFNGLISPROGRAMPROC                  proc_glIsProgram               = NULL;

static void load_opengl() {
    SAFELOADGLPROC(glGenBuffers);
    SAFELOADGLPROC(glBindBuffer);
    SAFELOADGLPROC(glDeleteBuffers);
    SAFELOADGLPROC(glNamedBufferData);
    SAFELOADGLPROC(glGenerateMipmap);
    SAFELOADGLPROC(glCompileShader);
    SAFELOADGLPROC(glCreateShader);
    SAFELOADGLPROC(glDeleteShader);
    SAFELOADGLPROC(glGetShaderiv);
    SAFELOADGLPROC(glGetShaderInfoLog);
    SAFELOADGLPROC(glShaderSource);
    SAFELOADGLPROC(glCreateProgram);
    SAFELOADGLPROC(glAttachShader);
    SAFELOADGLPROC(glBindFragDataLocation);
    SAFELOADGLPROC(glLinkProgram);
    SAFELOADGLPROC(glUseProgram);
    SAFELOADGLPROC(glDeleteProgram);
    SAFELOADGLPROC(glGetUniformLocation);
    SAFELOADGLPROC(glBindBufferBase);
    SAFELOADGLPROC(glNamedBufferSubData);
    SAFELOADGLPROC(glEnableVertexArrayAttrib);
    SAFELOADGLPROC(glGetAttribLocation);
    SAFELOADGLPROC(glVertexAttribPointer);
    SAFELOADGLPROC(glBindVertexArray);
    SAFELOADGLPROC(glGenVertexArrays);
    SAFELOADGLPROC(glDrawElementsInstanced);
    SAFELOADGLPROC(glActiveTexture);
    SAFELOADGLPROC(glCreateBuffers);
    SAFELOADGLPROC(glValidateProgram);
    SAFELOADGLPROC(glGetProgramInfoLog);
    SAFELOADGLPROC(glIsProgram);
}
static void glGenBuffers(GLsizei n, GLuint* buffers) { proc_glGenBuffers(n, buffers); }
static void glBindBuffer(GLenum target, GLuint buffer) { proc_glBindBuffer(target, buffer); }
static void glDeleteBuffers(GLsizei n, const GLuint* buffers) { proc_glDeleteBuffers(n, buffers); }
static void glNamedBufferData(GLuint buffer, GLsizei size, const void* data, GLenum usage) {
    proc_glNamedBufferData(buffer, size, data, usage);
}
static void glGenerateMipmap(GLenum target) { proc_glGenerateMipmap(target); }
static void glCompileShader(GLuint shader) { proc_glCompileShader(shader); }
static GLuint glCreateShader(GLenum type) { return proc_glCreateShader(type); }
static void glDeleteShader(GLuint shader) { proc_glDeleteShader(shader); }
static void glGetShaderiv(GLuint shader, GLenum pname, GLint* params) { proc_glGetShaderiv(shader, pname, params); }
static void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) {
    proc_glGetShaderInfoLog(shader, bufSize, length, infoLog);
}
static void glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) {
    proc_glShaderSource(shader, count, string, length);
}
static GLuint glCreateProgram(void) { return proc_glCreateProgram(); }
static void glAttachShader(GLuint program, GLuint shader) { proc_glAttachShader(program, shader); }
static void glBindFragDataLocation(GLuint program, GLuint color, const GLchar* name) {
    proc_glBindFragDataLocation(program, color, name);
}
static void glLinkProgram(GLuint program) { proc_glLinkProgram(program); }
static void glUseProgram(GLuint program) { proc_glUseProgram(program); }
static void glDeleteProgram(GLuint program) { proc_glDeleteProgram(program); }
static GLint glGetUniformLocation(GLuint program, const GLchar* name) {
    return proc_glGetUniformLocation(program, name);
}
static void glBindBufferBase(GLenum target, GLuint index, GLuint buffer) {
    proc_glBindBufferBase(target, index, buffer);
}
static void glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data) {
    proc_glNamedBufferSubData(buffer, offset, size, data);
}
static void glEnableVertexArrayAttrib(GLuint vaobj, GLuint index) { proc_glEnableVertexArrayAttrib(vaobj, index); }
static GLint glGetAttribLocation(GLuint program, const GLchar* name) { return proc_glGetAttribLocation(program, name); }
static void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
                                  const void* pointer) {
    proc_glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}
static void glBindVertexArray(GLuint array) { proc_glBindVertexArray(array); }
static void glGenVertexArrays(GLsizei n, GLuint* arrays) { proc_glGenVertexArrays(n, arrays); }
static void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                    GLsizei instancecount) {
    proc_glDrawElementsInstanced(mode, count, type, indices, instancecount);
}
static void glActiveTexture(GLenum texture) { proc_glActiveTexture(texture); }
static void glCreateBuffers(GLsizei n, GLuint* buffers) { proc_glCreateBuffers(n, buffers); }
static void glValidateProgram(GLuint program) { proc_glValidateProgram(program); }
static void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) {
    proc_glGetProgramInfoLog(program, bufSize, length, infoLog);
}
static GLboolean glIsProgram(GLuint program) { return proc_glIsProgram(program); }

static GLuint load_shader(const char* file_path, GLenum shader_type) {
    SDL_RWops* handle          = SDL_RWFromFile(file_path, "rb");
    uint32_t   file_length     = (uint32_t)SDL_RWsize(handle);
    int        file_length_int = (int)file_length;
    uint8_t*   buffer          = memoria_stack_alloc(file_length);

    SDL_RWread(handle, buffer, sizeof(uint8_t), file_length);
    SDL_RWclose(handle);

    GLuint shader = glCreateShader(shader_type);

    GLint status = 0;

    glShaderSource(shader, 1, (const GLchar* const*)&buffer, &file_length_int);

    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (!status) {
        char* compilation_status = memoria_stack_alloc(1024);
        glGetShaderInfoLog(shader, 1024, NULL, compilation_status);
        SDL_Log("%s", compilation_status);
        memoria_stack_free(1024);
    }

    memoria_stack_free(file_length);

    return shader;
}
static GLuint cubes_vb_create() {
    uint32_t                count         = 0;
    uint32_t                alloc_size    = 0;
    struct TrianglesVertex* vertices      = NULL;
    GLuint                  vertex_buffer = 0;

    saya_cube_vertices(&count, NULL);
    alloc_size = count * sizeof(struct TrianglesVertex);
    vertices   = memoria_stack_alloc(alloc_size);
    saya_cube_vertices(&count, vertices);

    glCreateBuffers(1, &vertex_buffer);
    glNamedBufferData(vertex_buffer, alloc_size, vertices, GL_STATIC_DRAW);

    memoria_stack_free(alloc_size);

    return vertex_buffer;
}

struct IndexBuffer {
    GLuint   buffer;
    uint32_t count;
};

static struct IndexBuffer cubes_ib_create() {
    struct IndexBuffer index_buffer = {0, 0};
    uint32_t           alloc_size   = 0;
    uint32_t*          indices      = NULL;

    saya_cube_indices(&index_buffer.count, NULL);
    alloc_size = index_buffer.count * sizeof(uint32_t);
    indices    = memoria_stack_alloc(alloc_size);
    saya_cube_indices(&index_buffer.count, indices);

    glCreateBuffers(1, &index_buffer.buffer);
    glNamedBufferData(index_buffer.buffer, alloc_size, indices, GL_STATIC_DRAW);

    memoria_stack_free(alloc_size);

    return index_buffer;
}

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    uint32_t w = 1200;
    uint32_t h = 800;

    SDL_Window* window =
        SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL);

    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    SDL_GL_LoadLibrary(NULL);
    load_opengl();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    application_stack_memory.top = 0;

    struct IndexBuffer index_buffer  = cubes_ib_create();
    GLuint             vertex_buffer = cubes_vb_create();
    GLuint             texture_buffer;
    GLuint             ssbo_buffer;
    GLuint             vertex_shader;
    GLuint             fragment_shader;
    GLuint             program;
    GLuint             vao;

    {

    }

    {
        glCreateBuffers(1, &ssbo_buffer);
        glNamedBufferData(ssbo_buffer, sizeof(mat4x4) * 100, NULL, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_buffer);
    }

    {
        int x;
        int y;
        int n;

        unsigned char* data = stbi_load("assets/rem_chibi.png", &x, &y, &n, 0);

        glGenTextures(1, &texture_buffer);
        glBindTexture(GL_TEXTURE_2D, texture_buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }

    vertex_shader   = load_shader("assets/triangle.vert", GL_VERTEX_SHADER);
    fragment_shader = load_shader("assets/triangle.frag", GL_FRAGMENT_SHADER);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glUseProgram(program);

    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.buffer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_buffer);

        glEnableVertexArrayAttrib(vao, 0);
        glEnableVertexArrayAttrib(vao, 1);
        glEnableVertexArrayAttrib(vao, 2);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct TrianglesVertex),
                              (void*)offsetof(struct TrianglesVertex, position));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct TrianglesVertex),
                              (void*)offsetof(struct TrianglesVertex, normal));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct TrianglesVertex),
                              (void*)offsetof(struct TrianglesVertex, tex_coord));
    }

    uint64_t performence_frequency = SDL_GetPerformanceFrequency();
    double   frequency_as_double   = (double)performence_frequency;
    double   frame_time_sec        = 0.001;
    uint64_t start_of_frame_ticks  = 0;

    while (!SDL_QuitRequested()) {
        start_of_frame_ticks                 = SDL_GetPerformanceCounter();
        double current_measurement_as_double = (double)start_of_frame_ticks;
        double current_time                  = current_measurement_as_double / frequency_as_double;

        float  aspect_ratio   = (float)(w) / (float)(h);
        mat4x4 view           = {0};
        mat4x4 projection     = {0};
        mat4x4 projectionview = {0};
        vec3   eye            = {0.0f, 2.7f, -15.0f};
        vec3   center         = {0.0f, 1.0f, 0.0f};
        vec3   up             = {0.0f, 0.0f, -1.0f};

        mat4x4_look_at(view, eye, center, up);
        mat4x4_identity(projection);
        mat4x4_perspective(projection, 100.0f, aspect_ratio, 0.001f, 10000.0f);
        mat4x4_mul(projectionview, projection, view);

        uint32_t cube_mvps_alloc_size = sizeof(mat4x4) * 100;
        mat4x4*  cube_mvps            = memoria_stack_alloc(cube_mvps_alloc_size);

        for (uint32_t i = 0; i < 100; ++i) {
            uint32_t modulo             = i % 10;
            uint32_t divide             = i / 1;
            float    time_current_float = (float)current_time;
            float    separation         = 10.0f + 9.0f * sinf(time_current_float * 1.5f);
            float    x                  = -separation * 4 + (separation * modulo);
            float    z                  = 0.0f + separation / 5.0f * divide;
            bool     useSinus           = (1 == i % 2);
            float    y_rotation         = useSinus ? sinf(time_current_float) : cosf(time_current_float);
            float    z_rotation         = useSinus ? 3.0f * cosf(time_current_float) : 2.0f * cosf(time_current_float);
            mat4x4   model              = {0};

            mat4x4_identity(model);
            mat4x4_translate(model, x, 0.0, z);
            mat4x4_rotate_Y(model, model, y_rotation);
            mat4x4_rotate_Z(model, model, z_rotation);
            mat4x4_rotate_Y(model, model, time_current_float);
            mat4x4_mul(cube_mvps[i], projectionview, model);
        }

        glNamedBufferSubData(ssbo_buffer, 0, cube_mvps_alloc_size, cube_mvps);

        memoria_stack_free(cube_mvps_alloc_size);

        SDL_GL_MakeCurrent(window, glcontext);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElementsInstanced(GL_TRIANGLES, index_buffer.count, GL_UNSIGNED_INT, NULL, 100);
        SDL_GL_SwapWindow(window);

        uint32_t desired_fps             = 200;
        double   total_frame_time_sec    = 1.0 / desired_fps;
        double   total_frame_time_ms     = 1000.0 * total_frame_time_sec;
        uint64_t delta                   = SDL_GetPerformanceCounter() - start_of_frame_ticks;
        double   delta_as_double         = (double)delta;
        double   elapsed_ms              = 1000.0 * (delta_as_double / frequency_as_double);
        double   time_to_wait_ms_double  = total_frame_time_ms - elapsed_ms;
        double   time_to_wait_ms_rounded = round(time_to_wait_ms_double);
        uint32_t time_to_wait_ms         = (uint32_t)time_to_wait_ms_rounded;

        if (elapsed_ms < total_frame_time_ms)
            SDL_Delay(time_to_wait_ms);

        uint64_t frame_ticks = SDL_GetPerformanceCounter() - start_of_frame_ticks;
        frame_time_sec       = (double)frame_ticks / frequency_as_double;

        //SDL_Log("%f", frame_time_sec);
    }

    glDeleteProgram(program);
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteTextures(1, &texture_buffer);
    glDeleteBuffers(1, &ssbo_buffer);
    glDeleteBuffers(1, &index_buffer.buffer);
    glDeleteBuffers(1, &vertex_buffer);

    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_GL_UnloadLibrary();
    SDL_Quit();
}
