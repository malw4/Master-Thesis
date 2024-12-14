#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "structures.h"


void mat_vec_crs(double* x, double* y,struct CSR csr,int nn,int nt){
  register long int i;
  //int j;
    register long int n=nn;
    register long int j;
    register long int x0;
    for(i=0;i<n;i++){
      long int edge=csr.row_ptr[i+1]-1; 	
      for(j=csr.row_ptr[i];j<=edge;j++){
        x0=csr.col_ind[j];
        y[i]+=csr.a_csr[j] * x[x0];
      }
    }
}

