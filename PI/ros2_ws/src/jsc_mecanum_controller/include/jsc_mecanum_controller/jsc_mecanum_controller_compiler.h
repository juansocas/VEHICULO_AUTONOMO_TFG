
#ifndef __JSC_MECANUM_CONTROLLER__MECANUM_CONTROLLER_COMPILER_H__
#define __JSC_MECANUM_CONTROLLER__MECANUM_CONTROLLER_COMPILER_H__

#if defined _WIN32 || defined __CYGWIN__
#   ifdef __GNUC__
#       define JSC_MECANUM_CONTROLLER_EXPORT __attribute__((dllexport))
#       define JSC_MECANUM_CONTROLLER_IMPORT __attribute__((dllimport))
#   else
#       define JSC_MECANUM_CONTROLLER_EXPORT __declspec(dllexport)
#       define JSC_MECANUM_CONTROLLER_IMPORT __declspec(dllimport)
#   endif
#   ifdef JSC_MECANUM_CONTROLLER_BUILDING_DLL
#       define JSC_MECANUM_CONTROLLER_PUBLIC JSC_MECANUM_CONTROLLER_EXPORT
#   else
#       define JSC_MECANUM_CONTROLLER_PUBLIC JSC_MECANUM_CONTROLLER_IMPORT
#   endif
#   define JSC_MECANUM_CONTROLLER_PUBLIC_TYPE JSC_MECANUM_CONTROLLER_PUBLIC
#   define JSC_MECANUM_CONTROLLER_LOCAL
#else
#   define JSC_MECANUM_CONTROLLER_EXPORT __attribute__((visibility("default")))
#   define JSC_MECANUM_CONTROLLER_IMPORT
#   if __GNUC__ >= 4
#       define JSC_MECANUM_CONTROLLER_PUBLIC __attribute__((visibility("default")))
#       define JSC_MECANUM_CONTROLLER_LOCAL __attribute__((visibility("hidden")))
#   else
#       define JSC_MECANUM_CONTROLLER_PUBLIC
#       define JSC_MECANUM_CONTROLLER_LOCAL
#   endif
#   define JSC_MECANUM_CONTROLLER_PUBLIC_TYPE
#endif

#endif // __JSC_MECANUM_CONTROLLER__MECANUM_CONTROLLER_COMPILER_H__