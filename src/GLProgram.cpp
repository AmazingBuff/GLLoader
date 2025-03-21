//
// Created by Hash Liu on 2025/2/28.
//

#include "GLProgram.h"
#include "GLFunctions.h"

namespace GL
{
    enum class ProgramStatus
    {
        compile,
        link
    };


    static bool check_program(GLFunctions const* func, ProgramStatus status, GLuint value)
    {
        GLint state;
        switch (status)
        {
        case ProgramStatus::compile:
            func->glGetShaderiv(value, GL_COMPILE_STATUS, &state);
            break;
        case ProgramStatus::link:
            func->glGetProgramiv(value, GL_LINK_STATUS, &state);
            break;
        }

        if (state != GL_TRUE)
        {
            char info_log[512];
            switch (status)
            {
            case ProgramStatus::compile:
                func->glGetShaderInfoLog(value, sizeof(info_log), nullptr, info_log);
                break;
            case ProgramStatus::link:
                func->glGetProgramInfoLog(value, sizeof(info_log), nullptr, info_log);
                break;
            }

            return false;
        }
        return true;
    }

    GLProgram::GLProgram(GLContext* context)
    {
        m_context = context;
        m_program = m_context->get_func()->glCreateProgram();
    }

    GLProgram::~GLProgram()
    {
        auto func = m_context->get_func();
        func->glUseProgram(0);
        func->glDeleteProgram(m_program);
    }

    void GLProgram::attach_shader(ShaderType type, const char* source)
    {
        auto func = m_context->get_func();
        switch (type)
        {
            case ShaderType::Vertex:
            {
                m_vertex = func->glCreateShader(GL_VERTEX_SHADER);
                func->glShaderSource(m_vertex, 1, &source, nullptr);
                func->glCompileShader(m_vertex);
                check_program(func, ProgramStatus::compile, m_vertex);

                func->glAttachShader(m_program, m_vertex);
                break;
            }
            case ShaderType::Fragment:
            {
                m_fragment = func->glCreateShader(GL_FRAGMENT_SHADER);
                func->glShaderSource(m_fragment, 1, &source, nullptr);
                func->glCompileShader(m_fragment);
                check_program(func, ProgramStatus::compile, m_fragment);

                func->glAttachShader(m_program, m_fragment);
                break;
            }
            default:
                break;
        }
    }

    void GLProgram::link() const
    {
        auto func = m_context->get_func();
        func->glLinkProgram(m_program);
        check_program(func, ProgramStatus::link, m_program);

        func->glDeleteShader(m_vertex);
        func->glDeleteShader(m_fragment);
    }

    void GLProgram::use() const
    {
        m_context->get_func()->glUseProgram(m_program);
    }

    void GLProgram::release() const
    {
        m_context->get_func()->glUseProgram(0);
    }

    void GLProgram::set_uniform_value(const char* name, const bool& value) const
    {
        auto func = m_context->get_func();
        func->glUniform1i(func->glGetUniformLocation(m_program, name), static_cast<int>(value));
    }

    void GLProgram::set_uniform_value(const char* name, const int32_t& value) const
    {
        auto func = m_context->get_func();
        func->glUniform1i(func->glGetUniformLocation(m_program, name), value);
    }

    void GLProgram::set_uniform_value(const char* name, const float& value) const
    {
        auto func = m_context->get_func();
        func->glUniform1f(func->glGetUniformLocation(m_program, name), value);
    }

    void GLProgram::set_uniform_value(const char* name, const float& v1, const float& v2) const
    {
        auto func = m_context->get_func();
        func->glUniform2f(func->glGetUniformLocation(m_program, name), v1, v2);
    }

    void GLProgram::set_uniform_value(const char* name, const float* matrix, int rows, int cols) const
    {
        auto func = m_context->get_func();
        switch (rows)
        {
        case 2:
        {
            switch (cols)
            {
            case 2:
                func->glUniformMatrix2fv(func->glGetUniformLocation(m_program, name), 1, GL_TRUE, matrix);
                break;
            case 3:
                func->glUniformMatrix2x3fv(func->glGetUniformLocation(m_program, name), 1, GL_TRUE, matrix);
                break;
            case 4:
                func->glUniformMatrix2x4fv(func->glGetUniformLocation(m_program, name), 1, GL_TRUE, matrix);
                break;
            default:
                break;
            }
        }
        case 3:
        {
            switch (cols)
            {
            case 2:
                func->glUniformMatrix3x2fv(func->glGetUniformLocation(m_program, name), 1, GL_TRUE, matrix);
                break;
            case 3:
                func->glUniformMatrix3fv(func->glGetUniformLocation(m_program, name), 1, GL_TRUE, matrix);
                break;
            case 4:
                func->glUniformMatrix3x4fv(func->glGetUniformLocation(m_program, name), 1, GL_TRUE, matrix);
                break;
            default:
                break;
            }
        }
        case 4:
        {
            switch (cols)
            {
            case 2:
                func->glUniformMatrix4x2fv(func->glGetUniformLocation(m_program, name), 1, GL_TRUE, matrix);
                break;
            case 3:
                func->glUniformMatrix4x3fv(func->glGetUniformLocation(m_program, name), 1, GL_TRUE, matrix);
                break;
            case 4:
                func->glUniformMatrix4fv(func->glGetUniformLocation(m_program, name), 1, GL_TRUE, matrix);
                break;
            default:
                break;
            }
        }
        default:
              break;
        }
    }

    void GLProgram::attribute_location(const char* name) const
    {
        m_context->get_func()->glGetAttribLocation(m_program, name);
    }
}
