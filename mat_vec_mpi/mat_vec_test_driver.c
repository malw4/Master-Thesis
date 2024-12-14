#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include "uth_time.h"
#include "structures.h"

#include <mpi.h>

//#define DEBUG

#ifdef PAPI_TEST
#include "papi_driver.h"
//printf("PAPI_TEST_OK");
#endif

void mat_vec_crs(double* x, double* y, struct CSR csr,  int n,  int nt);
void mat_vec_sell(double* x, double* y, struct Sell sell, int nn,int nt);
void mat_vec_vector(double *x,double* y,struct Sell sell,int nn,int nt);
struct Sell csrToSell(struct Sell sell,struct CSR CSR, int wymiar, int nonz);
struct CSR read_modfem_crs_matrix();
struct Sell read_modfem_sell_matrix();

int main(int argc, char** argv) {	
  struct CSR csr;
  struct Sell sell;
  double t1;
  int pom, x0;
  int n, i, j, k;
  long int ilosc = 0, row_sum = 1;
  const long int ione = 1;
  const double done = 1.0;
  const double dzero = 0.0;
  long int rozmiar;
  int wymiar;
  int nonz;
  int rank, size, source, dest, tag=0; 
  int n_wier, n_wier_last;
  int val, val_last, cl_max=0;
  int iproc, islice;
  double* x;
  double* x_extended;
  double* y;
  double* y_csr;
  double* y_sell;
  double* y_vec;
  MPI_Status status;
  
  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &rank ); 
  MPI_Comm_size( MPI_COMM_WORLD, &size ); 
  
  if(rank==0){  
    csr = read_modfem_crs_matrix(&wymiar, &nonz); 
    //sell =  read_modfem_sell_matrix(&wymiar, &nonz, csr);
    sell=csrToSell(sell,csr,wymiar, nonz); 
    
    rozmiar=wymiar*wymiar;
    printf("\nafter reading ModFEM crs matrix: wymiar %d, nnz %d\n", wymiar, nonz);
    x = (double*)malloc((wymiar + 1) * sizeof(double));
    for (i = 0; i < wymiar; i++) x[i] = 0.1 * (wymiar - i);
    y_csr = (double*)malloc((wymiar + 1) * sizeof(double));
    for (i = 0; i < wymiar; i++) y_csr[i] = 0.0;
    y_sell = (double*)malloc((wymiar + 1) * sizeof(double));
    for (i = 0; i < wymiar; i++) y_sell[i] = 0.0;
    y_vec = (double*)aligned_alloc(64,(wymiar + 1) * sizeof(double));
    for (i = 0; i < wymiar; i++) y_vec[i] = 0.0;
    
    // razem z row_ind
    double tmp = 2.0;
    double rozmiar_danych = (nonz*(tmp*sizeof(int)+sizeof(double))+wymiar*sizeof(int))/1024/1024;
    
    /************************************* CSR *********************************/
    printf("\nPoczatek procedury macierz-wektor CRS: rozmiar danych %lf MB and %d rank, %d size\n",
	   rozmiar_danych,rank, size);
    
    int fake = 1;
    n = wymiar;
    
    printf("poczatek (wykonanie sekwencyjne)\n");
    
#ifdef DEBUG
    mat_vec_crs( x, y_csr, csr, n, fake);
    mat_vec_sell( x, y_sell,sell, n, fake);
    
    for(int i=0; i<wymiar; i++){
      if(fabs(y_csr[i]-y_sell[i]) > 1.e-9*y_csr[i]) {
	printf("Error! i=%d, y_csr[i]=%lf, y_sell[i]=%lf\n",i, y_csr[i], y_sell[i]);
      } 
      else{
	//printf("OK! i=%d, y_csr[i]=%lf, y_sell[i]=%lf\n",i, y_csr[i], y_sell[i]);
      }
    }        
#endif
    
    t1 = MPI_Wtime();
    mat_vec_vector( x, y_vec,sell,n, fake); 
    t1 = MPI_Wtime() - t1;
      
    printf("\tczas wykonania (zaburzony przez MPI?): %lf, Gflop/s: %lf, GB/s> %lf\n",  
	   t1, 2.0e-9*nonz/t1, (1.0+1.0/n)*8.0e-9*nonz/t1);
    
  }
  
  if(size>0){
    int nr_slice_local;
    if(rank==0){
      cl_max=0;
      for (i = 0; i < sell.nr_slice; i++) {
        if (sell.cl[i] > cl_max)
          cl_max = sell.cl[i];
      }
      nr_slice_local = ceil((float)sell.nr_slice / size);
      
      y = (double*)malloc(nr_slice_local*size*C*sizeof(double));
      for(i=0;i<nr_slice_local*size*C;i++) y[i]=0.0;
      
    } 
    /************** || block row decomposition || *******************/
    
    // broadcast parametry integer sell - dla macierzy lokalnych
    MPI_Bcast(&nr_slice_local, 1, MPI_INT, 0, MPI_COMM_WORLD );
    int nr_slice_new = size*nr_slice_local;
    
    MPI_Bcast(&cl_max, 1, MPI_INT, 0, MPI_COMM_WORLD );
    printf("\nafter bcast: rank %d, size %d, nr_slice_local %d, cl_max %d\n",
	   rank, size, nr_slice_local, cl_max );
    
    // podzial wierszowy
    
    int local_elements = nr_slice_local * cl_max * C;
    double* x_new = (double*)malloc(nr_slice_new*C*sizeof(double));
    for (i = 0; i < nr_slice_new*C; i++){
      x_new[i] = 0.0;
    }
    
    // ??? MPI_Scatter(y, nr_slice_local*C, MPI_DOUBLE, y_local, nr_slice_local*C, MPI_DOUBLE, 0, MPI_COMM_WORLD );
    double *y_local = (double *) malloc(nr_slice_local*C*sizeof(double)); 
    for (i = 0; i < nr_slice_local*C; i++){
      y_local[i] = 0.0;
    }
    
    int* cl_local= (int *) malloc(nr_slice_local*sizeof(int));
    int* sliceStart_local= (int *) malloc((nr_slice_local+1)*sizeof(int));
    for (i = 0; i < nr_slice_local; i++){
      cl_local[i] = cl_max; // !!!
      sliceStart_local[i] = cl_max*C*i;  // !!!
    }
    sliceStart_local[nr_slice_local] = local_elements;

    double *val_local = (double *) malloc(local_elements*sizeof(double)); 
    int *slice_col_local = (int*) malloc(local_elements*sizeof(int)); 
    for(i=0;i<local_elements;i++){
      val_local[i]=0.0;
      slice_col_local[i]=0;
    }


#ifdef DEBUG
    printf("\nafter inicjalizacja i before scatter: rank %d, size %d\n",rank, size);
    for(i=0;i<nr_slice_local;i++){
      printf("cl_local %d, slicestart_local %d-%d\n",cl_local[i],sliceStart_local[i],sliceStart_local[i+1]);
    }
#endif
    
    
    if(rank==0){
      for(i=0; i<wymiar; i++) x_new[i] = x[i];
    }
    MPI_Bcast(x_new, nr_slice_new*C, MPI_DOUBLE, 0, MPI_COMM_WORLD );
    
    
#ifdef DEBUG
    printf("\nafter Bcast x\n");
    for(i=0;i<nr_slice_new*C;i++){
      printf("rank %d: x[%2d] = %lf\n",rank, i, x_new[i]);
    }
#endif

    double* val_send = malloc(size * local_elements * sizeof(double));
    int* slice_col_send = malloc(size * local_elements * sizeof(int));

    if (rank == 0) {
        for (iproc = 0; iproc < size; iproc++) {
            for (islice = 0; islice < nr_slice_local; islice++) {
                int slice_glob = iproc * nr_slice_local + islice;
                
                for (int j = 0; j < sell.cl[slice_glob] * C; j++) {
                    int valuestart = sell.sliceStart[slice_glob] + j;
                    val_send[valuestart] = sell.val[valuestart];
                    slice_col_send[valuestart] = sell.slice_col[valuestart];
                }
            }
        }
    }

    MPI_Scatter(val_send, local_elements, MPI_DOUBLE, val_local, local_elements, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(slice_col_send, local_elements, MPI_INT, slice_col_local, local_elements, MPI_INT, 0, MPI_COMM_WORLD);
    
    if(rank==0) {
      printf("Starting MPI matrix-vector product with block row decomposition!\n");
      t1 = MPI_Wtime();
    }
    printf("allgather przed\n");
    
    MPI_Allgather(&x_new[rank* nr_slice_local*C], nr_slice_local*C, MPI_DOUBLE, x_new, nr_slice_local*C, MPI_DOUBLE, MPI_COMM_WORLD );
    
#ifdef DEBUG
    printf("\nafter Allgather x\n");
    for(i=0;i<nr_slice_new*C;i++){
      printf("rank %d: x[%2d] = %lf\n",rank, i, x_new[i]);
    }
#endif


    for (i = 0; i < nr_slice_local; i++) {
      // p�tla po kolumnach w i-tym bloku wierszy
      for (j = 0; j < cl_max; j++) {
        for(k=0;k<C;k++){
          pom = sliceStart_local[i] + j * C + k;
          x0 = slice_col_local[pom];
          y_local[i * C + k] += val_local[pom] * x_new[x0];
	  
        }
      }
   }  
    
#ifdef DEBUG
    for (i = 0; i < nr_slice_local*C; i++){
      printf("final: rank %d: y_local[%d] = %lf\n", rank, i,  y_local[i]);
    }
#endif

    if(rank==0) {
      t1 = MPI_Wtime() - t1;
      printf("Werja rownolegla MPI z dekompozycją wierszową blokową\n");
      printf("\tczas wykonania: %lf, Gflop/s: %lf, GB/s> %lf\n",  
	     t1, 2.0e-9 * nonz / t1, (1.0 + 1.0 / n) * 8.0e-9 * nonz / t1);
      
    }
    // just to measure time
    MPI_Barrier(MPI_COMM_WORLD);   
    printf("\nafter barier and %d rank, %d size\n",rank, size);     
    
    
    MPI_Gather( y_local, nr_slice_local*C, MPI_DOUBLE, y, nr_slice_local*C, MPI_DOUBLE, 0, MPI_COMM_WORLD ); 
    /*if(rank==0){
      
      for(i=0;i<wymiar;i++){
        if(fabs(y_vec[i]-y[i])>1.e-6*y_vec[i]) {
          printf("Blad! i=%d, y[i]=%lf, z[i]=%lf\n",i, y_vec[i], y[i]);
        }
      }        
    }*/
  }
  
  
  MPI_Finalize(); 
  
}