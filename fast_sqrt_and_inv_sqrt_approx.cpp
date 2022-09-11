// fast_sqrt_and_inv_sqrt_aprox
// Author: João Nuno Carvalho
// Date:   2022.09.11

// To Run:
//    > make o3
//    > ./a.out


/* Program output:

> ./a.out

===========
Simple tests of sqrt() approximation and inv_sqrt() Quake III approximation 
in float32. 

Currently there are SIMD instructions to calculate SQRT() and INV_SQRT() 
that are faster then this approximations, but for embedded systems this 
this optimizations give a real boost in performance. 

Tested on x86_64 AMD Ryzen 7 2700,  Linux in GCC with optimization flags -O3 

Functions: 
sqrt(a)         - Standard C or C++ math.h lib in float 32 bits. 
sqrt_approx(a)  - Wikipedia - Methods of computing square roots.  
                  Approximations that depend on the floating point representation. 
                  Number format IEEE 754 single precision 
                  https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Approximations_that_depend_on_the_floating_point_representation 
    
1/sqrt(a)  - Standard C or C++ math.h lib in float 32 bits. 
q_rsqrt(a) - Wikipedia - Fast inverse square root. 
             Used by Quake III, but without the undefined behavior. 
             https://en.wikipedia.org/wiki/Fast_inverse_square_root  
q_rsqrt_v2_more_precise(a) - 
           - Wikipedia - Fast inverse square root better approx  
             An even better tunned approximation, with lower error. 
            "Jan Kadlec reduced the relative error by a further factor 
             of 2.7 by adjusting the constants in the single Newton's  
             method iteration as well,[32] arriving after an exhaustive 
             search at n" 
             https://en.wikipedia.org/wiki/Fast_inverse_square_root 

========
Results: 

Error calculated at equal intervals:

  Sqrt approx:
    sqrt_approx_max_abs_perc_error: 3.474731 % 

  Inverse sqrt approx Quake III:
    q_rsqrt_approx_max_abs_perc_error: 0.175235 % 

  Inverse sqrt V2 approx, more precise:
    q_rsqrt_v2_approx_max_abs_perc_error: 0.065024 % lower_error: x2.695 times 


Performance: 

  Elapsed sqrt() stdlib: 0.054588 seconds 

  Elapsed sqrt() approximation: 0.015391 seconds, speedup: x3.547 

  Elapsed inverse rsqrt() stdlib: 0.065589 seconds 

  Elapsed inv q_rsqrt() Quake III approximation: 0.020726 seconds, speedup: x3.165 

  Elapsed inv q_rsqrt_v2_more_precise() approximation: 0.022856 seconds, speedup: x2.870 

  Total_sum_value: -nan IGNORE


Error calculated with random values.... 

  Medium_error_sqrt_approx_perc:        1.570 % 

  Medium_error_inv_sqrt_approx_perc:    0.098 % 
  Medium_error_inv_sqrt_approx_v2_perc: 0.038 % 

*/

#include <stdio.h>
#include <math.h>
#include <time.h>

#include <float.h>
#include <fenv.h>

#include <stdint.h>

#include "fast_sqrt_and_inv_sqrt_approx.hpp"

// Number simulation steps.
#define NUM_STEPS ((int) 1000000); 

// Important note: There is also a version for 64 bits.

// Wikipedia - Methods of computing square roots
// Approximations that depend on the floating point representation
// https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Approximations_that_depend_on_the_floating_point_representation
/* Assumes that float is in the IEEE 754 single precision floating point format */
float sqrt_approx(float z)
{
	union { float f; uint32_t i; } val = {z};	/* Convert type, preserving bit pattern */
	/*
	 * To justify the following code, prove that
	 *
	 * ((((val.i / 2^m) - b) / 2) + b) * 2^m = ((val.i - 2^m) / 2) + ((b + 1) / 2) * 2^m)
	 *
	 * where
	 *
	 * b = exponent bias
	 * m = number of mantissa bits
	 */
	val.i -= 1 << 23;	/* Subtract 2^m. */
	val.i >>= 1;		/* Divide by 2. */
	val.i += 1 << 29;	/* Add ((b + 1) / 2) * 2^m. */
    val.i += -0x4b0d2;  // Adjustment to lower the maximum absolute error from ~6 % (measured 6.066 %) to ~3.5 % ( measured 3.474 %) .
	return val.f;		/* Interpret again as float */
}

