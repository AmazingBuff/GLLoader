//
// Created by Hash Liu on 2025/3/12.
//

#include <GLContext.h>
#include <GLFunctions.h>
#include <GLVao.h>

namespace GL
{

    static constexpr GLfloat quad[] = {
        // positions		// coords
        -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
    };

    GLVao::GLVao(MeshType type, GLContext* context) : m_context(context), m_type(type)
    {
        auto func = m_context->get_func();

        func->glGenVertexArrays(1, &m_vao);
        func->glBindVertexArray(m_vao);

        func->glGenBuffers(1, &m_vbo);
        func->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        switch (m_type)
        {
        case MeshType::quad:
        {
            func->glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

            func->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            func->glEnableVertexAttribArray(0);

            func->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            func->glEnableVertexAttribArray(1);
        }
        default:
            break;
        }

        func->glBindBuffer(GL_ARRAY_BUFFER, 0);
        func->glBindVertexArray(0);
    }

    GLVao::~GLVao()
    {
        auto func = m_context->get_func();

        func->glDeleteBuffers(1, &m_vbo);
        func->glDeleteVertexArrays(1, &m_vao);
    }

    void GLVao::bind() const
    {
        m_context->get_func()->glBindVertexArray(m_vao);
    }

    void GLVao::unbind() const
    {
        m_context->get_func()->glBindVertexArray(0);
    }

    void GLVao::draw() const
    {
        auto func = m_context->get_func();
        switch (m_type)
        {
        case MeshType::quad:
            func->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            break;
        default:
            break;
        }
    }


}