#ifndef FAST_SQRT_AND_INV_SQRT_APPROX_HPP
#define FAST_SQRT_AND_INV_SQRT_APPROX_HPP

// Wikipedia - Methods of computing square roots
// Approximations that depend on the floating point representation
// https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Approximations_that_depend_on_the_floating_point_representation
/* Assumes that float is in the IEEE 754 single precision floating point format */
float sqrt_approx(float z);

// Wikipedia - Fast inverse square root
// Used by Quake III, but without the undefined behavior.
// https://en.wikipedia.org/wiki/Fast_inverse_square_root
float q_rsqrt(float number);

// Wikipedia - Fast inverse square root
// An even better tunned approximation, with lower error.
// "Jan Kadlec reduced the relative error by a further factor
//  of 2.7 by adjusting the constants in the single Newton's
//  method iteration as well,[32] arriving after an exhaustive
//  search at"
// https://en.wikipedia.org/wiki/Fast_inverse_square_root
float q_rsqrt_v2_more_precise(float number);

double diff_error_signed(float a_precise, float b_aprox);
float error_percentage(float a_precise, float b_aprox);
void test_max_pos_error_methods();
void test_performance_speed_methods(void);
float gen_rand_float(float min, float max);
void test_random_accum_accuracy_methods();

#endif /* FAST_SQRT_AND_INV_SQRT_APPROX_HPP */