// This project is licensed under the GNU Affero General Public License
#pragma once

#include "mila.h"

Value* native_sys_get_platform(Env* env, int argc, Value** argv) {
// Supported and managed platforms
#ifdef __linux__
    return vstring_dup("linux");
#elif defined(__EMSCRIPTEN__) || defined(__wasi__)
    return vstring_dup("web");
#elif defined(__ANDROID__)
    return vstring_dup("android");
// Untested platforms
#elif defined(_WIN32) || defined(_WIN64)
    return vstring_dup("win");
#elif defined(__APPLE__) || defined(__MACH__)
// for apple devs that like being specific
#ifdef APPLE_MACH
    return vstring_dup("mach");
#elif defined(APPLE_IOS)
    return vstring_dup("ios");
#elif defined(APPLE_TV)
    return vstring_dup("tvOS"); // why not...
#else
    return vstring_dup("apple");
#endif
// Maybe?
#elif defined(__unix__)
    return vstring_dup("unix");
// Good luck
#else
    return vstring_dup("unknown");
#endif
}

Value* native_sys_get_arch(Env* env, int argc, Value** argv) {
// Architectures that ARE supported
#if defined(__x86_64__) || defined(_M_X64)
    return vstring_dup("x86_64");
#elif defined(__i386__) || defined(_M_IX86)
    return vstring_dup("x86");
#elif defined(__aarch64__) || defined(_M_ARM64)
    return vstring_dup("arm64");
#elif defined(__arm__) || defined(_M_ARM)
    return vstring_dup("arm");
// Not so supported platforms
// Might add partial support due to abundance
#elif defined(__riscv)
    return vstring_dup("riscv");
// Might never get support (and even removed here)
#elif defined(__powerpc64__) || defined(__ppc64__)
    return vstring_dup("ppc64");
#elif defined(__powerpc__) || defined(__ppc__)
    return vstring_dup("ppc");
#elif defined(__mips__)
    return vstring_dup("mips");
// All hope is lost, if its too "niche" it may never get added here
#else
    return vstring_dup("unknown");
#endif
}