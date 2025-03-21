//
// Created by Hash Liu on 2025/3/7.
//

#include <GLFunctions.h>

#include "EGLContext.h"

#include <dxgiformat.h>

#include "PlatformGLContext.h"
#include "Utils.h"

#ifndef _WIN32
#include <dlfcn.h>
#endif

namespace GL
{
#define EGL_CHK(expr) error_chk((expr), #expr);
#ifndef NDEBUG
#define EGL_CHK_AND_RET_FALSE(expr) if (!(expr)) { printf(#expr); return false; }
#else
#define EGL_CHK_AND_RET_FALSE(expr) if (!(expr)) { return false; }
#endif
    // fork form blender
    static ::EGLContext s_shared_context = nullptr;
    static int s_shared_count = 0;


    EGLFunctions s_egl_funcs;

    void EGLFunctions::initialize()
    {
        if (!initialized)
        {
#ifdef _WIN32
            HMODULE module = LoadLibraryA("libEGL.dll");
            if (module != nullptr)
            {
                eglGetProcAddress = (PFNEGLGETPROCADDRESSPROC)GetProcAddress(module, "eglGetProcAddress");
                eglMakeCurrent = (PFNEGLMAKECURRENTPROC)GetProcAddress(module, "eglMakeCurrent");
                eglDestroySurface = (PFNEGLDESTROYSURFACEPROC)GetProcAddress(module, "eglDestroySurface");
                eglDestroyContext = (PFNEGLDESTROYCONTEXTPROC)GetProcAddress(module, "eglDestroyContext");
                eglTerminate = (PFNEGLTERMINATEPROC)GetProcAddress(module, "eglTerminate");
                eglGetDisplay = (PFNEGLGETDISPLAYPROC)GetProcAddress(module, "eglGetDisplay");
                eglInitialize = (PFNEGLINITIALIZEPROC)GetProcAddress(module, "eglInitialize");
                eglChooseConfig = (PFNEGLCHOOSECONFIGPROC)GetProcAddress(module, "eglChooseConfig");
                eglCreateContext = (PFNEGLCREATECONTEXTPROC)GetProcAddress(module, "eglCreateContext");
                eglCreatePbufferSurface = (PFNEGLCREATEPBUFFERSURFACEPROC)GetProcAddress(module, "eglCreatePbufferSurface");
                eglCreatePbufferFromClientBuffer = (PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC)GetProcAddress(module, "eglCreatePbufferFromClientBuffer");
                eglSwapBuffers = (PFNEGLSWAPBUFFERSPROC)GetProcAddress(module, "eglSwapBuffers");
                eglGetCurrentDisplay = (PFNEGLGETCURRENTDISPLAYPROC)GetProcAddress(module, "eglGetCurrentDisplay");
                eglGetCurrentSurface = (PFNEGLGETCURRENTSURFACEPROC)GetProcAddress(module, "eglGetCurrentSurface");
                eglGetCurrentContext = (PFNEGLGETCURRENTCONTEXTPROC)GetProcAddress(module, "eglGetCurrentContext");
                eglBindAPI = (PFNEGLBINDAPIPROC)GetProcAddress(module, "eglBindAPI");
                eglBindTexImage = (PFNEGLBINDTEXIMAGEPROC)GetProcAddress(module, "eglBindTexImage");
                eglGetError = (PFNEGLGETERRORPROC)GetProcAddress(module, "eglGetError");
                eglGetConfigAttrib = (PFNEGLGETCONFIGATTRIBPROC)GetProcAddress(module, "eglGetConfigAttrib");
                eglQueryString = (PFNEGLQUERYSTRINGPROC)GetProcAddress(module, "eglQueryString");
                eglQueryDisplayAttribEXT = (PFNEGLQUERYDISPLAYATTRIBEXTPROC)GetProcAddress(module, "eglQueryDisplayAttribEXT");
                eglGetPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYPROC)GetProcAddress(module, "eglGetPlatformDisplay");
                eglQueryDeviceAttribEXT = (PFNEGLQUERYDEVICEATTRIBEXTPROC)GetProcAddress(module, "eglQueryDeviceAttribEXT");
                eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)GetProcAddress(module, "eglCreateImageKHR");

                initialized = true;
            }
#else
#if defined(__APPLE__)
            static const char* Names[] = {"libEGL.dylib", "libEGL.so"};
#elif defined(__linux__)
            static const char* Names[] = {"libEGL.so.1", "libEGL.so"};
#endif
            unsigned int index = 0;
            void* module = nullptr;
            for(index = 0; index < (sizeof(Names) / sizeof(Names[0])); index++)
            {
                module = dlopen(Names[index], RTLD_NOW | RTLD_GLOBAL);

                if(module != nullptr)
                    break;
            }

