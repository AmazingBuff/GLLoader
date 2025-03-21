
#ifndef GLLoader_EXPORT_H
#define GLLoader_EXPORT_H

#ifdef GLLOADER_STATIC_DEFINE
#  define GLLoader_EXPORT
#  define GLLOADER_NO_EXPORT
#else
#  ifndef GLLoader_EXPORT
#    ifdef GLLoader_EXPORTS
        /* We are building this library */
#      define GLLoader_EXPORT 
#    else
        /* We are using this library */
#      define GLLoader_EXPORT 
#    endif
#  endif

#  ifndef GLLOADER_NO_EXPORT
#    define GLLOADER_NO_EXPORT 
#  endif
#endif

#ifndef GLLOADER_DEPRECATED
#  define GLLOADER_DEPRECATED __declspec(deprecated)
#endif

#ifndef GLLOADER_DEPRECATED_EXPORT
#  define GLLOADER_DEPRECATED_EXPORT GLLoader_EXPORT GLLOADER_DEPRECATED
#endif

#ifndef GLLOADER_DEPRECATED_NO_EXPORT
#  define GLLOADER_DEPRECATED_NO_EXPORT GLLOADER_NO_EXPORT GLLOADER_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GLLOADER_NO_DEPRECATED
#    define GLLOADER_NO_DEPRECATED
#  endif
#endif

#endif /* GLLoader_EXPORT_H */
