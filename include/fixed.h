/* 
 * Fixed.h
 *
 * Description: Fixed Point Maths library (8.8) to
 * support fixed-point arithmetic on 8086 based CPUs
 * 
 */
#ifndef FIXED_H
#define FIXED_H

typedef short fixed;

#define FIX_SHIFT 8
#define FIX_ONE   (1 << FIX_SHIFT)

#define INT_TO_FIX(a)  ((fixed)((a) << FIX_SHIFT))
#define FIX_TO_INT(a)  ((int)((a) >> FIX_SHIFT))

/* fixed multiply (16×16 -> 32, then >> 8) */
static fixed FIX_MUL(fixed a, fixed b)
{
    long t = (long)a * (long)b;
    return (fixed)(t >> FIX_SHIFT);
}

/* fixed divide */
static fixed FIX_DIV(fixed a, fixed b)
{
    long t = ((long)a << FIX_SHIFT);
    return (fixed)(t / b);
}

/* float -> fixed */
static fixed FLOAT_TO_FIX(float f)
{
    return (fixed)(f * (float)FIX_ONE);
}

#endif