#include "cmpxchg.h"
#include "delta_time.h"
#include "graphics.h"
#include "julia.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <sys/time.h>

#define IGNORE(x) (void)(x)

#define ARRAYLEN(x) \
    (sizeof((x)) / sizeof((x[0])))

static char const *vert_shader_source =
    #include "shader/basic.vert"
        ;
static char const *fragl_shader_source =
    #include "shader/basic.frag"
        ;

static GLFWwindow *window;
static GLuint texid, shaderid;

static const GLfloat vertices[] = {
    -1.f, -1.f, 0.f, 0.f, 0.f,
     1.f, -1.f, 0.f, 1.f, 0.f,
     1.f,  1.f, 0.f, 1.f, 1.f,
    -1.f,  1.f, 0.f, 0.f, 1.f
};

static const GLuint indices[] = {
    0, 1, 3,
    1, 2, 3
};

static uint32_t volatile win_width, win_height;
static uint32_t volatile fb_width, fb_height;
static GLuint vao, vbo, idx_buf;

static struct timeval delta_start, delta_stop;
static float delta_time;

static inline bool compilation_successful(GLuint id) {
    GLint result;
    int info_len;

    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_len);
    if(info_len > 0) {
        char *buf = malloc(info_len + 1);
        glGetShaderInfoLog(id, info_len, NULL, buf);
        fprintf(stderr, "%s", buf);
        free(buf);
        return false;
    }
    return true;
}

static inline bool linkingl_successful(void) {
    GLint result;
    int info_len;

    glGetProgramiv(shaderid, GL_LINK_STATUS, &result);
    glGetProgramiv(shaderid, GL_INFO_LOG_LENGTH, &info_len);
    if(info_len > 0) {
        char *buf = malloc(info_len + 1);
        glGetProgramInfoLog(shaderid, info_len, NULL, buf);
        fprintf(stderr, "%s", buf);
        free(buf);
        return false;
    }

    return true;
}

static void setup_renderer(void) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(vertices[0]), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(vertices[0]), (void*)(3 * sizeof(vertices[0])));

    glGenBuffers(1, &idx_buf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void cleanup_renderer(void) {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &idx_buf);
}

static void create_texture(void) {

    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void destroy_texture(void) {
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &texid);
}

static bool setup_shaders(void) {
    GLuint vertid = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragid = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertid, 1, &vert_shader_source, NULL);
    glCompileShader(vertid);

    if(!compilation_successful(vertid)) {
        glDeleteShader(vertid);
        glDeleteShader(fragid);
        return false;
    }

    glShaderSource(fragid, 1, &fragl_shader_source, NULL);
    glCompileShader(fragid);

    if(!compilation_successful(fragid)) {
        glDeleteShader(vertid);
        glDeleteShader(fragid);
        return false;
    }
    shaderid = glCreateProgram();
    glAttachShader(shaderid, vertid);
    glAttachShader(shaderid, fragid);
    glLinkProgram(shaderid);

    glDetachShader(shaderid, vertid);
    glDetachShader(shaderid, fragid);

    glDeleteShader(vertid);
    glDeleteShader(fragid);

    if(!linkingl_successful()) {
        glDeleteProgram(shaderid);
        return false;
    }

    return true;
}

static void window_size_callback(GLFWwindow *win, int w, int h) {
    IGNORE(win);
    atomic_writeu32(&win_width, w);
    atomic_writeu32(&win_height, h);
}

static void framebuffer_size_callback(GLFWwindow *win, int width, int height) {
    IGNORE(win);
    glViewport(0, 0, width, height);
    (void)julia_update_dims(width, height);
    atomic_writeu32(&fb_width, width);
    atomic_writeu32(&fb_height, height);
}

static bool create_window(uint32_t width, uint32_t height) {
    atomic_writeu32(&win_width, width);
    atomic_writeu32(&win_height, height);
    glViewport(0, 0, width, height);

    if(!glfwInit()) {
        fputs("Failed to initialize glfw\n", stderr);
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Julia", NULL, NULL);

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;

    if(glewInit() != GLEW_OK) {
        fputs("Failed to initialize glew\n", stderr);
        glfwTerminate();
        glfwDestroyWindow(window);
        return false;
    }

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwGetFramebufferSize(window, (int*)&fb_width, (int*)&fb_height);

    return true;
}

void gl_terminate(void) {
    glDeleteProgram(shaderid);
    cleanup_renderer();
    destroy_texture();
    glfwTerminate();
    glfwDestroyWindow(window);
}

bool gl_window_should_close(void) {
    return glfwWindowShouldClose(window);
}

void gl_clear(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl_update(void) {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

bool gl_init(uint32_t width, uint32_t height) {
    if(!create_window(width, height) || !setup_shaders()) {
        return false;
    }
    create_texture();
    setup_renderer();
    return true;
}

void gl_update_texture(unsigned char *texdata) {
    static uint32_t width = 0, height = 0;

    glBindTexture(GL_TEXTURE_2D, texid);
    if(win_width != width || win_height != height) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb_width, fb_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texdata);

        width = win_width;
        height = win_height;
    }
    else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fb_width, fb_height, GL_RGB, GL_UNSIGNED_BYTE, texdata);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void gl_render(void) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texid);
    glUseProgram(shaderid);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, ARRAYLEN(indices), GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void gl_framebuffer_size(uint32_t *width, uint32_t *height) {
    *width = fb_width;
    *height = fb_height;
}

void gl_delta_tick(void) {
    delta_tick(&delta_start, &delta_stop, &delta_time);
}

float gl_delta_time(void) {
    return 1.f / (delta_time * 1000.f);
}

unsigned gl_fps(void) {
    return 1.f / gl_delta_time();
}
