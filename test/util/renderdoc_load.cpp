//
// Created by Hash Liu on 2025/3/20.
//

#ifdef _WIN32
#include <windows.h>
#endif
#include <cstdio>

#include "renderdoc_load.h"

RENDERDOC_API_1_6_0* s_renderdoc_api = nullptr;

bool load_renderdoc()
{
    if (s_renderdoc_api == nullptr)
    {
#ifdef _WIN32
        if (HMODULE module = GetModuleHandleA("renderdoc.dll"))
        {
            pRENDERDOC_GetAPI RENDERDOC_GetApi = (pRENDERDOC_GetAPI)GetProcAddress(module, "RENDERDOC_GetAPI");
            if (RENDERDOC_GetApi && RENDERDOC_GetApi(eRENDERDOC_API_Version_1_6_0, (void**)&s_renderdoc_api) == 1)
                return true;
        }
#endif
        s_renderdoc_api = nullptr;
        return false;
    }
    return true;
}