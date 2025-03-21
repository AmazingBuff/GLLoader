//
// Created by Hash Liu on 2025/3/7.
//
#pragma once

namespace GL
{
    class GLContext;

    struct ContextConfig
    {
        // gl specified
        bool core_profile = true;
        bool compatibility_profile = false;
        bool context_debug_flags = false;   // debug or not debug

        // all
        bool need_alpha = true;
#if defined(_WIN32) && !defined(GL_ES)
        int major_version = 4;
        int minor_version = 6;
#else
        int major_version = 3;
        int minor_version = 2;
#endif
    };

#ifdef _WIN32
    // only valid in hardware rendering
    GLContext* create_wgl_offscreen_context(bool shared);
#endif
    // valid in hardware and software rendering
    GLContext* create_egl_offscreen_context(bool shared);
}
