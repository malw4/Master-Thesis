#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include "structures.h"

//#define DEBUG

//#define FILENAME "small.txt"
//#define FILENAME "modfem_crs_1210.txt"
#define FILENAME "modfem_crs_476912.txt"
//#define FILENAME "/home/dyplomy/malwina_ciesla/mgr/data/modfem_crs_1210.txt"
//#define FILENAME "/arch/data_Malwina_Ciesla/modfem_crs_476912.txt"

struct CSR read_modfem_crs_matrix(int *n, int *nnz){
  
  int i,j;
  struct CSR CSR;
  //struct Sell Sell;
  
  int wymiar;
  int nonz;
  
  FILE* file_in=NULL;
  file_in = fopen(FILENAME, "r");
  if (file_in == NULL) {
    printf("Nie mozna otworzyc pliku %s\n", FILENAME);
    exit(0);
  }  
  
  fscanf(file_in, "%d\n", &wymiar);
  fscanf(file_in, "%d\n", &nonz);
  
  
  CSR.a_csr = (double*)malloc(nonz * sizeof(double));
  CSR.row_ptr = (int*)malloc((wymiar + 1) * sizeof(int));
  CSR.col_ind = (int*)malloc(nonz * sizeof(int));
  CSR.row_ind = (int*)malloc(nonz * sizeof(int));
  
  for(i=0; i<wymiar+1; i++){
    CSR.row_ptr[i]=0;
  }
  for(i=0;i<nonz;i++){
    CSR.col_ind[i]=0;
    CSR.row_ind[i]=0;
    CSR.a_csr[i]=0.0;
  }
  
  for(i=0; i<wymiar+1; i++){
    fscanf(file_in, "%d\n",&CSR.row_ptr[i]);
  }
  
  for(i=0; i<nonz; i++){
    fscanf(file_in, "%d\n",&CSR.col_ind[i]);
  }
  /* for(i=0; i<nonz; i++){ */
  /*   fscanf(file_in, "%d\n",&CSR.row_ind[i]); */
  /* } */
  for(i=0;i<nonz;i++){
    
    fscanf(file_in, "%lf\n",&CSR.a_csr[i]);
  }

  
  //printf("%d\n",CSR.col_ind[1]);
  //printf("%lf\n",CSR.a_csr[1]);
  //printf("%d\n",CSR.row_ind[1]);
   j=0;
    int a=0, l,k=0;
    for(i=1; i<wymiar+1; i++){
      k = CSR.row_ptr[i]-CSR.row_ptr[i-1];
      //printf(" %d to k \n",k);
      for(j,l=0;l<k;j++,l++){
        CSR.row_ind[j]=a;
        //printf(" %d to row ind\n",CSR.row_ind[j]);
      }
      a++;
    }
  
  fclose(file_in);
    
#ifdef DEBUG
  
  FILE* file_out = fopen("modfem_crs.txt", "w");
  if (file_out == NULL) {
    //printf("Nie mozna otworzyc pliku.\n");
    exit(-1);
  } else {
    fprintf(file_out, "%d\n",wymiar);
    fprintf(file_out, "%d\n",nonz);  
    for(i=0; i<wymiar+1; i++){
      fprintf(file_out, "%d ",CSR.row_ptr[i]);
    }
    fprintf(file_out,"\n");
    for(i=0;i<nonz;i++){
      fprintf(file_out, "%d ",CSR.col_ind[i]);
    }
    fprintf(file_out,"\n");
    for(i=0;i<nonz;i++){
      fprintf(file_out, "%lf ",CSR.a_csr[i]);
    }
    fprintf(file_out,"\n");
    
    fclose(file_out);
  }
  
  printf("debug reading ModFEM crs matrix: after writing, press twice any key\n");
  getchar(); getchar();
#endif

  printf("after reading ModFEM crs matrix: wymiar =  %d, nonz = %d\n", wymiar, nonz);
  
  *n = wymiar;
  *nnz = nonz;
  return CSR;
}


// read_modfem_sell_matrix moved to file read_modfem_sell_matrix.c as read_SELL()
// struct Sell read_modfem_sell_matrix(int *n, int *nnz, struct CSR CSR)
  
