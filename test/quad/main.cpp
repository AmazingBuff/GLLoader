//
// Created by Hash Liu on 2025/3/20.
//

#include <GLContext.h>
#include <GLFunctions.h>
#include <GLProgram.h>
#include <GLTexture.h>
#include <GLVao.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <vector>

#include "renderdoc_load.h"

static const char* VertexShader =
#if defined(_WIN32) && !defined(GL_ES)
R"(#version 330

out vec2 v_texcoord;

void main()
{
    v_texcoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(v_texcoord * 2.0 - 1.0, 0.0, 1.0);
}
)";
#else
R"(#version 310 es

out vec2 v_texcoord;

void main()
{
    v_texcoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(v_texcoord * 2.0 - 1.0, 0.0, 1.0);
}
)";
#endif

static const char* FragmentShader =
#if defined(_WIN32) && !defined(GL_ES)
R"(#version 330
in vec2 v_texcoord;

layout (location = 0) out vec4 frag_color;

uniform sampler2D s_texture;

void main()
{
    frag_color = texture(s_texture, v_texcoord);
}
)";
#else
R"(#version 310 es

precision mediump float;

in vec2 v_texcoord;

layout (location = 0) out vec4 frag_color;

uniform sampler2D s_texture;

void main()
{
    frag_color = texture(s_texture, v_texcoord);
}
)";
#endif

static constexpr int s_width = 3840;
static constexpr int s_height = 2160;

int main()
{
    load_renderdoc();

    GL::GLContext* context = GL::create_offscreen_context(false);
    context->activate();

    auto func = context->get_func();

    int width, height, channels;
    uint8_t* image = stbi_load(ASSETS_DIR"a.png", &width, &height, &channels, STBI_rgb_alpha);

    GL::GLProgram* program = new GL::GLProgram();
    program->attach_shader(GL::ShaderType::Vertex, VertexShader);
    program->attach_shader(GL::ShaderType::Fragment, FragmentShader);
    program->link();

    GL::GLTexture* texture = new GL::GLTexture(s_width, s_height, GL_RGBA, GL_RGBA);
    func->glBindTexture(GL_TEXTURE_2D, texture->id());
    func->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s_width, s_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    func->glBindTexture(GL_TEXTURE_2D, 0);

    GLuint fbo;
    func->glGenFramebuffers(1, &fbo);
    func->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    func->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->id(), 0);
    func->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GL::GLTexture* tex = new GL::GLTexture(width, height, GL_RGBA, GL_RGBA);
    func->glBindTexture(GL_TEXTURE_2D, tex->id());
    func->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    //while (true)
    //{
        if (s_renderdoc_api)
            s_renderdoc_api->StartFrameCapture(nullptr, nullptr);

        func->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        func->glClearColor(0.0, 0.0, 0.0, 1.0);
        func->glClear(GL_COLOR_BUFFER_BIT);
        func->glDisable(GL_DEPTH_TEST);

        program->use();

        func->glActiveTexture(GL_TEXTURE0);
        func->glBindTexture(GL_TEXTURE_2D, tex->id());
        func->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        func->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        func->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        func->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        func->glViewport(0, 0, texture->width(), texture->height());
        func->glDrawArrays(GL_TRIANGLES, 0, 3);

        program->release();

        if (s_renderdoc_api)
            s_renderdoc_api->EndFrameCapture(nullptr, nullptr);
    //}

    func->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    std::vector<uint8_t> v(texture->height() * texture->width() * 4);
    func->glReadPixels(0, 0, texture->width(), texture->height(), GL_RGBA, GL_UNSIGNED_BYTE, v.data());

    stbi_write_png("f.png", texture->width(), texture->height(), 4, v.data(), 0);

    delete texture;
    delete program;
    delete tex;

    stbi_image_free(image);

    context->release();
    GL::destroy_context(context);

    return 0;
}