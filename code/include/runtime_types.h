#include <stdio.h>
#include <stdlib.h>

#ifdef __ARM_NEON
#else
#define float32_t float
#endif