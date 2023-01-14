#include "math.h"

#include <types.h>

double fmod(double x, double y) {
    union {
        double f;
        uint64_t i;
    } ux = {x}, uy = {y};
    
    int ex = ux.i >> 52 & 0x7ff;
    int ey = uy.i >> 52 & 0x7ff;
    int sx = ux.i >> 63;
    uint64_t i;

    /* in the followings uxi should be ux.i, but then gcc wrongly adds */
    /* float load/store to inner loops ruining performance and code size */
    uint64_t uxi = ux.i;

    if (uy.i << 1 == 0 || ex == 0x7ff) return (x * y) / (x * y);
    if (uxi << 1 <= uy.i << 1) {
        if (uxi << 1 == uy.i << 1) return 0 * x;
        return x;
    }

    /* normalize x and y */
    if (!ex) {
        for (i = uxi << 12; i >> 63 == 0; ex--, i <<= 1)
            ;
        uxi <<= -ex + 1;
    } else {
        uxi &= -1ULL >> 12;
        uxi |= 1ULL << 52;
    }
    if (!ey) {
        for (i = uy.i << 12; i >> 63 == 0; ey--, i <<= 1)
            ;
        uy.i <<= -ey + 1;
    } else {
        uy.i &= -1ULL >> 12;
        uy.i |= 1ULL << 52;
    }

    /* x mod y */
    for (; ex > ey; ex--) {
        i = uxi - uy.i;
        if (i >> 63 == 0) {
            if (i == 0) return 0 * x;
            uxi = i;
        }
        uxi <<= 1;
    }
    i = uxi - uy.i;
    if (i >> 63 == 0) {
        if (i == 0) return 0 * x;
        uxi = i;
    }
    for (; uxi >> 52 == 0; uxi <<= 1, ex--)
        ;

    /* scale result */
    if (ex > 0) {
        uxi -= 1ULL << 52;
        uxi |= (uint64_t)ex << 52;
    } else {
        uxi >>= -ex + 1;
    }
    uxi |= (uint64_t)sx << 63;
    ux.i = uxi;
    return ux.f;
}

static double _dbl_inv_fact[] = {
    1.0 / 1.0,                   // 1 / 1!
    1.0 / 6.0,                   // 1 / 3!
    1.0 / 120.0,                 // 1 / 5!
    1.0 / 5040.0,                // 1 / 7!
    1.0 / 362880.0,              // 1 / 9!
    1.0 / 39916800.0,            // 1 / 11!
    1.0 / 6227020800.0,          // 1 / 13!
    1.0 / 1307674368000.0,       // 1 / 15!
    1.0 / 355687428096000.0,     // 1 / 17!
    1.0 / 121645100408832000.0,  // 1 / 19!
};

double sin(double x) {
    double x_squared;
    double sin_x;
    size_t i;

    /* Move x to [-pi, pi) */

    x = fmod(x, 2 * M_PI);
    if (x >= M_PI) {
        x -= 2 * M_PI;
    }

    if (x < -M_PI) {
        x += 2 * M_PI;
    }

    /* Move x to [-pi/2, pi/2) */

    if (x >= M_PI_2) {
        x = M_PI - x;
    }

    if (x < -M_PI_2) {
        x = -M_PI - x;
    }

    x_squared = x * x;
    sin_x = 0.0;

    /* Perform Taylor series approximation for sin(x) with ten terms */

    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sin_x += x * _dbl_inv_fact[i];
        } else {
            sin_x -= x * _dbl_inv_fact[i];
        }

        x *= x_squared;
    }

    return sin_x;
}
