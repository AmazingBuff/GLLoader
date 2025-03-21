//
// Created by Hash Liu on 2025/3/7.
//

#include <GLFunctions.h>

#include "WGLContext.h"
#include "PlatformGLContext.h"
#include "Utils.h"


namespace GL
{
#define WIN32_CHK(expr) error_chk((expr), #expr);
#ifndef NDEBUG
#define WIN32_CHK_AND_RET(expr) if (!(expr)) { std::printf(#expr); return; }
#else
#define WIN32_CHK_AND_RET(expr) if (!(expr)) { return; }
#endif
    // fork from blender
    static HGLRC s_shared_hglrc = nullptr;
    static int s_shared_count = 0;

    WGLFunctions s_wgl_funcs;

    void WGLFunctions::initialize()
    {
        if (!initialized)
        {
            HMODULE module = LoadLibraryA("opengl32.dll");
            if (module != nullptr)
            {
                wglGetProcAddress = (PFNWGLGETPROCADDRESSPROC)GetProcAddress(module, "wglGetProcAddress");
                wglCreateContext = (PFNWGLCREATECONTEXTPROC)GetProcAddress(module, "wglCreateContext");
                wglDeleteContext = (PFNWGLDELETECONTEXTPROC)GetProcAddress(module, "wglDeleteContext");
                wglGetCurrentContext = (PFNWGLGETCURRENTCONTEXTPROC)GetProcAddress(module, "wglGetCurrentContext");
                wglGetCurrentDC = (PFNWGLGETCURRENTDCPROC)GetProcAddress(module, "wglGetCurrentDC");
                wglMakeCurrent = (PFNWGLMAKECURRENTPROC)GetProcAddress(module, "wglMakeCurrent");
                wglShareLists = (PFNWGLSHARELISTSPROC)GetProcAddress(module, "wglShareLists");

                wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
                wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
                wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
                wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

                initialized = true;
            }
        }
    }

    /* Ron Fosner's code for weighting pixel formats and forcing software.
     * See http://www.opengl.org/resources/faq/technical/weight.cpp
     */
    static int weight_pixel_format(PIXELFORMATDESCRIPTOR& pfd, PIXELFORMATDESCRIPTOR& preferredPFD)
    {
        int weight = 0;

        /* assume desktop color depth is 32 bits per pixel */

        /* cull unusable pixel formats */
        /* if no formats can be found, can we determine why it was rejected? */
        if (!(pfd.dwFlags & PFD_SUPPORT_OPENGL) || !(pfd.dwFlags & PFD_DRAW_TO_WINDOW) ||
            !(pfd.dwFlags & PFD_DOUBLEBUFFER) || /* Blender _needs_ this. */
            !(pfd.iPixelType == PFD_TYPE_RGBA) ||
            (pfd.cColorBits > 32))               /* 64 bit formats disable AERO. */
        {
            return 0;
        }

        weight = 1; /* it's usable */

        weight += pfd.cColorBits - 8;

        if (preferredPFD.cAlphaBits > 0 && pfd.cAlphaBits > 0) {
            weight++;
        }

        return weight;
    }

