#ifndef MSGPLUS_HPP
#define MSGPLUS_HPP

#define MSGPLUS_VERSION_MAJOR 0
#define MSGPLUS_VERSION_MINOR 1
#define MSGPLUS_VERSION_PATCH 0

#ifndef __cplusplus
#    error "__cplusplus is not defined"
#endif

#if defined(_MSVC_LANG) && defined(_MSC_VER) && 190024210 <= _MSC_FULL_VER
#  define MSGPLUS_CPLUSPLUS_STANDARD_VERSION _MSVC_LANG
#else
#  define MSGPLUS_CPLUSPLUS_STANDARD_VERSION __cplusplus
#endif

#if MSGPLUS_CPLUSPLUS_STANDARD_VERSION < 202002L
#    error "msgplus requires C++20 or later."
#endif

// IWYU pragma: begin_exports
#include "msgplus/flat_map.hpp"
#include "msgplus/ordered_map.hpp"
#include "msgplus/value.hpp"
#include "msgplus/reader.hpp"
#include "msgplus/read.hpp"
#include "msgplus/writer.hpp"
#include "msgplus/write.hpp"
// IWYU pragma: end_exports

#endif//MSGPLUS_HPP
