#include "graphics.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define IGNORE(x) (void)(x)

#define BYTESIZE(buf) \
    sizeof((buf)) * sizeof((buf[0]))

static char const *vert_shader_source = 
    #include "shader/basic.vert"
        ;
static char const *frag_shader_source = 
    #include "shader/basic.frag"
        ;

static GLFWwindow *window;
static GLuint texid, shaderid;

static const GLfloat vertices[] = {
    -1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
     1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f,
     1.f,  1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f,
    -1.f,  1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f
};

static const GLuint indices[] = {
    0, 1, 3,
    1, 2, 3
};

static unsigned win_width, win_height;
static GLuint vao, vbo, idx_buf;


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

static inline bool linking_successful(void) {
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

    glBufferData(GL_ARRAY_BUFFER, BYTESIZE(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(vertices[0]), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(vertices[0]), (void*)(3 * sizeof(vertices[0])));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(vertices[0]), (void*)(6 * sizeof(vertices[0])));

    glGenBuffers(1, &idx_buf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, BYTESIZE(indices), indices, GL_STATIC_DRAW);

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

static void create_texture(unsigned char *texdata) {

    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, win_width, win_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texdata);

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

    glShaderSource(fragid, 1, &frag_shader_source, NULL);
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

    if(!linking_successful()) {
        glDeleteProgram(shaderid);
        return false;
    }

    return true;
}

void window_size_callback(GLFWwindow *win, int w, int h) {
    IGNORE(win);
    win_width = w;
    win_height = h;
    printf("%ux%u\n", win_width, win_height);
}

void framebuffer_size_callback(GLFWwindow *win, int width, int height) {
    IGNORE(win);
    glViewport(0, 0, width, height);
}

bool g_create_window(unsigned width, unsigned height) {
    win_width = width;
    win_height = height;
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
        return false;
    }

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return true;
}

void g_terminate(void) {
    glDeleteProgram(shaderid);
    cleanup_renderer();
    destroy_texture();
    glfwTerminate();
    glfwDestroyWindow(window);
}

bool g_window_should_close(void) {
    return glfwWindowShouldClose(window);
}

void g_clear(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void g_update(void) {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

bool g_init(unsigned char *texdata) {
    if(!setup_shaders()) {
        return false;
    }
    create_texture(texdata);
    setup_renderer();
    return true;
}

void g_update_texture(unsigned char *texdata) {
    destroy_texture();
    create_texture(texdata);
}

void g_render(void) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texid);
    glUseProgram(shaderid);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, sizeof indices, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

