#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include "uth_time.h"
#include "structures.h"

#include <mpi.h>

#define DEBUG
//#define DEBUG_PRINT_MPI
//#define DEBUG_PRINT_ALL
//#define DEBUG_READ
#define TOLERANCE 1.e-6

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

int write_SELL(struct Sell Sell);
struct Sell  read_SELL();
int check_SELL(struct Sell sell_1, struct Sell sell_2 );


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
    

#ifdef DEBUG_PRINT_ALL
    printf("a original\n");
    int l = 0;
    for(int i=0; i<wymiar; i++){
      for(int j=0; j<wymiar; j++){
	int col = csr.col_ind[l];
	if(col!=j){
	  printf("%6.2lf", 0.0);
	}
	else{
 	  printf("%6.2lf", csr.a_csr[l]);
	  l++;
	}
      }
      printf("\n");
    }
    
    printf("a in CRS\n");
    l = 0;
    for(int i=0; i<wymiar; i++){
      for(int j = csr.row_ptr[i]; j < csr.row_ptr[i+1]; j++){
	printf("%6.2lf (%2d)", csr.a_csr[l],  csr.col_ind[l]);
	l++;
      }
      printf("\n");
    }
    
#endif
    
    
    //sell=csrToSell(sell,csr,wymiar, nonz);
#ifdef DEBUG_READ
    //sell =  read_modfem_sell_matrix(&wymiar, &nonz, csr);
    struct Sell sell_from_csr;
    sell_from_csr=csrToSell(sell,csr,wymiar, nonz);
    int result = write_SELL(sell_from_csr);
    //sell = sell_from_csr;
    sell = read_SELL();
    check_SELL(sell_from_csr, sell );
#else
    sell = read_SELL();
#endif
    if(wymiar != sell.nr_slice*C){
      printf("Error in reading SELL!\n");
    }
    
#ifdef DEBUG_PRINT_ALL
    printf("a in SELL\n");
    for (i = 0; i < sell.nr_slice; i++) {
      for (j = 0; j < sell.cl[i]; j++) {
	k=0;
	//for(k=0;k<C;k++)
	{
	  pom = sell.sliceStart[i] + j * C + k;
	  //x0 = sell.slice_col[pom];
	  //y_global[i * C + k] += sell.val[pom] * x[x0];
	  printf("slice %d, col %d , %lf (%d), %lf (%d), %lf (%d), %lf (%d)\n", 
		 i, j, sell.val[pom], sell.slice_col[pom], 
		 sell.val[pom+1], sell.slice_col[pom+1],
		 sell.val[pom+2], sell.slice_col[pom+2],
		 sell.val[pom+3], sell.slice_col[pom+3]); 
	}
      }
      //getchar();
    }  
#endif
    
    
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
    
    int result1_ok = 1;
    for(int i=0; i<wymiar; i++){
      if(fabs(y_csr[i]-y_sell[i]) > TOLERANCE*fabs(y_csr[i])) {
	printf("Error! i=%d, y_csr[i]=%lf, y_sell[i]=%lf\n",i, y_csr[i], y_sell[i]);
	getchar(); getchar();
 	result1_ok=0;
     } 
      else{
	//printf("OK! i=%d, y_csr[i]=%lf, y_sell[i]=%lf\n",i, y_csr[i], y_sell[i]);
      }
    }        
    if(result1_ok == 1) printf("OK!  y_csr[i] == y_sell\n");
#endif
    
    t1 = MPI_Wtime();
    mat_vec_vector( x, y_vec,sell,n, fake); 
    t1 = MPI_Wtime() - t1;
    
#ifdef DEBUG
    int result2_ok = 1;
    for(int i=0; i<wymiar; i++){
      if(fabs(y_csr[i]-y_vec[i]) > TOLERANCE*fabs(y_csr[i])) {
	printf("Error! i=%d, y_csr[i]=%lf, y_vec[i]=%lf\n",i, y_csr[i], y_vec[i]);
	getchar(); getchar();
	result2_ok=0;
      } 
      else{
	//printf("OK! i=%d, y_csr[i]=%lf, y_vec[i]=%lf\n",i, y_csr[i], y_vec[i]);
      }
    }  
    if(result2_ok == 1) printf("OK!  y_csr[i] == y_vec\n");
#endif
      
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
    
    
    // ??? int* cl_local= (int *) malloc(nr_slice_local*C*sizeof(int));
    // ??? int* sliceStart_local= (int *) malloc(nr_slice_local*C*sizeof(int));
    // ??? MPI_Scatter(sell.cl, nr_slice_local*C, MPI_INT, cl_local, nr_slice_local*C, MPI_INT, 0, MPI_COMM_WORLD );
    // ??? MPI_Scatter(sell.sliceStart, nr_slice_local*C, MPI_INT, sliceStart_local, nr_slice_local*C, MPI_INT, 0, MPI_COMM_WORLD );
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


