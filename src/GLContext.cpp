//
// Created by Hash Liu on 2025/2/28.
//


#include <GLContext.h>
#include <GLFunctions.h>
#include <GLExtFunctions.h>

#include "platform/PlatformGLContext.h"

namespace GL
{
    static thread_local GLContext* t_context = nullptr;

    GLContext::GLContext(bool shared) : m_is_shared(shared) {}

    GLContext::~GLContext()
    {
        delete m_func;
        m_func = nullptr;

        delete m_ext_func;
        m_ext_func = nullptr;
    }

    GLFunctions const* GLContext::get_func() const
    {
        return m_func;
    }

    GLExtFunctions const* GLContext::get_ext_func() const
    {
        return m_ext_func;
    }

    bool GLContext::is_shared() const
    {
        return m_is_shared;
    }

    GLContext* GLContext::current_context()
    {
        return t_context;
    }


    GLContext* create_offscreen_context(bool shared)
    {
        GLContext* context = nullptr;
#if defined(_WIN32) && !defined(GL_ES)
        context = create_wgl_offscreen_context(shared);
#else
        context = create_egl_offscreen_context(shared);
#endif
        t_context = context;
        return context;
    }

    void destroy_context(GLContext* context)
    {
        delete context;
    }
}