    /*
     * A modification of Ron Fosner's replacement for ChoosePixelFormat
     * returns 0 on error, else returns the pixel format number to be used
     */
    static int choose_pixel_format_legacy(HDC hdc, PIXELFORMATDESCRIPTOR& preferredPFD)
    {
        int iPixelFormat = 0;
        int weight = 0;

        int iStereoPixelFormat = 0;
        int stereoWeight = 0;

        /* choose a pixel format using the useless Windows function in case we come up empty-handed */
        int iLastResortPixelFormat = ChoosePixelFormat(hdc, &preferredPFD);

        WIN32_CHK(iLastResortPixelFormat != 0);

        int lastPFD = DescribePixelFormat(hdc, 1, sizeof(PIXELFORMATDESCRIPTOR), nullptr);

        WIN32_CHK(lastPFD != 0);

        for (int i = 1; i <= lastPFD; i++) {
            PIXELFORMATDESCRIPTOR pfd;
            int check = DescribePixelFormat(hdc, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

            WIN32_CHK(check == lastPFD);

            int w = weight_pixel_format(pfd, preferredPFD);

            if (w > weight) {
                weight = w;
                iPixelFormat = i;
            }

            if (w > stereoWeight && (preferredPFD.dwFlags & pfd.dwFlags & PFD_STEREO)) {
                stereoWeight = w;
                iStereoPixelFormat = i;
            }
        }

        /* choose any available stereo format over a non-stereo format */
        if (iStereoPixelFormat != 0) {
            iPixelFormat = iStereoPixelFormat;
        }

        if (iPixelFormat == 0) {
            printf("Warning! Using result of ChoosePixelFormat.");
            iPixelFormat = iLastResortPixelFormat;
        }

        return iPixelFormat;
    }

    /**
    * Clone a window for the purpose of creating a temporary context to initialize WGL extensions.
    * There is no generic way to clone the lpParam parameter,
    * so the caller is responsible for cloning it themselves.
    */
    static HWND clone_window(HWND hWnd, LPVOID lpParam)
    {
        int count = 0;

        SetLastError(NO_ERROR);

        DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
        WIN32_CHK(GetLastError() == NO_ERROR);

        WCHAR lpClassName[100] = L"";
        count = GetClassNameW(hWnd, lpClassName, sizeof(lpClassName));
        WIN32_CHK(count != 0);

        WCHAR lpWindowName[100] = L"";
        count = GetWindowTextW(hWnd, lpWindowName, sizeof(lpWindowName));
        WIN32_CHK(count != 0);

        DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
        WIN32_CHK(GetLastError() == NO_ERROR);

        RECT rect;
        GetWindowRect(hWnd, &rect);
        WIN32_CHK(GetLastError() == NO_ERROR);

        HWND hWndParent = reinterpret_cast<HWND>(GetWindowLongPtr(hWnd, GWLP_HWNDPARENT));
        WIN32_CHK(GetLastError() == NO_ERROR);

        HMENU hMenu = GetMenu(hWnd);
        WIN32_CHK(GetLastError() == NO_ERROR);

        HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
        WIN32_CHK(GetLastError() == NO_ERROR);

        HWND hwndCloned = CreateWindowExW(dwExStyle,
            lpClassName,
            lpWindowName,
            dwStyle,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            hWndParent,
            hMenu,
            hInstance,
            lpParam);

        WIN32_CHK(hwndCloned != nullptr);

        return hwndCloned;
    }

    static bool has_extension(HDC hdc, const char* ext)
    {
        if (!s_wgl_funcs.wglGetExtensionsStringARB || !ext)
            return false;

        const char* extensions = s_wgl_funcs.wglGetExtensionsStringARB(hdc);
        size_t len = strlen(ext);

        if (extensions == nullptr || *extensions == '\0')
            return false;

        /* Make sure that don't just find an extension with our name as a prefix. */
        while (true)
        {
            extensions = strstr(extensions, ext);
            if (!extensions)
                return false;

            if (extensions[len] == ' ' || extensions[len] == 0)
                return true;
            extensions += len;
        }
    }


    /* Temporary context used to create the actual context. We need ARB pixel format
    * and context extensions, which are only available within a context. */
    struct DummyContextWGL
    {
        HWND dummyHWND = nullptr;

        HDC dummyHDC = nullptr;
        HGLRC dummyHGLRC = nullptr;

        HDC prevHDC = nullptr;
        HGLRC prevHGLRC = nullptr;

        int dummyPixelFormat = 0;

        PIXELFORMATDESCRIPTOR preferredPFD{};

        bool has_WGL_ARB_create_context_profile = false;

        DummyContextWGL(HDC hdc, HWND hWnd)
        {
            preferredPFD.nSize = sizeof(PIXELFORMATDESCRIPTOR);
            preferredPFD.nVersion = 1;
            preferredPFD.iPixelType = PFD_TYPE_RGBA;
            preferredPFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            preferredPFD.cColorBits = 32;
            preferredPFD.cAlphaBits = 8;
            preferredPFD.iLayerType = PFD_MAIN_PLANE;

            SetLastError(NO_ERROR);

            prevHDC = s_wgl_funcs.wglGetCurrentDC();
            WIN32_CHK(GetLastError() == NO_ERROR);

            prevHGLRC = s_wgl_funcs.wglGetCurrentContext();
            WIN32_CHK(GetLastError() == NO_ERROR);

            if (hWnd)
            {
                dummyHWND = clone_window(hWnd, nullptr);
                if (dummyHWND == nullptr)
                    return;

                dummyHDC = GetDC(dummyHWND);
                WIN32_CHK_AND_RET(dummyHDC != nullptr);
            }

            dummyPixelFormat = choose_pixel_format_legacy(dummyHDC, preferredPFD);

            if (dummyPixelFormat == 0)
                return;


            PIXELFORMATDESCRIPTOR chosenPFD;
            WIN32_CHK_AND_RET(DescribePixelFormat(dummyHDC, dummyPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &chosenPFD));
            WIN32_CHK_AND_RET(SetPixelFormat(dummyHDC, dummyPixelFormat, &chosenPFD));


            dummyHGLRC = s_wgl_funcs.wglCreateContext(dummyHDC);

            WIN32_CHK_AND_RET(dummyHGLRC != nullptr);

            WIN32_CHK(s_wgl_funcs.wglMakeCurrent(dummyHDC, dummyHGLRC));

            has_WGL_ARB_create_context_profile = has_extension(hdc, "WGL_ARB_create_context_profile");
        }

        ~DummyContextWGL()
        {
            WIN32_CHK(s_wgl_funcs.wglMakeCurrent(prevHDC, prevHGLRC));

            if (dummyHGLRC != nullptr)
                WIN32_CHK(s_wgl_funcs.wglDeleteContext(dummyHGLRC));

            if (dummyHWND != nullptr)
            {
                if (dummyHDC != nullptr)
                    WIN32_CHK(ReleaseDC(dummyHWND, dummyHDC));

                WIN32_CHK(DestroyWindow(dummyHWND));
            }
        }
    };

    static void make_attrib_list(std::vector<int>& out, bool needAlpha)
    {
        out.clear();
        out.reserve(30);

        out.push_back(WGL_SUPPORT_OPENGL_ARB);
        out.push_back(GL_TRUE);

        out.push_back(WGL_DRAW_TO_WINDOW_ARB);
        out.push_back(GL_TRUE);

        out.push_back(WGL_DOUBLE_BUFFER_ARB);
        out.push_back(GL_TRUE);

        out.push_back(WGL_ACCELERATION_ARB);
        out.push_back(WGL_FULL_ACCELERATION_ARB);

        out.push_back(WGL_PIXEL_TYPE_ARB);
        out.push_back(WGL_TYPE_RGBA_ARB);

        out.push_back(WGL_COLOR_BITS_ARB);
        out.push_back(24);

        if (needAlpha)
        {
            out.push_back(WGL_ALPHA_BITS_ARB);
            out.push_back(8);
        }

        out.push_back(0);
    }

    int choose_pixel_format_arb(HDC hdc, bool needAlpha)
    {
        std::vector<int> iAttributes;

        int iPixelFormat = 0;
        int iPixelFormats[32];

        make_attrib_list(iAttributes, needAlpha);

        UINT nNumFormats;
        WIN32_CHK(s_wgl_funcs.wglChoosePixelFormatARB(hdc, iAttributes.data(), nullptr, 32, iPixelFormats, &nNumFormats));

        if (nNumFormats > 0)
            iPixelFormat = iPixelFormats[0];

        // check pixel format
        if (iPixelFormat != 0)
        {
            if (needAlpha)
            {
                int alphaBits, iQuery = WGL_ALPHA_BITS_ARB;
                s_wgl_funcs.wglGetPixelFormatAttribivARB(hdc, iPixelFormat, 0, 1, &iQuery, &alphaBits);
                if (alphaBits == 0)
                    printf("Warning! Unable to find a frame buffer with alpha channel.");
            }
        }
        return iPixelFormat;
    }

    static HGLRC initialize_wgl_context(HWND hWnd, HDC hdc, bool shared, const ContextConfig& config)
    {
        bool result = false;
        HGLRC hglrc = nullptr;
        {
            DummyContextWGL dummy(hdc, hWnd);

            if (!s_wgl_funcs.wglCreateContextAttribsARB || GetPixelFormat(hdc) == 0)
            {
                int iPixelFormat = 0;

                if (s_wgl_funcs.wglChoosePixelFormatARB)
                    iPixelFormat = choose_pixel_format_arb(hdc, config.need_alpha);

                if (iPixelFormat == 0)
                    iPixelFormat = choose_pixel_format_legacy(hdc, dummy.preferredPFD);

                if (iPixelFormat == 0)
                    return hglrc;

                PIXELFORMATDESCRIPTOR chosenPFD;
                int lastPFD = DescribePixelFormat(hdc, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &chosenPFD);

                result = !WIN32_CHK(lastPFD != 0);
                if (result)
                    return hglrc;


                if (chosenPFD.cAlphaBits == 0)
                    printf("Unable to find a pixel format with an alpha channel.");

                result = !WIN32_CHK(SetPixelFormat(hdc, iPixelFormat, &chosenPFD));
                if (result)
                    return hglrc;
            }

            if (s_wgl_funcs.wglCreateContextAttribsARB)
            {
                std::vector<int> iAttributes;

                if (config.major_version != 0)
                {
                    iAttributes.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);
                    iAttributes.push_back(config.major_version);
                }

                if (config.minor_version != 0)
                {
                    iAttributes.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);
                    iAttributes.push_back(config.minor_version);
                }

                if (!dummy.has_WGL_ARB_create_context_profile && config.core_profile)
                    printf("Warning! OpenGL core profile not available.");

                if (!dummy.has_WGL_ARB_create_context_profile && config.compatibility_profile)
                    printf("Warning! OpenGL compatibility profile not available.");


                int profileMask = 0;

                if (dummy.has_WGL_ARB_create_context_profile && config.core_profile)
                    profileMask |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB;

                if (dummy.has_WGL_ARB_create_context_profile && config.compatibility_profile)
                    profileMask |= WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;

                if (profileMask)
                {
                    iAttributes.push_back(WGL_CONTEXT_PROFILE_MASK_ARB);
                    iAttributes.push_back(profileMask);
                }

                if (config.context_debug_flags)
                {
                    iAttributes.push_back(WGL_CONTEXT_FLAGS_ARB);
                    iAttributes.push_back(WGL_CONTEXT_DEBUG_BIT_ARB);
                }

                iAttributes.push_back(0);

                hglrc = s_wgl_funcs.wglCreateContextAttribsARB(hdc, nullptr, iAttributes.data());
            }
            else
                hglrc = s_wgl_funcs.wglCreateContext(hdc);
        }

