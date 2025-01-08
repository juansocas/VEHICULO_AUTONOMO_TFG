#ifndef __JSC_MECANUM_HARDWARE__MECANUM_HARDWARE_COMPILER_H__
#define __JSC_MECANUM_HARDWARE__MECANUM_HARDWARE_COMPILER_H__

#if defined _WIN32 || defined __CYGWIN__
#   ifdef __GNUC__
#       define JSC_MECANUM_HARDWARE_EXPORT __attribute__((dllexport))
#       define JSC_MECANUM_HARDWARE_IMPORT __attribute__((dllimport))
#   else
#       define JSC_MECANUM_HARDWARE_EXPORT __declspec(dllexport)
#       define JSC_MECANUM_HARDWARE_IMPORT __declspec(dllimport)
#   endif
#   ifdef JSC_MECANUM_HARDWARE_BUILDING_DLL
#       define JSC_MECANUM_HARDWARE_PUBLIC JSC_MECANUM_HARDWARE_EXPORT
#   else
#       define JSC_MECANUM_HARDWARE_PUBLIC JSC_MECANUM_HARDWARE_IMPORT
#   endif
#   define JSC_MECANUM_HARDWARE_PUBLIC_TYPE JSC_MECANUM_HARDWARE_PUBLIC
#   define JSC_MECANUM_HARDWARE_LOCAL
#else
#   define JSC_MECANUM_HARDWARE_EXPORT __attribute__((visibility("default")))
#   define JSC_MECANUM_HARDWARE_IMPORT
#   if __GNUC__ >= 4
#       define JSC_MECANUM_HARDWARE_PUBLIC __attribute__((visibility("default")))
#       define JSC_MECANUM_HARDWARE_LOCAL __attribute__((visibility("hidden")))
#   else
#       define JSC_MECANUM_HARDWARE_PUBLIC
#       define JSC_MECANUM_HARDWARE_LOCAL
#   endif
#   define JSC_MECANUM_HARDWARE_PUBLIC_TYPE
#endif

#endif // __JSC_MECANUM_HARDWARE__MECANUM_HARDWARE_COMPILER_H__