// Wikipedia - Fast inverse square root
// Used by Quake III, but without the undefined behavior.
// https://en.wikipedia.org/wiki/Fast_inverse_square_root
float q_rsqrt(float number)
{
	union {
		float    f;
		uint32_t i;
	} conv = { .f = number };
	conv.i  = 0x5f3759df - (conv.i >> 1);
	conv.f *= 1.5F - (number * 0.5F * conv.f * conv.f);
	return conv.f;
}

// Wikipedia - Fast inverse square root
// An even better tunned approximation, with lower error.
// "Jan Kadlec reduced the relative error by a further factor
//  of 2.7 by adjusting the constants in the single Newton's
//  method iteration as well,[32] arriving after an exhaustive
//  search at"
// https://en.wikipedia.org/wiki/Fast_inverse_square_root
float q_rsqrt_v2_more_precise(float number)
{
	union {
		float    f;
		uint32_t i;
	} conv = { .f = number };
    conv.i = 0x5F1FFFF9 - ( conv.i >> 1 );
	conv.f *= 0.703952253f * ( 2.38924456f - number * conv.f * conv.f );
	return conv.f;
}

double diff_error_signed(float a_precise, float b_aprox) {
    double diff = (double) b_aprox -  (double) a_precise;
    // if (fabs(a_precise) < 0.0000001)
    if (abs(a_precise) <= FLT_MIN)
        diff = b_aprox;
    return diff;
}

float error_percentage(float a_precise, float b_aprox) {
    double diff = (double) b_aprox -  (double) a_precise;
    // if (fabs(a_precise) < 0.0000001)
    if (abs(a_precise) <= FLT_MIN)
        diff = b_aprox;
    float err_percent = 0.0f;
    if (a_precise != 0)
       err_percent = (float) ((abs(diff) * 100) / abs(a_precise)); 
    return err_percent;
}

void test_max_pos_error_methods(void) {
    // const int NUM_STEPS = 1000;
    const int TOTAL_NUM_STEPS = 2 * NUM_STEPS ; // 0 .... 2*Pi and -2*Pi..0
    float inc = (FLT_MAX) / NUM_STEPS;
    float sqrt_approx_max_abs_perc_error       = 0.0;
    float q_rsqrt_approx_max_abs_perc_error    = 0.0;
    float q_rsqrt_v2_approx_max_abs_perc_error = 0.0;

    float a = 0.0;
    for(int j = 0; j < TOTAL_NUM_STEPS; j++) {
        a += inc;

        // Test sqrt(float) approximation.
        float prec_v_pos = sqrt(a);
        float approx_pos = sqrt_approx(a);
        // Calc in percentages.
        float diff_perc_approx = error_percentage(prec_v_pos, approx_pos);
        if (diff_perc_approx > sqrt_approx_max_abs_perc_error) {
            sqrt_approx_max_abs_perc_error = diff_perc_approx;
        }

        // Test q_rsqrt(float) approximation. Quake III
        prec_v_pos = 1/sqrt(a);
        approx_pos = q_rsqrt(a);
        // Calc in percentages.
        diff_perc_approx = error_percentage(prec_v_pos, approx_pos);
        if (diff_perc_approx > q_rsqrt_approx_max_abs_perc_error) {
            q_rsqrt_approx_max_abs_perc_error = diff_perc_approx;
        }

        // Test q_rsqrt_v2_more_precise(float) approximation.
        prec_v_pos = 1/sqrt(a);
        approx_pos = q_rsqrt_v2_more_precise(a);
        // Calc in percentages.
        diff_perc_approx = error_percentage(prec_v_pos, approx_pos);
        if (diff_perc_approx > q_rsqrt_v2_approx_max_abs_perc_error) {
            q_rsqrt_v2_approx_max_abs_perc_error = diff_perc_approx;
        }

    }

    printf("\nError calculated at equal intervals:\n");
    printf("\n  Sqrt approx:\n");
    printf("    sqrt_approx_max_abs_perc_error: %f %% \n\n", sqrt_approx_max_abs_perc_error);
    printf("  Inverse sqrt approx Quake III:\n");
    printf("    q_rsqrt_approx_max_abs_perc_error: %f %% \n\n", q_rsqrt_approx_max_abs_perc_error);
    printf("  Inverse sqrt V2 approx, more precise:\n");
    printf("    q_rsqrt_v2_approx_max_abs_perc_error: %f %% lower_error: x%.3f times \n\n",
        q_rsqrt_v2_approx_max_abs_perc_error, q_rsqrt_approx_max_abs_perc_error / q_rsqrt_v2_approx_max_abs_perc_error);
}

