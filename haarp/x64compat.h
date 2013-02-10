#ifndef X64COMPAT_H
#define X64COMPAT_H

#include "default.h"

# if __WORDSIZE == 64
#  define LLX        "%lx"
#  define LLD        "%ld"
# else
#  define LLX        "%llx"
#  define LLD        "%lld"
# endif

#endif
