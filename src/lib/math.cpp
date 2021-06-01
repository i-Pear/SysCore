#include "math.h"


size_t ppow(size_t x, size_t y) {
    size_t res = 1;
    while (y) {
        if (y & 1)
            res *= x;
        x *= x;
        y >>= 1;
    }
    return res;
}