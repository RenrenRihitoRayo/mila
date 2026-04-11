#pragma once

#include "ml_string.c"
#include <math.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int our_asprintf(char **strp, const char *fmt, ...);

typedef struct {
    double hi, lo;
} mila_float128_internal;
#define INFINITY128 (mila_float128_internal){INFINITY, INFINITY}
#define NINFINITY128 (mila_float128_internal){-INFINITY, -INFINITY}

#ifdef MILA_PROTO
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

#else

static inline void two_sum(double a, double b, double* s, double* err) {
    *s = a + b;
    double v = *s - a;
    *err = (a - (*s - v)) + (b - v);
}

static inline void quick_two_sum(double a, double b, double* s, double* err) {
    *s = a + b;
    *err = b - (*s - a);
}

static inline void two_prod(double a, double b, double* p, double* err) {
    *p = a * b;

    const double split = 134217729.0;
    double a_hi = a * split;
    double a_lo = a - a_hi;
    a_hi = a_hi + a_lo;
    a_lo = a - a_hi;

    double b_hi = b * split;
    double b_lo = b - b_hi;
    b_hi = b_hi + b_lo;
    b_lo = b - b_hi;

    *err = ((a_hi * b_hi - *p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
}

mila_float128_internal b_ff_add(mila_float128_internal a, mila_float128_internal b) {
    mila_float128_internal res;
    double s = a.hi + b.hi;
    double v = s - a.hi;
    double t = ((b.hi - v) + (a.hi - (s - v))) + a.lo + b.lo;

    res.hi = s + t;
    res.lo = t - (res.hi - s);
    return res;
}

mila_float128_internal b_ff_sub(mila_float128_internal a, mila_float128_internal b) {
    mila_float128_internal r;
    double s, e;

    two_sum(a.hi, -b.hi, &s, &e);
    e += a.lo - b.lo;

    quick_two_sum(s, e, &r.hi, &r.lo);
    return r;
}

mila_float128_internal b_ff_mul(mila_float128_internal a, mila_float128_internal b) {
    mila_float128_internal r;
    double p, e;

    two_prod(a.hi, b.hi, &p, &e);
    e += a.hi * b.lo + a.lo * b.hi;

    quick_two_sum(p, e, &r.hi, &r.lo);
    return r;
}

mila_float128_internal b_ff_div(mila_float128_internal a, mila_float128_internal b) {
    mila_float128_internal r;

    double approx = a.hi / b.hi;

    mila_float128_internal q = { approx, 0.0 };
    mila_float128_internal prod = b_ff_mul(b, q);
    mila_float128_internal diff = b_ff_sub(a, prod);

    double correction = diff.hi / b.hi;

    double s, e;
    quick_two_sum(approx, correction, &s, &e);

    r.hi = s;
    r.lo = e;
    return r;
}

mila_float128_internal b_ff_neg(mila_float128_internal a) {
    return (mila_float128_internal){ -a.hi, -a.lo };
}

mila_float128_internal b_ff_from_long(long x) {
    return (mila_float128_internal){ (double)x, 0.0 };
}

mila_float128_internal b_ff_from_ulong(unsigned long x) {
    return (mila_float128_internal){ (double)x, 0.0 };
}

mila_float128_internal b_ff_from_double(double x) {
    if (isinf(x))
    {
        if (x == INFINITY) return INFINITY128;
        else return NINFINITY128;
    } else if (isnan(x)) return (mila_float128_internal){ 0.0, 0.0 };
    return (mila_float128_internal){ x, 0.0 };
}

double b_ff_to_double(mila_float128_internal a) {
    return a.hi + a.lo;
}

int b_ff_cmp(mila_float128_internal a, mila_float128_internal b) {
    if (a.hi < b.hi) return -1;
    if (a.hi > b.hi) return 1;
    if (a.lo < b.lo) return -1;
    if (a.lo > b.lo) return 1;
    return 0;
}

mila_float128_internal b_ff_from_string(const char* s) {
    while (isspace(*s)) s++;
    int sign = 1;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    
    if (*s == 'i' || *s == 'I') {
        if ((s[1] == 'n' || s[1] == 'N') && (s[2] == 'f' || s[2] == 'F')) {
            return sign < 0 ? NINFINITY128 : INFINITY128;
        }
    }
    if (*s == 'n' || *s == 'N') {
        if ((s[1] == 'a' || s[1] == 'A') && (s[2] == 'n' || s[2] == 'N')) {
            return (mila_float128_internal){NAN, NAN};
        }
    }
    
    mila_float128_internal result = {0.0, 0.0};
    int has_digits = 0;
    
    while (isdigit(*s)) {
        has_digits = 1;
        int digit = *s++ - '0';
        result = b_ff_add(b_ff_mul(result, b_ff_from_double(10.0)),
                        b_ff_from_double((double)digit));
    }
    
    if (*s == '.') {
        s++;
        mila_float128_internal scale = b_ff_from_double(0.1);

        while (isdigit(*s)) {
            has_digits = 1;
            int digit = *s++ - '0';
            result = b_ff_add(result,
                            b_ff_mul(b_ff_from_double((double)digit), scale));
            scale = b_ff_mul(scale, b_ff_from_double(0.1));
        }
    }
    
    if ((*s == 'e' || *s == 'E') && has_digits) {
        s++;
        int exp_sign = 1;
        if (*s == '-') { exp_sign = -1; s++; }
        else if (*s == '+') s++;
        int exp = 0;
        int exp_has_digits = 0;
        while (isdigit(*s)) {
            exp_has_digits = 1;
            exp = exp * 10 + (*s++ - '0');
            if (exp > 308) exp = 308;
        }
        if (exp_has_digits) {
            exp *= exp_sign;
            mila_float128_internal ten = b_ff_from_double(10.0);
            mila_float128_internal factor = b_ff_from_double(1.0);
            int e = exp > 0 ? exp : -exp;
            while (e > 0) {
                if (e & 1)
                    factor = b_ff_mul(factor, ten);
                if (e > 1)
                    ten = b_ff_mul(ten, ten);
                e >>= 1;
            }
            if (exp > 0)
                result = b_ff_mul(result, factor);
            else if (exp < 0)
                result = b_ff_div(result, factor);
        }
    }
    
    if (sign < 0)
        result = b_ff_neg(result);
    return result;
}

__int128 b_ff_to_i128(mila_float128_internal x) {
    return (__int128)(x.hi);
}

long b_ff_to_long(mila_float128_internal x) {
    return (long)(x.hi);
}

unsigned long b_ff_to_ulong(mila_float128_internal x) {
    return (unsigned long)(x.hi);
}

__int128 b_ff_to_i128_round(mila_float128_internal x) {
    if (x.hi >= 0.0)
        return (__int128)(x.hi + 0.5);
    else
        return (__int128)(x.hi - 0.5);
}

mila_float128_internal b_ff_from_i128(__int128 v) {
    const double split = 134217729.0;

    double hi = (double)v;
    __int128 tmp = (__int128)hi;
    double lo = (double)(v - tmp);

    return (mila_float128_internal){ hi, lo };
}

mila_float128_internal b_ff_from_i128_quick(__int128 v) {
    return (mila_float128_internal){ (double)v, 0.0 };
}

int b_ff_is_zero(mila_float128_internal x) {
    return x.hi == 0.0 && x.lo == 0.0;
}

int b_ff_sign(mila_float128_internal x) {
    if (x.hi != 0.0) return x.hi < 0.0 ? -1 : 1;
    return x.lo < 0.0 ? -1 : 1;
}

mila_float128_internal b_ff_abs(mila_float128_internal x) {
    if (b_ff_sign(x) < 0) {
        x.hi = -x.hi;
        x.lo = -x.lo;
    }
    return x;
}

int b_ff_is_inf(mila_float128_internal x) {
    return isinf(x.hi) || isinf(x.lo);
}

char* b_ff_to_string(mila_float128_internal x) {
    int neg = 0;
    if (b_ff_is_inf(x)) {
        if (b_ff_cmp(x, INFINITY128)==0)
            return mila_strdup("inf");
        else
            return mila_strdup("-inf");
    }
    
    if (b_ff_sign(x) < 0) {
        neg = 1;
        x = b_ff_abs(x);
    }

    int bufcap = 256;
    char *buf = (char*)malloc(bufcap);
    int pos = 0;

    if (neg) buf[pos++] = '-';

    long long ip = (long long)x.hi;
    
    if (ip == 0) {
        buf[pos++] = '0';
    } else {
        char tmp[64];
        int t = 0;
        long long temp = ip;
        while (temp > 0) {
            tmp[t++] = '0' + (temp % 10);
            temp /= 10;
        }
        while (t > 0) {
            buf[pos++] = tmp[--t];
        }
    }

    mila_float128_internal frac = b_ff_sub(x, b_ff_from_double((double)ip));

    if (b_ff_is_zero(frac)) {
        buf[pos++] = '.';
        buf[pos++] = '0';
        buf[pos] = '\0';
        return buf;
    }

    buf[pos++] = '.';

    int max_digits = 120;
    int digits = 0;
    int trailing_zeros = 0;

    while (!b_ff_is_zero(frac) && digits < max_digits) {
        frac = b_ff_mul(frac, b_ff_from_double(10.0));
        int digit = (int)frac.hi;
        
        if (digit < 0) digit = 0;
        if (digit > 9) digit = 9;
        
        buf[pos++] = '0' + digit;
        frac = b_ff_sub(frac, b_ff_from_double((double)digit));
        digits++;

        if (digit != 0) {
            trailing_zeros = 0;
        } else {
            trailing_zeros++;
        }

        if (pos >= bufcap - 10) {
            bufcap *= 2;
            buf = (char*)realloc(buf, bufcap);
        }
    }

    if (trailing_zeros > 0 && digits > trailing_zeros) {
        pos -= trailing_zeros;
    }

    buf[pos] = '\0';
    return buf;
}

int b_ff_is_nan(mila_float128_internal x) {
    return isnan(x.hi) || isnan(x.lo);
}
#endif