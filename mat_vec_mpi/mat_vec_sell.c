#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include "structures.h"
#include <mpi.h>

void mat_vec_sell(double* x, double* y_global, struct Sell sell,int nn, int nt) {
  register long int i;
    register long int n = nn;
    register long int j;
    register long int pom;
    register long int x0;
    int k;
    int wyn = sell.nr_slice;
    
    // p�tla po blokach wierszy
    for (i = 0; i < wyn; i++) {
      
      // p�tla po kolumnach w i-tym bloku wierszy
      for (j = 0; j < sell.cl[i]; j++) {
	for(k=0;k<C;k++){
	  pom = sell.sliceStart[i] + j * C + k;
	  x0 = sell.slice_col[pom];
	  y_global[i * C + k] += sell.val[pom] * x[x0];
	
      }
    }
  }
  //printf("y[%ld] = %lf \n",16612,y[16612]);
  //printf("y[%ld] = %lf \n",16613,y[16613]);
}
