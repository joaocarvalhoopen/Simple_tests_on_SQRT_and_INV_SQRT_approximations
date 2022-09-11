# Simple tests on SQRT and INV_SQRT approximations
Just to test the error percentage and the speedup against stdlib implementation.

## Description
See the following output of the program that contains more information and the results. <br>
<br>

```
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
```


## References
* Wikipedia - Methods of computing square roots. <br>  
  Approximations that depend on the floating point representation. <br> 
  Number format IEEE 754 single precision <br>
  [https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Approximations_that_depend_on_the_floating_point_representation ](https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Approximations_that_depend_on_the_floating_point_representation )
    
* Wikipedia - Fast inverse square root. <br>
  Used by Quake III, but without the undefined behavior. <br>
  [https://en.wikipedia.org/wiki/Fast_inverse_square_root](https://en.wikipedia.org/wiki/Fast_inverse_square_root)

* Wikipedia - Fast inverse square root better approx <br>
  An even better tunned approximation, with lower error. <br>
  "Jan Kadlec reduced the relative error by a further factor <br> 
  of 2.7 by adjusting the constants in the single Newton's <br> 
  method iteration as well,[32] arriving after an exhaustive <br>
  search at n" <br>
  [https://en.wikipedia.org/wiki/Fast_inverse_square_root](https://en.wikipedia.org/wiki/Fast_inverse_square_root) 


## License
MIT Open Source License


## Have fun
Best regards, <br>
João Nuno Carvalho
