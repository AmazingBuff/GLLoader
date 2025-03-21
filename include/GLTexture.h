//
// Created by Hash Liu on 2025/3/18.
//

#pragma once

#include <gl/glcorearb.h>

#include "GLContext.h"

#if defined(_WIN32) && defined(GL_ES)
#include <d3d11.h>
#endif

namespace GL
{
    // this class is to generate gl texture and convert gl texture by context
    class GLLoader_EXPORT GLTexture
    {
    public:
        GLTexture(int width, int height, int internal_format, int encode_format, GLContext* context = GLContext::current_context());
#if defined(_WIN32) && defined(GL_ES)
        // shared memory with dx11, only create a texture with rgba
        GLTexture(HANDLE shared_handle, int width, int height, GLContext* context = GLContext::current_context());
        //GLTexture(ID3D11Texture2D* texture, int width, int height, GLContext* context = GLContext::current_context());
#endif
        ~GLTexture();
        [[nodiscard]] GLuint id() const;
        [[nodiscard]] int width() const;
        [[nodiscard]] int height() const;
        // nv12 yuv420 rgba, encode format, AVPixelFormat
        [[nodiscard]] int format() const;
        // rgba rgb rg r, channel format
        [[nodiscard]] int internal_format() const;
    private:
        GLContext*      m_context  = nullptr;
        GLuint          m_id       = 0;
        int             m_width    = 0;
        int             m_height   = 0;
        int             m_internal = 0;
        int             m_format   = 0;
    };
}
