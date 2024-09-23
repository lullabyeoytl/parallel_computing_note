#include <stdio.h>
#include <algorithm>
#include <math.h>
#include "CMU418intrin.h"
#include "logger.h"
using namespace std;


void absSerial(float* values, float* output, int N) {
    for (int i=0; i<N; i++) {
	float x = values[i];
	if (x < 0) {
	    output[i] = -x;
	} else {
	    output[i] = x;
	}
    }
}

// implementation of absolute value using 15418 instrinsics
void absVector(float* values, float* output, int N) {
    __cmu418_vec_float x;
    __cmu418_vec_float result;
    __cmu418_vec_float zero = _cmu418_vset_float(0.f);
    __cmu418_mask maskAll, maskIsNegative, maskIsNotNegative;

    //  Note: Take a careful look at this loop indexing.  This example
    //  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
    //  Why is that the case?
    for (int i=0; i<N; i+=VECTOR_WIDTH) {

	// All ones
	maskAll = _cmu418_init_ones();

	// All zeros
	maskIsNegative = _cmu418_init_ones(0);

	// Load vector of values from contiguous memory addresses
	_cmu418_vload_float(x, values+i, maskAll);               // x = values[i];

	// Set mask according to predicate
	_cmu418_vlt_float(maskIsNegative, x, zero, maskAll);     // if (x < 0) {

	// Execute instruction using mask ("if" clause)
	_cmu418_vsub_float(result, zero, x, maskIsNegative);      //   output[i] = -x;

	// Inverse maskIsNegative to generate "else" mask
	maskIsNotNegative = _cmu418_mask_not(maskIsNegative);     // } else {

	// Execute instruction ("else" clause)
	_cmu418_vload_float(result, values+i, maskIsNotNegative); //   output[i] = x; }

	// Write results back to memory
	_cmu418_vstore_float(output+i, result, maskAll);
    }
}

// Accepts an array of values and an array of exponents
// For each element, compute values[i]^exponents[i] and clamp value to
// 4.18.  Store result in outputs.
// Uses iterative squaring, so that total iterations is proportional
// to the log_2 of the exponent
void clampedExpSerial(float* values, int* exponents, float* output, int N) {
    for (int i=0; i<N; i++) {
	float x = values[i];
	float result = 1.f;
	int y = exponents[i];
	float xpower = x;
	while (y > 0) {
	    if (y & 0x1) {
			result *= xpower;
			//printf("result = %f\n", result);
		}
	    xpower = xpower * xpower;
		//printf("xpower = %f\n", xpower);
	    y >>= 1;
		//printf("y = %d\n", y);	
	}
	if (result > 4.18f) {
	    result = 4.18f;
	}
	output[i] = result;
    }
}

void clampedExpVector(float* values, int* exponents, float* output, int N) {
	// Accepts an array of values and an array of exponents
	// For each element, compute values[i]^exponents[i] and clamp value to
	// 4.18.  Store result in outputs.
	// Uses iterative squaring, so that total iterations is proportional
	// to the log_2 of the exponent
    // Implement your vectorized version of clampedExpSerial here
    //  ...
	int i;
	__cmu418_vec_float x, result, xpower;
	__cmu418_mask maskAll, maskIsNegative, maskIsNotNegative,ymask;
	__cmu418_vec_int  y,zero,one;
	__cmu418_vec_float four_point_one = _cmu418_vset_float(4.18f);

    maskAll = _cmu418_init_ones(VECTOR_WIDTH);
	maskIsNegative = _cmu418_init_ones(0);
	zero  = _cmu418_vset_int(0);
	one = _cmu418_vset_int(1);

	for (i=0; i<N; i+=VECTOR_WIDTH) {
		_cmu418_vload_float(x, values+i, maskAll);
		_cmu418_vload_int(y, exponents+i, maskAll);
		result = _cmu418_vset_float(1.f);
		_cmu418_vmove_float(xpower, x, maskAll);

		_cmu418_vlt_int(ymask, zero,y , maskAll);  //ymask = (y > 0);

		while(_cmu418_cntbits(ymask) > 0){        // yseries :while (y > 0) {
			__cmu418_vec_int y_is_odd = _cmu418_vset_int(0);
			_cmu418_vbitand_int(y_is_odd, y, one, maskAll);
			__cmu418_mask mask_y_is_odd = _cmu418_init_ones(0);
			_cmu418_vgt_int(mask_y_is_odd, y_is_odd,zero, maskAll);    // if (y & 0x1) {
			_cmu418_vmult_float(result, result, xpower, mask_y_is_odd);
            //printf("result = %f\n", result.value[0]);
			_cmu418_vmult_float(xpower,xpower,xpower, maskAll);
			//printf("xpower = %f\n", xpower.value[0]);
			_cmu418_vshiftright_int(y, y, one, ymask);

			_cmu418_vlt_int(ymask, zero,y , maskAll);  //refresh ymask
	    
		    // int y0 = y.value[0];
			//printf("y0 = %d\n", y0);
		}

        __cmu418_mask mask_result_is_greater_than_four_point_one = _cmu418_init_ones(0);
		_cmu418_vlt_float(mask_result_is_greater_than_four_point_one,four_point_one, result, maskAll);
		_cmu418_vmove_float(result, four_point_one, mask_result_is_greater_than_four_point_one);
	    
		_cmu418_vstore_float(output+i, result, maskAll);
	}

	//remaining elements
	absSerial(values+i, output+i, N-i);

	//如果可以调整output数组的大小，则可以将最后的结果直接写入output数组，而不用再次处理剩余的元素。
	//然后只要store特定数目的元素即可？
}


float arraySumSerial(float* values, int N) {
    float sum = 0;
    for (int i=0; i<N; i++) {
	sum += values[i];
    }

    return sum;
}

// Assume N % VECTOR_WIDTH == 0
// Assume VECTOR_WIDTH is a power of 2
float arraySumVector(float* values, int N) {
    // Implement your vectorized version here
    //  ...
	return 0.f;
}