void test_performance_speed_methods(void) {
    // const int NUM_STEPS = 1000;
    const int TOTAL_NUM_STEPS = 2 * NUM_STEPS; // 0 .... 2*Pi and -2*Pi..0
    float inc = (FLT_MAX) / NUM_STEPS;

    printf("\nPerformance: \n");

    // Test sqrt(float) stdlib.
    clock_t tic_1 = clock();

    float a = 0.0;
    float total = 0.0;
    for(int j = 0; j < TOTAL_NUM_STEPS; j++) {
        a += inc;

        total += sqrt(a);
        total += sqrt(a - 0.00000001);
        total += sqrt(a - 0.00000002);
        total += sqrt(a - 0.00000003);
        total += sqrt(a - 0.00000004);
        total += sqrt(a - 0.00000005);
        total += sqrt(a - 0.00000006);
        total += sqrt(a - 0.00000007);
        total += sqrt(a - 0.00000008);
        total += sqrt(a - 0.00000009);
    }

    clock_t toc_2 = clock();
    double time_1 = (double)(toc_2 - tic_1) / CLOCKS_PER_SEC;
    printf("\n  Elapsed sqrt() stdlib: %f seconds \n", time_1);


    // Test sqrt_approx(float) approximation.
    tic_1 = clock();

    a = 0.0;
    // total = 0.0;
    for(int j = 0; j < TOTAL_NUM_STEPS; j++) {
        a += inc;

        total += sqrt_approx(a);
        total += sqrt_approx(a - 0.00000001);
        total += sqrt_approx(a - 0.00000002);
        total += sqrt_approx(a - 0.00000003);
        total += sqrt_approx(a - 0.00000004);
        total += sqrt_approx(a - 0.00000005);
        total += sqrt_approx(a - 0.00000006);
        total += sqrt_approx(a - 0.00000007);
        total += sqrt_approx(a - 0.00000008);
        total += sqrt_approx(a - 0.00000009);
    }

    toc_2 = clock();
    double time_2 = (double)(toc_2 - tic_1) / CLOCKS_PER_SEC;
    printf("\n  Elapsed sqrt() approximation: %f seconds, speedup: x%.3f \n", time_2, time_1 / time_2);
    

    // Test sqrt(float) stdlib.
    tic_1 = clock();

    a = 0.0;
    // total = 0.0;
    for(int j = 0; j < TOTAL_NUM_STEPS; j++) {
        a += inc;

        total += 1/sqrt(a);
        total += 1/sqrt(a - 0.00000001);
        total += 1/sqrt(a - 0.00000002);
        total += 1/sqrt(a - 0.00000003);
        total += 1/sqrt(a - 0.00000004);
        total += 1/sqrt(a - 0.00000005);
        total += 1/sqrt(a - 0.00000006);
        total += 1/sqrt(a - 0.00000007);
        total += 1/sqrt(a - 0.00000008);
        total += 1/sqrt(a - 0.00000009);
    }

    toc_2 = clock();
    double time_3 = (double)(toc_2 - tic_1) / CLOCKS_PER_SEC;
    printf("\n  Elapsed inverse rsqrt() stdlib: %f seconds \n", time_3);
    

    // Test q_rsqrt(float) approximation - Quake III.
    tic_1 = clock();

    a = 0.0;
    // total = 0.0;
    for(int j = 0; j < TOTAL_NUM_STEPS; j++) {
        a += inc;

        total += q_rsqrt(a);
        total += q_rsqrt(a - 0.00000001);
        total += q_rsqrt(a - 0.00000002);
        total += q_rsqrt(a - 0.00000003);
        total += q_rsqrt(a - 0.00000004);
        total += q_rsqrt(a - 0.00000005);
        total += q_rsqrt(a - 0.00000006);
        total += q_rsqrt(a - 0.00000007);
        total += q_rsqrt(a - 0.00000008);
        total += q_rsqrt(a - 0.00000009);
    }

    toc_2 = clock();
    double time_4 = (double)(toc_2 - tic_1) / CLOCKS_PER_SEC;
    printf("\n  Elapsed inv q_rsqrt() Quake III approximation: %f seconds, speedup: x%.3f \n", time_4, time_3 / time_4);
    

    // Test q_rsqrt_v2_more_precise(float) approximation.
    tic_1 = clock();

    a = 0.0;
    // total = 0.0;
    for(int j = 0; j < TOTAL_NUM_STEPS; j++) {
        a += inc;

        total += q_rsqrt_v2_more_precise(a);
        total += q_rsqrt_v2_more_precise(a - 0.00000001);
        total += q_rsqrt_v2_more_precise(a - 0.00000002);
        total += q_rsqrt_v2_more_precise(a - 0.00000003);
        total += q_rsqrt_v2_more_precise(a - 0.00000004);
        total += q_rsqrt_v2_more_precise(a - 0.00000005);
        total += q_rsqrt_v2_more_precise(a - 0.00000006);
        total += q_rsqrt_v2_more_precise(a - 0.00000007);
        total += q_rsqrt_v2_more_precise(a - 0.00000008);
        total += q_rsqrt_v2_more_precise(a - 0.00000009);
    }

    toc_2 = clock();
    double time_5 = (double)(toc_2 - tic_1) / CLOCKS_PER_SEC;
    printf("\n  Elapsed inv q_rsqrt_v2_more_precise() approximation: %f seconds, speedup: x%.3f \n", time_5, time_3 / time_5);
    
    printf("\n  Total_sum_value: %f IGNORE\n", total);
}

