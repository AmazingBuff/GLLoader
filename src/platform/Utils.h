//
// Created by Hash Liu on 2025/3/5.
//

#pragma once

#include <cassert>
#include <vector>
#include <algorithm>

namespace GL
{
    inline bool error_chk(bool expr, const char* msg)
    {
#ifndef NDEBUG
        if (!expr)
            printf("%s", msg);
#endif
        return expr;
    }

    void load_gl_functions(void** func, void** ext_func);
    void load_gl_es_functions(void** func, void** ext_func);
}