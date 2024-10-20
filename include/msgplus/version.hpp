#ifndef MSGPLUS_VERSION_HPP
#define MSGPLUS_VERSION_HPP

#define MSGPLUS_VERSION_MAJOR 0
#define MSGPLUS_VERSION_MINOR 1
#define MSGPLUS_VERSION_PATCH 0

#ifndef __cplusplus
#    error "__cplusplus is not defined"
#endif

#ifndef MSGPLUS_CXX20_VALUE
#define MSGPLUS_CXX20_VALUE 202002L
#endif//MSGPLUS_CXX20_VALUE

#define MSGPLUS_CXX_STANDARD_VERSION __cplusplus

#if MSGPLUS_CXX_STANDARD_VERSION < MSGPLUS_CXX20_VALUE
#    error "msgplus requires C++20 or later."
#endif

#if ! defined(__has_include)
#  define __has_include(x) 0
#endif

#if ! defined(__has_cpp_attribute)
#  define __has_cpp_attribute(x) 0
#endif

#if ! defined(__has_builtin)
#  define __has_builtin(x) 0
#endif

#endif // MSGPLUS_VERSION_HPP