#ifdef DEBUG_PRINT_ALL
    printf("\nafter inicjalizacja i before scatter: rank %d, size %d\n",rank, size);
    for(i=0;i<nr_slice_local;i++){
      printf("cl_local %d, slicestart_local %d-%d\n",cl_local[i],sliceStart_local[i],sliceStart_local[i+1]);
    }
#endif
    
    
    if(rank==0){
      for(i=0; i<wymiar; i++) x_new[i] = x[i];
    }
    MPI_Bcast(x_new, nr_slice_new*C, MPI_DOUBLE, 0, MPI_COMM_WORLD );
    
    
#ifdef DEBUG_PRINT_ALL
    printf("\nafter Bcast x\n");
    for(i=0;i<nr_slice_new*C;i++){
      printf("rank %d: x[%2d] = %lf\n",rank, i, x_new[i]);
    }
#endif
    

    
    if(rank==0){

        iproc = 0; // rewrite to local structures
        for(islice=0; islice<nr_slice_local; islice++){
          int slice_glob=iproc*nr_slice_local+islice;
          int valuestart=sell.sliceStart[slice_glob];
 	  int nr_cl = sell.cl[slice_glob]; // original number of columns
         //printf("\nfor  send and %d rank, %d size\n",rank, size);
	  int pom = sliceStart_local[islice];;
	  for(int i=0; i<nr_cl*C;i++){
	    val_local[pom+i] = sell.val[valuestart+i];
	    slice_col_local[pom+i] = sell.slice_col[valuestart+i];
	  }
	}
	  

      for(iproc=1; iproc<size; iproc++) {
	printf("\nrank 0, before send to rank %d\n",iproc);
        for(islice=0; islice<nr_slice_local; islice++){
          int slice_glob=iproc*nr_slice_local+islice;
          int valuestart=sell.sliceStart[slice_glob];
          //printf("\nfor  send and %d rank, %d size\n",rank, size);
	  MPI_Send( &sell.cl[slice_glob], 1, MPI_INT,  iproc, tag, MPI_COMM_WORLD );
          MPI_Send( &sell.val[valuestart], sell.cl[slice_glob]*C, MPI_DOUBLE, iproc, tag, MPI_COMM_WORLD );
          //printf("\nsend val udany dla %d rank do %d proces",rank,iproc);
          MPI_Send( &sell.slice_col[valuestart], sell.cl[slice_glob]*C, MPI_INT, iproc, tag, MPI_COMM_WORLD );
          
#ifdef DEBUG_PRINT_MPI
	  printf("rank 0 send to proc %d, islice %d (global %d):\n", iproc, islice, iproc*nr_slice_local+islice);
	  //for(islice=0; islice<nr_slice_local; islice++){
	  for (j = 0; j < sell.cl[slice_glob]; j++) {
	    k=0;
	    //for(k=0;k<C;k++)
	    {
	      pom = sell.sliceStart[slice_glob] + j * C + k;
	      //x0 = sell.slice_col[pom];
	      //y_global[i * C + k] += sell.val[pom] * x[x0];
	      printf("col %d , %lf (%d), %lf (%d), %lf (%d), %lf (%d)\n", 
		     j, sell.val[pom], sell.slice_col[pom], 
		     sell.val[pom+1], sell.slice_col[pom+1],
		     sell.val[pom+2], sell.slice_col[pom+2],
		     sell.val[pom+3], sell.slice_col[pom+3]); 
	    }
	  }
	  //getchar(); getchar();
#endif
        }
        //printf("\nafter send and %d rank, %d size\n",rank,size);
      }
    } 
    else{
      printf("\nrank %d, before recv from rank 0\n", rank);
      //for (i=0;i<nr_slice_local;i++)
      //printf("\n cl_local = %d\n",cl_local[i]);
      //printf("else Rank %d: nr_slice_local = %d, C = %d, nr_slice_new = %d\n", rank, nr_slice_local, C, nr_slice_new);
      for(islice=0; islice<nr_slice_local; islice++) {
	int nr_cl; // number of columns in slice
	int pom = sliceStart_local[islice];;
	MPI_Recv( &nr_cl, 1, MPI_INT,  0, tag, MPI_COMM_WORLD, &status );
	MPI_Recv( &val_local[pom], nr_cl*C, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD, &status );
	MPI_Recv( &slice_col_local[pom], nr_cl*C, MPI_INT, 0, tag, MPI_COMM_WORLD, &status );
	
#ifdef DEBUG_PRINT_MPI
	printf("rank %d received from rank 0, islice %d (global %d):\n", rank, islice, rank*nr_slice_local+islice);
	//  for(islice=0; islice<nr_slice_local; islice++){
	for (j = 0; j < nr_cl; j++) {
	  k=0;
	  //for(k=0;k<C;k++)
	  {
	    pom = sliceStart_local[islice] + j * C + k; // sliceStart_local[islice] = cl_max*C*i;  // !!!
	    //x0 = sell.slice_col[pom];
	    //y_global[i * C + k] += sell.val[pom] * x[x0];
	    printf("slice %d,  col %d , %lf (%d), %lf (%d), %lf (%d), %lf (%d)\n", 
		   islice,  j, val_local[pom], slice_col_local[pom], 
		   val_local[pom+1], slice_col_local[pom+1],
		   val_local[pom+2], slice_col_local[pom+2],
		   val_local[pom+3], slice_col_local[pom+3]); 
	  }
	}
	//getchar(); getchar();
#endif
      }
    } 


