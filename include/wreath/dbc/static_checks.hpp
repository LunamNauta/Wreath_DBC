#ifndef WREATH_DBC_STATIC_CHECKS
#define WREATH_DBC_STATIC_CHECKS

#include <limits>

static_assert(
    std::numeric_limits<unsigned char>::digits == 8,
    "Static Error: Byte is not 8-bits\n"
);
static_assert(
    std::numeric_limits<float>::is_iec559 && sizeof(float) == 4,
    "Static Error: Float is not IEC-559 32-bit\n"
);
static_assert(
    std::numeric_limits<double>::is_iec559 && sizeof(double) == 8,
    "Static Error: Double is not IEC-559 64-bit\n"
);

#endif
