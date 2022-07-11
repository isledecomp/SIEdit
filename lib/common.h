#ifndef LIBWEAVER_GLOBAL_H
#define LIBWEAVER_GLOBAL_H

#if defined(__GNUC__)
#define LIBWEAVER_PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#elif defined(_MSC_VER)
#define LIBWEAVER_PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

#ifdef _MSC_VER
#define LIBWEAVER_EXPORT __declspec(dllexport)
#else
#define LIBWEAVER_EXPORT
#endif

#if defined(_WIN32)
#define LIBWEAVER_OS_WINDOWS
#elif defined(__APPLE__)
#define LIBWEAVER_OS_MACOS
#elif defined(__linux__)
#define LIBWEAVER_OS_LINUX
#endif

#endif // LIBWEAVER_GLOBAL_H