        result = !WIN32_CHK(hglrc != nullptr);
        if (result)
            return hglrc;

        if (shared)
        {
            s_shared_count++;

            if (s_shared_hglrc == nullptr)
                s_shared_hglrc = hglrc;
            else
            {
                result = !WIN32_CHK(s_wgl_funcs.wglShareLists(s_shared_hglrc, hglrc));
                if (result)
                {
                    s_shared_count--;

                    if (hglrc == s_wgl_funcs.wglGetCurrentContext())
                        WIN32_CHK(s_wgl_funcs.wglMakeCurrent(nullptr, nullptr));

                    WIN32_CHK(s_wgl_funcs.wglDeleteContext(hglrc));
                    return nullptr;
                }
            }
        }

        WIN32_CHK(s_wgl_funcs.wglMakeCurrent(hdc, hglrc));

        return hglrc;
    }


    WGLContext::WGLContext(HWND hwnd, HDC hdc, bool shared) : GLContext(shared), m_hwnd(hwnd), m_hdc(hdc), m_hglrc(nullptr)
    {
        assert(m_hdc != nullptr);
    }

    WGLContext::~WGLContext()
    {
        if (m_hglrc != nullptr)
        {
            if (m_hglrc == s_wgl_funcs.wglGetCurrentContext())
                WIN32_CHK(s_wgl_funcs.wglMakeCurrent(nullptr, nullptr));

            if (m_is_shared && (m_hglrc != s_shared_hglrc || s_shared_count == 1))
            {
                assert(s_shared_count > 0);

                s_shared_count--;

                if (s_shared_count == 0)
                    s_shared_hglrc = nullptr;
            }
            WIN32_CHK(s_wgl_funcs.wglDeleteContext(m_hglrc));
            m_hglrc = nullptr;
        }
    }

    bool WGLContext::initialize()
    {
        SetLastError(NO_ERROR);

        HGLRC prevHGLRC = s_wgl_funcs.wglGetCurrentContext();
        WIN32_CHK(GetLastError() == NO_ERROR);

        HDC prevHDC = s_wgl_funcs.wglGetCurrentDC();
        WIN32_CHK(GetLastError() == NO_ERROR);

        ContextConfig config;
        while (true)
        {
            m_hglrc = initialize_wgl_context(m_hwnd, m_hdc, m_is_shared, config);
            if (m_hglrc)
            {
                load_gl_functions(reinterpret_cast<void**>(&m_func), reinterpret_cast<void**>(&m_ext_func));
                break;
            }
            else
                WIN32_CHK(s_wgl_funcs.wglMakeCurrent(prevHDC, prevHGLRC));

            config.minor_version--;
            if (config.minor_version < 0 && config.major_version == 4)
            {
                config.major_version = 3;
                config.minor_version = 3;
            }
            else if (config.minor_version < 0 && config.major_version == 3)
                break;
        }

        if (m_func)
        {
            m_func->glClearColor(0.294, 0.294, 0.294, 0.000);
            m_func->glClear(GL_COLOR_BUFFER_BIT);
            m_func->glClearColor(0.000, 0.000, 0.000, 0.000);
            WIN32_CHK(SwapBuffers(m_hdc));

            WIN32_CHK(s_wgl_funcs.wglMakeCurrent(prevHDC, prevHGLRC));

            return true;
        }

        return false;
    }

    bool WGLContext::activate() const
    {
        bool result = WIN32_CHK(s_wgl_funcs.wglMakeCurrent(m_hdc, m_hglrc));
        return result;
    }

    bool WGLContext::release() const
    {
        bool result = WIN32_CHK(s_wgl_funcs.wglMakeCurrent(nullptr, nullptr));
        return result;
    }

    bool WGLContext::swap_buffers() const
    {
        bool result = WIN32_CHK(SwapBuffers(m_hdc))
        return result;
    }

    bool WGLContext::is_opengl_es() const
    {
        return false;
    }


    GLContext* create_wgl_offscreen_context(bool shared)
    {
        /* OpenGL needs a dummy window to create a context on windows. */
        HWND wnd = CreateWindowA("Static",
            "Dummy OpenGL Window",
            WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0,
            0,
            64,
            64,
            nullptr,
            nullptr,
            GetModuleHandle(nullptr),
            nullptr);

        HDC hdc = GetDC(wnd);

        s_wgl_funcs.initialize();

        WGLContext* context = new WGLContext(wnd, hdc, shared);
        if (!context->initialize())
        {
            delete context;
            context = nullptr;
        }

        return context;
    }

#undef WIN32_CHK
#undef WIN32_CHK_AND_RET
}