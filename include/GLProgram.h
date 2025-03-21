//
// Created by Hash Liu on 2025/2/28.
//

#pragma once

#include <cstdint>

#include "GLContext.h"

namespace GL
{
    enum class ShaderType : uint8_t
    {
        Vertex,
        Fragment
    };

    class GLLoader_EXPORT GLProgram
    {
    public:
        explicit GLProgram(GLContext* context = GLContext::current_context());
        ~GLProgram();

        void attach_shader(ShaderType type, const char* source);
        void link() const;

        void use() const;
        void release() const;

        // must call use method before set
        void set_uniform_value(const char* name, const bool& value) const;
        // must call use method before set
        void set_uniform_value(const char* name, const int32_t& value) const;
        // must call use method before set
        void set_uniform_value(const char* name, const float& value) const;
        // must call use method before set
        void set_uniform_value(const char* name, const float& v1, const float& v2) const;
        // must call use method before set
        void set_uniform_value(const char* name, const float* matrix, int rows, int cols) const;

        void attribute_location(const char* name) const;
    private:
        GLContext*    m_context = nullptr;
        uint32_t      m_program = 0;
        uint32_t      m_vertex = 0;
        uint32_t      m_fragment = 0;
    };


}