            if (module != nullptr)
            {
                eglGetProcAddress = (PFNEGLGETPROCADDRESSPROC)dlsym(module, "eglGetProcAddress");
                eglMakeCurrent = (PFNEGLMAKECURRENTPROC)dlsym(module, "eglMakeCurrent");
                eglDestroySurface = (PFNEGLDESTROYSURFACEPROC)dlsym(module, "eglDestroySurface");
                eglDestroyContext = (PFNEGLDESTROYCONTEXTPROC)dlsym(module, "eglDestroyContext");
                eglTerminate = (PFNEGLTERMINATEPROC)dlsym(module, "eglTerminate");
                eglGetDisplay = (PFNEGLGETDISPLAYPROC)dlsym(module, "eglGetDisplay");
                eglInitialize = (PFNEGLINITIALIZEPROC)dlsym(module, "eglInitialize");
                eglChooseConfig = (PFNEGLCHOOSECONFIGPROC)dlsym(module, "eglChooseConfig");
                eglCreateContext = (PFNEGLCREATECONTEXTPROC)dlsym(module, "eglCreateContext");
                eglCreatePbufferSurface = (PFNEGLCREATEPBUFFERSURFACEPROC)dlsym(module, "eglCreatePbufferSurface");
                eglCreatePbufferFromClientBuffer = (PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC)dlsym(module, "eglCreatePbufferFromClientBuffer");
                eglSwapBuffers = (PFNEGLSWAPBUFFERSPROC)dlsym(module, "eglSwapBuffers");
                eglGetCurrentDisplay = (PFNEGLGETCURRENTDISPLAYPROC)dlsym(module, "eglGetCurrentDisplay");
                eglGetCurrentSurface = (PFNEGLGETCURRENTSURFACEPROC)dlsym(module, "eglGetCurrentSurface");
                eglGetCurrentContext = (PFNEGLGETCURRENTCONTEXTPROC)dlsym(module, "eglGetCurrentContext");
                eglBindAPI = (PFNEGLBINDAPIPROC)dlsym(module, "eglBindAPI");
                eglBindTexImage = (PFNEGLBINDTEXIMAGEPROC)dlsym(module, "eglBindTexImage");
                eglGetError = (PFNEGLGETERRORPROC)dlsym(module, "eglGetError");
                eglGetConfigAttrib = (PFNEGLGETCONFIGATTRIBPROC)dlsym(module, "eglGetConfigAttrib");
                eglQueryString = (PFNEGLQUERYSTRINGPROC)dlsym(module, "eglQueryString");
                eglQueryDisplayAttribEXT = (PFNEGLQUERYDISPLAYATTRIBEXTPROC)dlsym(module, "eglQueryDisplayAttribEXT");
                eglGetPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYPROC)dlsym(module, "eglGetPlatformDisplay");
                eglQueryDeviceAttribEXT = (PFNEGLQUERYDEVICEATTRIBEXTPROC)dlsym(module, "eglQueryDeviceAttribEXT");
                eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)dlsym(module, "eglCreateImageKHR");

                initialized = true;
            }
