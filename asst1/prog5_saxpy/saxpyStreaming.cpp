#include <smmintrin.h> // For _mm_stream_load_si128
#include <emmintrin.h> // For _mm_mul_ps
#include <assert.h>
#include <stdint.h>

extern void saxpySerial(int N,
			float scale,
			float X[],
			float Y[],
			float result[]);


extern void saxpyStreaming(int N,
                    float scale,
                    float X[],
                    float Y[],
                    float result[])
{
    // Replace this code with ones that make use of the streaming instructions
    int i;

    for (i=0;i<N;i+=4){
        __m128 x_vec = _mm_load_ps(&X[i]);
        __m128 y_vec = _mm_load_ps(&Y[i]);
        __m128 result_vec = _mm_mul_ps(x_vec, y_vec);
        result_vec = _mm_add_ps(result_vec, _mm_set1_ps(scale));
        _mm_stream_ps(&result[i], result_vec);
    }

    for(;i<N;i++){
        result[i] = X[i] * Y[i] + scale;
    }

    _mm_sfence(); // Ensure all stores are completed before returning
}

