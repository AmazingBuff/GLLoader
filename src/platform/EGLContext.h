//
// Created by Hash Liu on 2025/3/18.
//

#pragma once

#include <cstdint>
#include <GLContext.h>

#include "fork/egl.h"
#include "fork/eglext.h"
#include "fork/eglext_angle.h"

namespace GL
{
    enum class GLESBackend : uint8_t
    {
        unknown,
        direct3d_9,
        direct3d_11,
        gl,
        gl_es,
        vulkan,
        metal,
    };

    class GLTexture;

    class EGLContext final : public GLContext
    {
    public:
        explicit EGLContext(bool shared);
        ~EGLContext() override;

        bool initialize();
        [[nodiscard]] GLESBackend backend() const;

        bool activate() const override;
        bool release() const override;
        bool swap_buffers() const override;
        bool is_opengl_es() const override;
    private:
        EGLDisplay      m_display = nullptr;
        EGLSurface      m_surface = nullptr;
        ::EGLContext    m_context = nullptr;
        EGLConfig       m_config = nullptr;

        friend class GLTexture;
    };


    struct EGLFunctions
    {
        PFNEGLGETPROCADDRESSPROC                    eglGetProcAddress                   = nullptr;
        PFNEGLMAKECURRENTPROC                       eglMakeCurrent                      = nullptr;
        PFNEGLDESTROYSURFACEPROC                    eglDestroySurface                   = nullptr;
        PFNEGLDESTROYCONTEXTPROC                    eglDestroyContext                   = nullptr;
        PFNEGLTERMINATEPROC                         eglTerminate                        = nullptr;
        PFNEGLGETDISPLAYPROC                        eglGetDisplay                       = nullptr;
        PFNEGLINITIALIZEPROC                        eglInitialize                       = nullptr;
        PFNEGLCHOOSECONFIGPROC                      eglChooseConfig                     = nullptr;
        PFNEGLCREATECONTEXTPROC                     eglCreateContext                    = nullptr;
        PFNEGLCREATEPBUFFERSURFACEPROC              eglCreatePbufferSurface             = nullptr;
        PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC     eglCreatePbufferFromClientBuffer    = nullptr;
        PFNEGLSWAPBUFFERSPROC                       eglSwapBuffers                      = nullptr;
        PFNEGLGETCURRENTDISPLAYPROC                 eglGetCurrentDisplay                = nullptr;
        PFNEGLGETCURRENTSURFACEPROC                 eglGetCurrentSurface                = nullptr;
        PFNEGLGETCURRENTCONTEXTPROC                 eglGetCurrentContext                = nullptr;
        PFNEGLBINDAPIPROC                           eglBindAPI                          = nullptr;
        PFNEGLBINDTEXIMAGEPROC                      eglBindTexImage                     = nullptr;
        PFNEGLGETERRORPROC                          eglGetError                         = nullptr;
        PFNEGLGETCONFIGATTRIBPROC                   eglGetConfigAttrib                  = nullptr;
        PFNEGLQUERYSTRINGPROC                       eglQueryString                      = nullptr;
        PFNEGLQUERYDISPLAYATTRIBEXTPROC             eglQueryDisplayAttribEXT            = nullptr;
        PFNEGLGETPLATFORMDISPLAYPROC                eglGetPlatformDisplay               = nullptr;
        PFNEGLQUERYDEVICEATTRIBEXTPROC              eglQueryDeviceAttribEXT             = nullptr;
        PFNEGLCREATEIMAGEKHRPROC                    eglCreateImageKHR                   = nullptr;

        void initialize();
    private:
        bool initialized = false;
    };
    extern EGLFunctions s_egl_funcs;
}