#endif
        }
    }

    static bool has_extension(EGLDisplay display, const char* extension)
    {
        if (!s_egl_funcs.eglQueryString || !extension)
            return false;

        const char* extensions = s_egl_funcs.eglQueryString(display, EGL_EXTENSIONS);
        size_t len = strlen(extension);

        if (extensions == nullptr || *extensions == '\0')
            return false;

        /* Make sure that don't just find an extension with our name as a prefix. */
        while (true)
        {
            extensions = strstr(extensions, extension);
            if (!extensions)
                return false;

            if (extensions[len] == ' ' || extensions[len] == 0)
                return true;
            extensions += len;
        }
    }

    static bool initialize_egl_context(EGLDisplay display, bool shared, const ContextConfig& context_config, ::EGLContext* context, EGLSurface* surface, EGLConfig* config)
    {
        std::vector<EGLint> attribs;

        attribs.push_back(EGL_RED_SIZE);
        attribs.push_back(8);

        attribs.push_back(EGL_GREEN_SIZE);
        attribs.push_back(8);

        attribs.push_back(EGL_BLUE_SIZE);
        attribs.push_back(8);

        if (context_config.need_alpha)
        {
            attribs.push_back(EGL_ALPHA_SIZE);
            attribs.push_back(8);

            attribs.push_back(EGL_BIND_TO_TEXTURE_RGBA);
            attribs.push_back(EGL_TRUE);
        }
        else
        {
            attribs.push_back(EGL_BIND_TO_TEXTURE_RGB);
            attribs.push_back(EGL_TRUE);
        }

        attribs.push_back(EGL_SURFACE_TYPE);
        attribs.push_back(EGL_PBUFFER_BIT);

        attribs.push_back(EGL_RENDERABLE_TYPE);
        if (context_config.major_version >= 3)
            attribs.push_back(EGL_OPENGL_ES3_BIT);
        else
            attribs.push_back(EGL_OPENGL_ES2_BIT);

        attribs.push_back(EGL_NONE);

        EGLint num_config;
        EGL_CHK_AND_RET_FALSE(s_egl_funcs.eglChooseConfig(display, attribs.data(), nullptr, 0, &num_config));

        std::vector<EGLConfig> configs(num_config);
        EGL_CHK_AND_RET_FALSE(s_egl_funcs.eglChooseConfig(display, attribs.data(), configs.data(), num_config, &num_config));
        *config = configs[1];

        static const EGLint pb_attrib_list[] = {
            EGL_WIDTH, 1,
            EGL_HEIGHT, 1,
            EGL_NONE,
        };

        *surface = s_egl_funcs.eglCreatePbufferSurface(display, *config, pb_attrib_list);
        EGL_CHK_AND_RET_FALSE(*surface != nullptr);

        EGLint attrib_list[] = {
            EGL_CONTEXT_MAJOR_VERSION, context_config.major_version,
            EGL_CONTEXT_MINOR_VERSION, context_config.minor_version,
            EGL_NONE
        };

        if (shared)
        {
            *context = s_egl_funcs.eglCreateContext(display, *config, s_shared_context, attrib_list);
            EGL_CHK_AND_RET_FALSE(*context != nullptr);

            if (s_shared_context == nullptr)
                s_shared_context = *context;

            s_shared_count++;
        }
        else
        {
            *context = s_egl_funcs.eglCreateContext(display, *config, nullptr, attrib_list);
            EGL_CHK_AND_RET_FALSE(*context != nullptr);
        }

        EGL_CHK_AND_RET_FALSE(s_egl_funcs.eglMakeCurrent(display, *surface, *surface, *context));

        return true;
    }


    EGLContext::EGLContext(bool shared) : GLContext(shared) {}

    EGLContext::~EGLContext()
    {
        if (m_display != EGL_NO_DISPLAY)
        {
            if (m_context != nullptr)
            {
                if (m_context == s_egl_funcs.eglGetCurrentContext())
                    s_egl_funcs.eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, nullptr);

                if (m_is_shared && (m_context != s_shared_context || s_shared_count == 1))
                {
                    assert(s_shared_count > 0);

                    s_shared_count--;

                    if (s_shared_count == 0)
                        s_shared_context = nullptr;
                }
                s_egl_funcs.eglDestroyContext(m_display, m_context);
                m_context = nullptr;
            }

            if (m_surface != EGL_NO_SURFACE)
            {
                s_egl_funcs.eglDestroySurface(m_display, m_surface);
                m_surface = EGL_NO_SURFACE;
            }

            s_egl_funcs.eglTerminate(m_display);
            m_display = EGL_NO_DISPLAY;
        }
    }

    bool EGLContext::initialize()
    {
        EGLSurface prev_display = s_egl_funcs.eglGetCurrentDisplay();
        EGLSurface prev_draw_surface = s_egl_funcs.eglGetCurrentSurface(EGL_DRAW);
        EGLSurface prev_read_surface = s_egl_funcs.eglGetCurrentSurface(EGL_READ);
        ::EGLContext prev_context = s_egl_funcs.eglGetCurrentContext();

        m_display = s_egl_funcs.eglGetDisplay(EGL_DEFAULT_DISPLAY);
        EGL_CHK_AND_RET_FALSE(m_display != EGL_NO_DISPLAY);

        EGLint major, minor;
        EGL_CHK_AND_RET_FALSE(s_egl_funcs.eglInitialize(m_display, &major, &minor));
        EGL_CHK_AND_RET_FALSE(s_egl_funcs.eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, nullptr));
        EGL_CHK_AND_RET_FALSE(s_egl_funcs.eglBindAPI(EGL_OPENGL_ES_API));

        ContextConfig config;
        while (true)
        {
            if (initialize_egl_context(m_display, m_is_shared, config, &m_context, &m_surface, &m_config))
            {
                load_gl_es_functions(reinterpret_cast<void**>(&m_func), reinterpret_cast<void**>(&m_ext_func));
                break;
            }

            if (prev_display != EGL_NO_DISPLAY)
                EGL_CHK(s_egl_funcs.eglMakeCurrent(prev_display, prev_draw_surface, prev_read_surface, prev_context));

            config.minor_version--;
            if (config.minor_version < 0 && config.major_version == 3)
            {
                config.major_version = 2;
                config.minor_version = 0;
            }
            else if (config.minor_version < 0 && config.major_version == 2)
                break;
        }

        if (m_func)
        {
            m_func->glClearColor(0.294, 0.294, 0.294, 0.000);
            m_func->glClear(GL_COLOR_BUFFER_BIT);
            m_func->glClearColor(0.000, 0.000, 0.000, 0.000);
            EGL_CHK_AND_RET_FALSE(s_egl_funcs.eglSwapBuffers(m_display, m_surface));

            if (prev_display != EGL_NO_DISPLAY)
                EGL_CHK(s_egl_funcs.eglMakeCurrent(prev_display, prev_draw_surface, prev_read_surface, prev_context));

            return true;
        }

        return false;
    }

    GLESBackend EGLContext::backend() const
    {
#if defined(_WIN32)
        return GLESBackend::direct3d_11;
#elif defined(__APPLE__)
        return GLESBackend::metal;
#endif
    }

    bool EGLContext::activate() const
    {
        EGL_CHK_AND_RET_FALSE(s_egl_funcs.eglMakeCurrent(m_display, m_surface, m_surface, m_context));
        return true;
    }

    bool EGLContext::release() const
    {
        EGL_CHK_AND_RET_FALSE(s_egl_funcs.eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, nullptr));
        return true;
    }

    bool EGLContext::swap_buffers() const
    {
        EGL_CHK_AND_RET_FALSE(s_egl_funcs.eglSwapBuffers(m_display, m_surface));
        return true;
    }

    bool EGLContext::is_opengl_es() const
    {
        return true;
    }


    GLContext* create_egl_offscreen_context(bool shared)
    {
        s_egl_funcs.initialize();

        EGLContext* context = new EGLContext(shared);
        if (!context->initialize())
        {
            delete context;
            context = nullptr;
        }

        return context;
    }

#undef EGL_CHK
#undef EGL_CHK_AND_RET_FALSE
}