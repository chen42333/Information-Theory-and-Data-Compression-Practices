#ifndef __CONFIG_H
#define __CONFIG_H

#include <cstdint>

#if (BITS == 64)
typedef uint64_t alphabet;
#elif (BITS == 32)
typedef uint32_t alphabet;
#elif (BITS == 16)
typedef uint16_t alphabet;
#elif (BITS == 8)
typedef uint8_t alphabet;
#else
#error "Unsupported alphabet size"
#endif

#endif