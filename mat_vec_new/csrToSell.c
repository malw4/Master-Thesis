#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include "structures.h"

int max(int* m);

struct Sell csrToSell(struct Sell Sell, struct CSR CSR, int WYMIAR, int NONZ) {

	// liczba wierszy WYMIAR
	int l_r = WYMIAR;
	// liczba plastrów (wysokość plastra C)
	int l_s = ceil((float)l_r / C);
printf("poczatek\n");
	Sell.nr_slice = l_s;
	Sell.cl = malloc(l_s * sizeof(int)); // pomocnicza tablica długości plastrów
	int i, j;
	// obliczanie długości wiersza w bloku wierszy i zapis w pomocniczej tablicy cl
	for(i = 0; i < l_s; i++) {
		Sell.cl[i] = 0;
		for(j = 0; j < C && i * C + j < l_r; j++) {
			int i_temp = CSR.row_ptr[i * C + j + 1] - CSR.row_ptr[i * C + j];
			//printf("slice %d, row in slice %d, row global %d -> row length %d, slice length %d\n", i, j, i * C + j, i_temp, Sell.cl[i]);
			if (i_temp > Sell.cl[i]) Sell.cl[i] = i_temp;
			//         cl[i]=(sell.sliceStart[i+1]-sell.sliceStart[i])/C;
		}
	}

	int nr_entries = 0;
	for(i = 0; i < l_s; i++) {
		nr_entries += C * Sell.cl[i];
	}
	Sell.val = (double*)malloc(nr_entries * sizeof(double));
	Sell.sliceStart = (int*)malloc((l_s + 1) * sizeof(int));
	Sell.slice_col = (int*)malloc(nr_entries * sizeof(int));

	for(i = 0; i < l_s + 1; i++)
		Sell.sliceStart[i] = 0;
	for(j = 0; j < nr_entries; j++)
		Sell.val[j] = 0.0;
	for(j = 0; j < nr_entries; j++)
		Sell.slice_col[i] = 0;
printf("przed inicjalizacje\n");
	int wymiar = l_s;
	//Sell.val = (double*)malloc(WYMIAR * WYMIAR * sizeof(double));//główna tablica wartości dla slice_C
	//Sell.slice_col = (int*)malloc(ROZMIAR * sizeof(int)); //główna tablica kolumn dla slice_c
	//Sell.sliceStart=(int*)malloc(wymiar * sizeof(int));
		//double p[C][WYMIAR]; //tablica pomocniczych rzędów i tablica slice start
	double **p = (double**)malloc(C * sizeof(double*));

double **col=(double**)malloc(C * sizeof(double*)); //[C][WYMIAR], m[C]; //tablica pomocniczych kolumn i tablica zmiennych określających liczbę elementów w tablicy p
	int pomptr[2];
	int m[C];
	int A = 0, B = 0, D, m1, zmM, pomM, slice_start = 0, sliceVal = 0, ilosc;
	pomptr[0] = 0;
	for (int i=0;i<C;i++){
   p[i]=(double*)malloc(WYMIAR*sizeof(double));
   col[i]=(double*)malloc(WYMIAR*sizeof(double));
}
	//inicjowanie tablic
	for(i = 0; i < C; i++) {
		m[i] = 0;
	}
	for(i = 0; i < C; i++) {
		for(j = 0; j < WYMIAR; j++) {
			p[i][j] = 0.0;
		}
	}
	for(i = 0; i < C; i++) {
		for(j = 0; j < WYMIAR; j++) {
			col[i][j] = 0.0;
		}
	}
	//for (i = 0; i < NONZ; i++)
	//for (i = 0; i < wymiar; i++)
	//	Sell.sliceStart[i]=0;
	//for (j = 0; j < WYMIAR*WYMIAR; j++)
	//	Sell.val[j]=0.0;
	//for (j = 0; j < ROZMIAR; j++)
	//	Sell.slice_col[i]=0;