float gen_rand_float(float min, float max) {
    double scale = rand() / (double) RAND_MAX; // 0 .. 1.0
    return (float) (min + scale * ( max - min ));      // min .. max
}

void test_random_accum_accuracy_methods() {
    // const int NUM_STEPS = 1000;
    const int TOTAL_NUM_STEPS = 2 * NUM_STEPS;

    // Generate TOTAL_NUM_STEPS random numbers in range [0, FLT_MAX] 
    double accum_perc_approx_sqrt        = 0.0;
    double accum_perc_approx_inv_sqrt    = 0.0;
    double accum_perc_approx_inv_sqrt_v2 = 0.0;

    for(int j = 0; j < TOTAL_NUM_STEPS; j++) {
        // Generate random numbers in range.  
        float a = gen_rand_float(0, FLT_MAX);
        
        // Square Root.
        float precision_value_sqrt = sqrt(a);
        
        // Test error sqrt_approx() approximation.
        float approx_pos = sqrt_approx(a);
        // Calc in percentages.
        accum_perc_approx_sqrt = accum_perc_approx_sqrt + (double) error_percentage(precision_value_sqrt, approx_pos);

        
        // Inverse Square Root.
        float precision_value_inv_sqrt = 1/sqrt(a);
        
        // Test q_rsqrt(float) approximation. Quake III
        approx_pos = q_rsqrt(a);
        // Calc in percentages.
        accum_perc_approx_inv_sqrt = accum_perc_approx_inv_sqrt + (double) error_percentage(precision_value_inv_sqrt, approx_pos);

        // Test q_rsqrt_v2_more_precise(float) approximation. See notes and link to Wikipedia on the function.
        approx_pos = q_rsqrt_v2_more_precise(a);
        // Calc in percentages.
        accum_perc_approx_inv_sqrt_v2 = accum_perc_approx_inv_sqrt_v2 + (double) error_percentage(precision_value_inv_sqrt, approx_pos);
    } 

    float total_medium_error_sqrt_approx_perc        = (float) (accum_perc_approx_sqrt / TOTAL_NUM_STEPS);
    float total_medium_error_inv_sqrt_approx_perc    = (float) (accum_perc_approx_inv_sqrt / TOTAL_NUM_STEPS);
    float total_medium_error_inv_sqrt_approx_v2_perc = (float) (accum_perc_approx_inv_sqrt_v2 / TOTAL_NUM_STEPS);

    printf("\n\nError calculated with random values.... \n\n");
    printf("  Medium_error_sqrt_approx_perc:        %.3f %% \n\n", total_medium_error_sqrt_approx_perc);
    printf("  Medium_error_inv_sqrt_approx_perc:    %.3f %% \n", total_medium_error_inv_sqrt_approx_perc);
    printf("  Medium_error_inv_sqrt_approx_v2_perc: %.3f %% \n", total_medium_error_inv_sqrt_approx_v2_perc);
}

