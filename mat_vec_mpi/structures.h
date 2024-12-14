#define C 4

struct CSR{
  double* a_csr;	//g��wna tablica warto�ci
  int* row_ptr;		//tablica row_ptr
  int* col_ind;	 //tablica kolumn
  int* row_ind; //tablica potrzebna do ustawienia Sell-C
};

struct Sell{
  int nr_slice; 		//wysoko�� plastra
  double* val; 			//g��wna tablica warto�ci dla slice_C
  int* slice_col;  //tablica kolumn
  int* sliceStart; //tablica slice_start
  int* cl; 				//pomocnicza tablica d�ugo�ci plastr�w
  int l_s;
  int nr_entries;
};