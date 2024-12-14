#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>
#include <emmintrin.h>
#include "structures.h"
#include <mpi.h>

int mat_vec_vector(double *x,double* y,struct Sell sell,int nn,int nt){
  int c;
  for (c=0; c < sell.nr_slice; c ++){ //pętla po plastrach
    int j , offs ;
    __m256d tmp , val , rhs ;
    __m128d rhstmp ;
    tmp = _mm256_load_pd(&y[c<<2]); //wpakuj 4 wartości LHS 
    offs = sell.sliceStart[c]; // poczatkowy offset jest początkiem plastra
    for (j =0; j < sell.cl[c]; j ++){ //pętla w plastrze po wartościach do długści plastra
      val = _mm256_load_pd(&sell.val[offs]); //załaduj 4 wartości macierzy rzadkiej
      rhstmp = _mm_loadl_pd(rhstmp ,&x[sell.slice_col[offs ++]]); //załaduj pierwszą wartość RHS (wektor)
      rhstmp = _mm_loadh_pd(rhstmp ,&x[sell.slice_col[offs ++]]); //załaduj drugą wartość RHS (wektor)
      rhs = _mm256_insertf128_pd(rhs , rhstmp ,0); //wstaw spakowane wartości 
      rhstmp = _mm_loadl_pd(rhstmp ,&x[sell.slice_col[offs ++]]); //załaduj trzecią wartość RHS (wektor)
      rhstmp = _mm_loadh_pd(rhstmp ,&x[sell.slice_col[offs ++]]); //załaduj czwartą wartość RHS (wektor)
      rhs = _mm256_insertf128_pd(rhs , rhstmp ,1); //wstaw spakowane wartości
      tmp = _mm256_add_pd(tmp , _mm256_mul_pd(val , rhs )); //gromadzenie wartości
    }
    _mm256_store_pd(&y[c<<2] , tmp ); //przechowaj 4 wartości LHS (y)
  }
}