	for(ilosc = 0; ilosc < NONZ; ilosc++) {
		
	  //printf("nonz %d\n", ilosc);
		int j = CSR.col_ind[ilosc];
		int i = CSR.row_ind[ilosc];

		if(i == 0) { //pierwszy przypadek gdy rząd 0
			zmM = m[0]; //zmienna pomocnicza jest brana z tablicy 
			p[0][zmM] = CSR.a_csr[ilosc]; //pomocniczy zerowy rząd wyełniany wartościami
			col[0][zmM] = CSR.col_ind[ilosc];
			m[0]++;
		}
		for (pomM = 1; pomM < C; pomM++) { //dla każdego kolejnego rzędu 
			if ((i % C) == pomM) {//modulo C 
				zmM = m[pomM];   				//wpisywane wartości
				p[pomM][zmM] = CSR.a_csr[ilosc];//do pomocniczej tablicy rzędów
				col[pomM][zmM] = CSR.col_ind[ilosc];
				m[pomM]++;
				A = 0;										//wyzerowanie zmiennych A i B potrzebnych 
				B = 0;										//do wpisywania do końcowej tablicy val i slice_col
			}
		}
		if (i != 0 && (i % C) == 0) {
				if(A==0){
					sliceVal++; //zapisać do tablic val i slice_col
					m1 = max(m); //wyliczanie maksymalnej długości rzędu implementacja funkcji na dole
					for (D = 0; D < m1; D++) { //wyliczanie val i colind dla sell-c
						for (pomM = 0; pomM < C; pomM++) { //dla wszystkich C rzędów
							Sell.val[slice_start + A] = p[pomM][B]; //wpisywane wartości do val
							Sell.slice_col[slice_start + A] = col[pomM][B]; //wpisywane wartości do slice_col
							A++; //po kazdej iteracji zwiększamy A dla następnego rzędu
						}
						B++; //po kazdej iteracji zwiększamy B dla kolejnego elementu 
					}	     //w pomocniczej tablicy rzędów
					slice_start += A;  //dodanie wartości do sliceStart
					Sell.sliceStart[sliceVal] = slice_start;
					for (i = 0; i < C; i++) {
						m[i] = 0;
						for (j = 0; j < WYMIAR; j++) {
							p[i][j] = 0.0;
							col[i][j] = 0;
						}
					}
				}
			zmM = m[0]; //wpisanie wartości do zerowego pomocniczego rzędu 
			p[0][zmM] = CSR.a_csr[ilosc];
			col[0][zmM] = CSR.col_ind[ilosc];
			m[0]++;
		}
	}


	//ostatni slice C
	A = 0;										//wyzerowanie zmiennych A i B potrzebnych 
	B = 0;										//do wpisywania do końcowej tablicy val i slice_col
	sliceVal++;
	m1 = max(m);
	for (D = 0; D < m1; D++) { //wyliczanie val i colind dla sell-c
		for (pomM = 0; pomM < C; pomM++) {
			Sell.val[slice_start + A] = p[pomM][B];
			Sell.slice_col[slice_start + A] = col[pomM][B];
			//printf("new entry %lf, col_ind %d, col[%d,%d] = %d\n", Sell.val[slice_start + A], Sell.slice_col[slice_start + A], pomM, B, col[pomM][B]);

						//printf("ostatni Asslice col=%d\n",Sell.slice_col[slice_start + A]);
			A++;
		}
		B++;
	}


				//printf("wyszedlem z petli\n");
	slice_start += A;  //dodanie wartości do sliceStart
	Sell.sliceStart[sliceVal] = slice_start;
	//wypisanie val, colind i slice_start dla sell-c
	/*
	printf("\n");
	printf("Val=[");
	for (i = 0; i < nr_entries; i++)
		printf(" %.1lf, ", Sell.val[i]);
	printf("]\n");
	printf("\n");
	printf("colind=[");
	for (i = 0; i < nr_entries; i++)
		printf(" %d, ", Sell.slice_col[i]);
	printf("]\n");
	printf("\n");
	printf("sliceStart=[");
	for (i = 0; i < wymiar + 1; i++)
		printf(" %d, ", Sell.sliceStart[i]);
	printf("]\n");
	printf("\n");
	//printf("wyw=%f\n",wyw);*/

/*
FILE* f1=NULL;
  f1 = fopen("fileSellmniejszy.txt", "w");
  if (f1 == NULL) {
    printf("Nie mozna otworzyc pliku.\n");
    exit(1);
  } else {
Sell.nr_entries=nr_entries;
Sell.l_s=l_s;
    fprintf(f1, "%d\n",Sell.nr_slice);
	printf("po nr_slice\n");
    for(i=0; i<(l_s+1); i++){
    fprintf(f1, "%d\n",Sell.sliceStart[i]);
    }
	printf("po slicestart\n");
    for(i=0;i<nr_entries;i++){
     fprintf(f1, "%f\n",Sell.val[i]);
    }
	printf("po val\n");
	for(i=0;i<nr_entries;i++){
     fprintf(f1, "%d\n",Sell.slice_col[i]);
    }
printf("po slice col\n");
	for(i=0;i<l_s;i++){
     fprintf(f1, "%d\n",Sell.cl[i]);
	}
    fclose(f1);
  }
*/
printf("koniec sell\n");
	return Sell;
}

int max(int* m) {
	int i, max = 0;
	for (i = 0; i < C; i++) {
		if (m[i] > max)
			max = m[i];
	}
	return max;
}
