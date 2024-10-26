#ifndef COSMORIA_MATH
#define COSMORIA_MATH


#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef CLMAP
#define CLAMP(x, upper, lower) (MIN(upper, MAX(x, lower)))
#endif

#endif