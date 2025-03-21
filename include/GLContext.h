//
// Created by Hash Liu on 2025/2/28.
//

#pragma once

#include "GLLoaderExport.h"

namespace GL
{
    struct GLFunctions;
    struct GLExtFunctions;

    class GLLoader_EXPORT GLContext
    {
    public:
        explicit GLContext(bool shared);
        virtual ~GLContext();

        virtual bool activate() const = 0;

        virtual bool release() const = 0;

        virtual bool swap_buffers() const = 0;

        [[nodiscard]] virtual bool is_opengl_es() const = 0;

        [[nodiscard]] GLFunctions const* get_func() const;

        [[nodiscard]] GLExtFunctions const* get_ext_func() const;

        [[nodiscard]] bool is_shared() const;

        static GLContext* current_context();
    protected:
        GLFunctions* m_func = nullptr;
        GLExtFunctions* m_ext_func = nullptr;
        bool m_is_shared = false;
    };

    // this context will be added to shared lists automatically while shared is true
    GLLoader_EXPORT GLContext* create_offscreen_context(bool shared = true);
    GLLoader_EXPORT void destroy_context(GLContext* context);

}