//
// Created by Hash Liu on 2025/3/18.
//

#pragma once

#include <d3d11.h>
#include <GLFunctions.h>
#include <GLExtFunctions.h>
#include <GLTexture.h>

#include "platform/EGLContext.h"

namespace GL
{
    GLTexture::GLTexture(int width, int height, int internal_format, int encode_format, GLContext* context)
        : m_context(context), m_width(width), m_height(height), m_internal(internal_format), m_format(encode_format)
    {
        m_context->get_func()->glGenTextures(1, &m_id);
    }

#if defined(_WIN32) && defined(GL_ES)
    GLTexture::GLTexture(HANDLE shared_handle, int width, int height, GLContext* context)
        : m_context(context), m_width(width), m_height(height), m_internal(GL_RGBA), m_format(GL_RGBA)
    {
        auto egl_context = dynamic_cast<EGLContext*>(m_context);
        auto func = egl_context->get_func();

        EGLSurface surface = EGL_NO_SURFACE;

        EGLint pb_attributes[] =
        {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
            EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
            EGL_NONE
        };

        surface = s_egl_funcs.eglCreatePbufferFromClientBuffer(egl_context->m_display, EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE, shared_handle, egl_context->m_config, pb_attributes);

        func->glGenTextures(1, &m_id);
        func->glBindTexture(GL_TEXTURE_2D, m_id);

        s_egl_funcs.eglBindTexImage(egl_context->m_display, surface, EGL_BACK_BUFFER);



    }

    // GLTexture::GLTexture(ID3D11Texture2D* texture, int width, int height, GLContext* context)
    //     : m_context(context), m_width(width), m_height(height), m_internal(GL_RGBA), m_format(GL_RGBA)
    // {
    //     auto egl_context = dynamic_cast<EGLContext*>(m_context);
    //     auto func = egl_context->get_func();
    //
    //     EGLint attribs[] = {
    //         EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_RGBA8,
    //         EGL_D3D11_TEXTURE_PLANE_ANGLE, 0,
    //         EGL_NONE
    //     };
    //
    //     const char* ext = s_egl_funcs.eglQueryString(egl_context->m_display, EGL_EXTENSIONS);
    //
    //     s_egl_funcs.eglCreateImageKHR(egl_context->m_display, nullptr, EGL_D3D11_TEXTURE_ANGLE, texture, attribs);
    //
    //     auto e = s_egl_funcs.eglGetError();
    //
    //     func->glGenTextures(1, &m_id);
    //     func->glBindTexture(GL_TEXTURE_2D, m_id);
    //     m_context->get_ext_func()->glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, texture);
    // }

#endif

    GLTexture::~GLTexture()
    {
        m_context->get_func()->glDeleteTextures(1, &m_id);
    }

    GLuint GLTexture::id() const
    {
        return m_id;
    }

    int GLTexture::width() const
    {
        return m_width;
    }

    int GLTexture::height() const
    {
        return m_height;
    }

    int GLTexture::format() const
    {
        return m_format;
    }

    int GLTexture::internal_format() const
    {
        return m_internal;
    }
}