#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include "structures.h"

#define DEBUG

#define FILENAME "modfem_sell_476912.txt"
//#define FILENAME "/arch/data_Malwina_Ciesla/modfem_sell_476912.txt"
//#define FILENAME  "small_sell.txt"

int write_SELL(struct Sell Sell);

struct Sell  read_SELL();

int check_SELL(struct Sell sell_1, struct Sell sell_2 );

int write_SELL(struct Sell Sell){
  
  int i,j;
  
  FILE* file=NULL;
  file = fopen(FILENAME, "w");
  if (file == NULL) {
    printf("Nie mozna otworzyc pliku %s\n", FILENAME);
    exit(0);
  }  
  else{
    printf("po fopen %s\n", FILENAME);
  }    

  fprintf(file, "%d\n",Sell.nr_slice);
  printf("po nr_slice\n");

 
  int l_s = Sell.nr_slice;

  for(i = 0; i < l_s; i++) {
    fprintf(file, "%d\n",  Sell.cl[i]);
  }
  printf("po cl\n");

  
  for(i=0; i<(l_s+1); i++){
    fprintf(file, "%d\n",Sell.sliceStart[i]);
  }
  printf("po slicestart\n");
  
  int nr_entries = 0;
  for(i = 0; i < l_s; i++) {
    //printf("  nr_entries %d\n", nr_entries);
    nr_entries += C * Sell.cl[i];
  }
  printf("po nr_entries %d\n", nr_entries);

  for(i=0; i<nr_entries; i++){
    fprintf(file, "%d\n",Sell.slice_col[i]);
  }
  printf("po slice col\n");

  for(i=0; i<nr_entries; i++){
    fprintf(file, "%le\n",Sell.val[i]);
  }
  printf("po val\n"); 

  printf("po write_sell \n");
  fclose(file);
  return 0;
}

  
struct Sell  read_SELL(){

  int i,j;
  struct Sell Sell;
  FILE* file=NULL;
  file = fopen(FILENAME, "r");
  if (file == NULL) {
    printf("Nie mozna otworzyc pliku %s\n", FILENAME);
    exit(0);
  }
  else{
    printf("po fopen %s\n", FILENAME);
  }    
  
  fscanf(file, "%d\n", &Sell.nr_slice);
  printf("po nr_slice %d\n", Sell.nr_slice);

  int l_s = Sell.nr_slice;
  Sell.cl = (int*)malloc(l_s * sizeof(int)); // pomocnicza tablica długości plastrów

  for(i = 0; i < l_s; i++) {
    fscanf(file, "%d\n",  &Sell.cl[i]);
  }
   printf("po cl\n");
 
  Sell.sliceStart = (int*)malloc((l_s + 1) * sizeof(int));
  for(i = 0; i <= l_s; i++) {
    fscanf(file, "%d\n",  &Sell.sliceStart[i]);
  }
  printf("po slicestart\n");

  int nr_entries = 0;
  for(i = 0; i < l_s; i++) {
    nr_entries += C * Sell.cl[i];
  }
 printf("po nr_entries %d\n", nr_entries);

  Sell.val = (double*)malloc(nr_entries * sizeof(double));
  Sell.slice_col = (int*)malloc(nr_entries * sizeof(int));
  
  for(i=0; i<nr_entries; i++){
    fscanf(file, "%d\n",&Sell.slice_col[i]);
  }
  printf("po slice col\n");

  for(i=0; i<nr_entries; i++){
    fscanf(file, "%le\n",&Sell.val[i]);
  }
  printf("po val\n"); 
  
  printf("po read_sell \n");
  fclose(file);
  return Sell;
}


int check_SELL(struct Sell Sell, struct Sell Sell_2 ){

  int i;
  
  if(Sell.nr_slice!=Sell_2.nr_slice){
    printf("po nr_slice %d = %d\n", Sell.nr_slice, Sell_2.nr_slice );
    exit(0);
  }
  printf("po nr_slice %d = %d\n", Sell.nr_slice, Sell_2.nr_slice );
  
  int l_s = Sell.nr_slice;
  
  for(i = 0; i < l_s; i++) {
    if(Sell.cl[i] != Sell_2.cl[i]){
      printf("po cl\n");
      exit(0);
    }
  }

  for(i = 0; i <= l_s; i++) {
    if( Sell.sliceStart[i] != Sell_2.sliceStart[i]   ){
      printf("po slicestart\n");
      exit(0);   
    }
  }
  printf("po slicestart\n");

  int nr_entries = 0;
  for(i = 0; i < l_s; i++) {
    nr_entries += C * Sell.cl[i];
  }
  printf("po nr_entries %d\n", nr_entries);
  
  for(i=0; i<nr_entries; i++){
    if( Sell.slice_col[i] != Sell_2.slice_col[i]  ){
      printf("po slice col\n");
      exit(0);
    }
  }
  printf("po slice col\n");

  for(i=0; i<nr_entries; i++){
    if( fabs(  (Sell.val[i] - Sell_2.val[i]) / Sell.val[i] )> 1.e-6 ){
      printf("po val\n"); 
      exit(0);
    }
  }
  printf("po val\n"); 
  
  printf("po check_sell \n");
  return(0);
}