#ifdef DEBUG_PRINT_MPI
    if(rank==0){
      for (i = 0; i < nr_slice_local; i++) {
	int islice = i;
        printf("rank %d: full data for islice %d (global %d):\n", rank, i, rank*nr_slice_local+i);
	for (j = 0; j < cl_max; j++) {
	  k=0;
	  //for(k=0;k<C;k++)
	  {
	    pom = sliceStart_local[islice] + j * C + k; // sliceStart_local[islice] = cl_max*C*i;  // !!!
	    x0 = slice_col_local[pom];
	    //y_local[i * C + k] += val_local[pom] * x_new[x0];
	    printf("slice %d,  col %d , %lf (%d), %lf (%d), %lf (%d), %lf (%d)\n", 
		   islice,  j, val_local[pom], slice_col_local[pom], 
		   val_local[pom+1], slice_col_local[pom+1],
		   val_local[pom+2], slice_col_local[pom+2],
		   val_local[pom+3], slice_col_local[pom+3]); 
	  }
	}
	//getchar();
      }
    }
#endif
    

    
    if(rank==0) {
      printf("Starting MPI matrix-vector product with block row decomposition!\n");
      t1 = MPI_Wtime();
    }
    
    //MPI_Allgather(&x_new[rank* nr_slice_local*C], nr_slice_local*C, MPI_DOUBLE, x_new, nr_slice_local*C, MPI_DOUBLE, MPI_COMM_WORLD );
    MPI_Allgather(MPI_IN_PLACE, nr_slice_local*C, MPI_DOUBLE, x_new, nr_slice_local*C, MPI_DOUBLE, MPI_COMM_WORLD );

#ifdef DEBUG_PRINT_ALL
    printf("\nafter Allgather x\n");
    for(i=0;i<nr_slice_new*C;i++){
      printf("rank %d: x[%2d] = %lf\n",rank, i, x_new[i]);
    }
#endif


    for (i = 0; i < nr_slice_local; i++) {
      // p tla po kolumnach w i-tym bloku wierszy
      for (j = 0; j < cl_max; j++) {
        for(k=0;k<C;k++){
          pom = sliceStart_local[i] + j * C + k;
          x0 = slice_col_local[pom];
          //printf("\nbefore - rank %d: y_local[%d]=%f (+ val_local[%d]=%lf * x_new[%d]=%lf) \n", 
	  //  rank,  i * C + k, y_local[i * C + k], pom, val_local[pom], x0, x_new[x0]);
          y_local[i * C + k] += val_local[pom] * x_new[x0];
          //printf("\nafter - rank %d: y_local[%d]=%f (+ val_local[%d]=%lf * x_new[%d]=%lf) \n", 
	  // rank,  i * C + k, y_local[i * C + k], pom, val_local[pom], x0, x_new[x0]);
	  
        }
      }
      //getchar();getchar();
   }  
    
#ifdef DEBUG_PRINT_ALL
    for (i = 0; i < nr_slice_local*C; i++){
      printf("final: rank %d: y_local[%d] = %lf\n", rank, i,  y_local[i]);
    }
#endif

    // just to measure time
    MPI_Barrier(MPI_COMM_WORLD);   
    if(rank==0) {
      t1 = MPI_Wtime() - t1;
      printf("Werja rownolegla MPI z dekompozycj� wierszow� blokow�\n");
      printf("\tczas wykonania: %lf, Gflop/s: %lf, GB/s> %lf\n",  
	     t1, 2.0e-9 * nonz / t1, (1.0 + 1.0 / n) * 8.0e-9 * nonz / t1);
      
    }  
    
    
    MPI_Gather( y_local, nr_slice_local*C, MPI_DOUBLE, y, nr_slice_local*C, MPI_DOUBLE, 0, MPI_COMM_WORLD ); 

    if(rank==0){
      int result_OK=1;
      for(i=0;i<wymiar;i++){
        if(fabs(y_vec[i]-y[i])>TOLERANCE*fabs(y_vec[i])) {
          printf("Blad! i=%d, y[i]=%lf, z[i]=%lf\n",i, y_vec[i], y[i]);
	  getchar();
	  result_OK=0;
        }
	else{
	  //printf("OK -  i=%d, y[i]=%lf, z[i]=%lf\n",i, y_vec[i], y[i]);
	}
      }
      if(result_OK==1){
	printf("Parallel version result same as sequential\n"); 
      
      }
    }
  }
  
  
  MPI_Finalize(); 
  
}