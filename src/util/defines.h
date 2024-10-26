#ifndef COSMORIA_DEFINES
#define COSMORIA_DEFINES

// Signed integer types
typedef signed char        i8;    // 8-bit signed integer
typedef signed short       i16;   // 16-bit signed integer
typedef signed int         i32;   // 32-bit signed integer
typedef signed long long   i64;   // 64-bit signed integer

// Unsigned integer types
typedef unsigned char      u8;   // 8-bit unsigned integer
typedef unsigned short     u16;  // 16-bit unsigned integer
typedef unsigned int       u32;  // 32-bit unsigned integer
typedef unsigned long long u64;  // 64-bit unsigned integer

// Floating point types
typedef float              f32; // 32-bit floating point
typedef double             f64; // 64-bit floating point

// Boolean type
typedef unsigned char      b8;    // Boolean type (1 byte)

// Define true and false for boolean logic
#define true  1
#define false 0

// Custom return states
#define COSMORIA_SUCCESS 1
#define COSMORIA_FAILURE 0


// Vectors

typedef struct Vec2f
{
    f32 x;
    f32 y;
} Vec2f;

typedef struct Vec2i
{
    i32 x;
    i32 y;
} Vec2i;

typedef struct Vec2u
{
    u32 x;
    u32 y;
} Vec2u;

// Util defines

// Count the number of elements in array.
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))


#endif
