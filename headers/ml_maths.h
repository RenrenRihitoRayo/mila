// This project is licensed under the GNU Affero General Public License

#pragma once

#include <math.h>
#include <stdint.h>

int malloc_sprintf(char **strp, const char *fmt, ...);

// double-double float implementation
// hypothetically 102 bits in precision
typedef struct {
    double hi, lo;
} mila_float128_internal;
#define INFINITY128 (mila_float128_internal){INFINITY, INFINITY}
#define NINFINITY128 (mila_float128_internal){-INFINITY, -INFINITY}

static inline void two_sum(double a, double b, double* s, double* err);
static inline void quick_two_sum(double a, double b, double* s, double* err);
static inline void two_prod(double a, double b, double* p, double* err);
mila_float128_internal b_ff_add(mila_float128_internal a, mila_float128_internal b);
mila_float128_internal b_ff_sub(mila_float128_internal a, mila_float128_internal b);
mila_float128_internal b_ff_mul(mila_float128_internal a, mila_float128_internal b);
mila_float128_internal b_ff_div(mila_float128_internal a, mila_float128_internal b);
mila_float128_internal b_ff_neg(mila_float128_internal a);
mila_float128_internal b_ff_from_long(long x);
mila_float128_internal b_ff_from_ulong(unsigned long x);
mila_float128_internal b_ff_from_double(double x);
double b_ff_to_double(mila_float128_internal a);
int b_ff_cmp(mila_float128_internal a, mila_float128_internal b);
mila_float128_internal b_ff_from_string(const char* s);
static __int128 b_ff_to_i128(mila_float128_internal x);
static long b_ff_to_long(mila_float128_internal x);
static unsigned long b_ff_to_ulong(mila_float128_internal x);
static __int128 b_ff_to_i128_round(mila_float128_internal x);
static mila_float128_internal b_ff_from_i128(__int128 v);
static mila_float128_internal b_ff_from_i128_quick(__int128 v);
static int b_ff_is_zero(mila_float128_internal x);
static int b_ff_sign(mila_float128_internal x);
static mila_float128_internal b_ff_abs(mila_float128_internal x);
char* b_ff_to_string(mila_float128_internal x);
int b_ff_is_nan(mila_float128_internal x);
int b_ff_is_inf(mila_float128_internal x);