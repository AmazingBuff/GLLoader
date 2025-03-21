//
// Created by Hash Liu on 2025/3/12.
//

#pragma once

#include <cstdint>
#include <gl/glcorearb.h>

#include "GLContext.h"

namespace GL
{
    enum class MeshType : uint8_t
    {
        quad,
    };

    class GLLoader_EXPORT GLVao
    {
    public:
        explicit GLVao(MeshType type, GLContext* context = GLContext::current_context());
        ~GLVao();
        void draw() const;

        void bind() const;
        void unbind() const;
    private:
        GLContext*       m_context  = nullptr;
        GLuint           m_vao      = 0;
        GLuint           m_vbo      = 0;
        MeshType         m_type;
    };
}
