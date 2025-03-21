//
// Created by Hash Liu on 2025/3/18.
//

#pragma once

#include <GLContext.h>

#include "fork/wgl.h"

namespace GL
{
    class WGLContext final : public GLContext
    {
    public:
        WGLContext(HWND hwnd, HDC hdc, bool shared);
        ~WGLContext() override;

        bool initialize();
        bool activate() const override;
        bool release() const override;
        bool swap_buffers() const override;
        bool is_opengl_es() const override;
    private:
        HWND m_hwnd;
        HDC m_hdc;
        HGLRC m_hglrc;
    };


    struct WGLFunctions
    {
        PFNWGLCREATECONTEXTPROC                 wglCreateContext = nullptr;
        PFNWGLDELETECONTEXTPROC                 wglDeleteContext = nullptr;
        PFNWGLGETCURRENTCONTEXTPROC             wglGetCurrentContext = nullptr;
        PFNWGLGETCURRENTDCPROC                  wglGetCurrentDC = nullptr;
        PFNWGLGETPROCADDRESSPROC                wglGetProcAddress = nullptr;
        PFNWGLMAKECURRENTPROC                   wglMakeCurrent = nullptr;
        PFNWGLSHARELISTSPROC                    wglShareLists = nullptr;
        PFNWGLGETEXTENSIONSSTRINGARBPROC        wglGetExtensionsStringARB = nullptr;
        PFNWGLGETPIXELFORMATATTRIBIVARBPROC     wglGetPixelFormatAttribivARB = nullptr;
        PFNWGLCHOOSEPIXELFORMATARBPROC          wglChoosePixelFormatARB = nullptr;
        PFNWGLCREATECONTEXTATTRIBSARBPROC       wglCreateContextAttribsARB = nullptr;

        void initialize();
    private:
        bool initialized = false;
    };
    extern WGLFunctions s_wgl_funcs;
}