int main( void ) {

    char text_info[2000] = \
    "\n===========\n" \
    "Simple tests of sqrt() approximation and inv_sqrt() Quake III approximation \n" \
    "in float32. \n" \
    "\n" \
    "Currently there are SIMD instructions to calculate SQRT() and INV_SQRT() \n" \
    "that are faster then this approximations, but for embedded systems this \n" \
    "this optimizations give a real boost in performance. \n" \
    "\n" \
    "Tested on x86_64 AMD Ryzen 7 2700,  Linux in GCC with optimization flags -O3 \n" \
    "\n" \
    "Functions: \n" \

    "sqrt(a)         - Standard C or C++ math.h lib in float 32 bits. \n" \

    "sqrt_approx(a)  - Wikipedia - Methods of computing square roots.  \n" \
    "                  Approximations that depend on the floating point representation. \n" \
    "                  Number format IEEE 754 single precision \n" \
    "                  https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Approximations_that_depend_on_the_floating_point_representation \n " \
    "   \n" \

    "1/sqrt(a)  - Standard C or C++ math.h lib in float 32 bits. \n" \

    "q_rsqrt(a) - Wikipedia - Fast inverse square root. \n" \
    "             Used by Quake III, but without the undefined behavior. \n" \
    "             https://en.wikipedia.org/wiki/Fast_inverse_square_root  \n" \
    "q_rsqrt_v2_more_precise(a) - \n" \
    "           - Wikipedia - Fast inverse square root better approx  \n" \
    "             An even better tunned approximation, with lower error. \n" \
    "            \"Jan Kadlec reduced the relative error by a further factor \n" \
    "             of 2.7 by adjusting the constants in the single Newton's  \n" \
    "             method iteration as well,[32] arriving after an exhaustive \n" \
    "             search at n\" \n" \
    "             https://en.wikipedia.org/wiki/Fast_inverse_square_root \n" \
    "\n" \
    "========\n" \
    "Results: \n";

    printf("%s", text_info);

    // Test the maximum positive error in a X86_64 in all range .
    test_max_pos_error_methods();
    // Test the speed performance in a X86_64 the speed up of this costly operation.
    test_performance_speed_methods();    
    // Test average percentage error with random values.
    test_random_accum_accuracy_methods();
}

