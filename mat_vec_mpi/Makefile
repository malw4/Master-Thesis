# kompilator c
#CCOMP = gcc -Wno-format -Wno-unused-result 
#CCOMP = /usr/lib/x86_64-linux-gnu/openmpi/bin/mpicc
CCOMP = /usr/bin/mpicc -Wno-format -Wno-unused-result 

# konsolidator
LOADER = $(CCOMP)

MPIRUN = /usr/bin/mpiexec -display-map --bind-to core --map-by socket

# PAPI
PAPI_HOME = /home/dyplomy/malwina_ciesla/mgr/papi-6.0.0/src
# PAPI_HOME = /home/krzysztof/Pobrane/papi-6.0.0/src_gcc
#PAPI_HOME = /home/krzysztof/PAPI/papi_6.0.0/src/

# opcje optymalizacji:
# wersja do debugowania
#OPT = -g -DDEBUG -p
# wersja zoptymalizowana do mierzenia czasu
# OPT = -O3 -fopenmp -p
OPT = -O3 -fopenmp -march=core-avx2 -p
#OPT = -g  -fopenmp 
#OPT = -O2 -parallel -opt-mem-bandwidth1

# pliki naglowkowe
#INC = -I../pomiar_czasu
#INC = -I../utd_time_unix -I../utd_papi_driver -I$(PAPI_HOME)
INC = -I../utd_time_unix  -I../utd_papi_driver -I$(PAPI_HOME) -I/usr/lib/x86_64-linux-gnu/openmpi/include

# biblioteki
#LIB = -L../pomiar_czasu -lm
#LIB = -L/opt/intel//mkl/8.1/lib/em64t -lmkl_em64t
#LIB = -L/opt/intel//mkl/8.1/lib/32 -lmkl_ia32
#LIB = -L/opt/intel/mkl/10.1.0.015/lib/em64t/ -lmkl -lguide -lpthread
#LIB = -lm
LIB =  -L../utd_time_unix -lutl_time -lm -L$(PAPI_HOME) $(PAPI_HOME)/libpapi.a -L../utd_papi_driver -lpapi_driver -L/usr/lib/x86_64-linux-gnu/openmpi/lib

# zaleznosci i komendy
all: mat_vec_test_driver mat_vec_test_driver_papi

mat_vec_test_driver: mat_vec_test_driver.o mat_vec_crs.o read_modfem_crs_matrix.o csrToSell.o mat_vec_sell.o mat_vec_vector.o
	$(LOADER) $(OPT) mat_vec_test_driver.o mat_vec_crs.o  read_modfem_crs_matrix.o csrToSell.o mat_vec_sell.o mat_vec_vector.o -o mat_vec_test_driver $(LIB)

mat_vec_test_driver.o: mat_vec_test_driver.c 
	$(CCOMP) -c $(OPT) mat_vec_test_driver.c $(INC)

mat_vec_test_driver_papi: mat_vec_test_driver_papi.o mat_vec_crs.o  mat_vec_sell.o mat_vec_vector.o read_modfem_crs_matrix.o  papi_set_user_events.o csrToSell.o
	$(LOADER) $(OPT) -DPAPI_TEST mat_vec_test_driver_papi.o  mat_vec_crs.o mat_vec_sell.o mat_vec_vector.o read_modfem_crs_matrix.o papi_set_user_events.o csrToSell.o -o mat_vec_test_driver_papi $(LIB)

mat_vec_test_driver_papi.o: mat_vec_test_driver.c  ../utd_papi_driver/papi_driver.h	
	$(CCOMP) -c $(OPT) mat_vec_test_driver.c -DPAPI_TEST -o mat_vec_test_driver_papi.o $(INC)

read_modfem_crs_matrix.o: read_modfem_crs_matrix.c
	$(CCOMP) -c $(OPT) read_modfem_crs_matrix.c

csrToSell.o: csrToSell.c
	$(CCOMP) -c $(OPT) csrToSell.c

mat_vec_crs.o: mat_vec_crs.c
	$(CCOMP) -c $(OPT) mat_vec_crs.c

mat_vec_sell.o: mat_vec_sell.c
	$(CCOMP) -c $(OPT) mat_vec_sell.c

mat_vec_vector.o: mat_vec_vector.c
	$(CCOMP) -c $(OPT) mat_vec_vector.c

papi_set_user_events.o: papi_set_user_events.c ../utd_papi_driver/papi_driver.h
	$(CCOMP) -c $(OPT) papi_set_user_events.c $(INC)


run:
	$(MPIRUN) -np 2 ./mat_vec_test_driver

clean:
	rm -f *.o

clean_papi_lib:
	rm -f ../utd_papi_driver/*.o ../utd_papi_driver/*.a

recreate_papi_lib:
	rm -f ../utd_papi_driver/*.o ../utd_papi_driver/*.a
	$(CCOMP) -c $(OPT) ../utd_papi_driver/papi_driver.c -o ../utd_papi_driver/papi_driver.o $(INC)
	ar -rs ../utd_papi_driver/libpapi_driver.a ../utd_papi_driver/papi_driver.o

recreate_time_lib:
	rm -f ../utd_time_unix/*.o ../utd_time_unix/*.a
	$(CCOMP) -c $(OPT) ../utd_time_unix/uts_time.c -o ../utd_time_unix/uts_time.o $(INC)
	ar -rs ../utd_time_unix/libutl_time.a ../utd_time_unix/uts_time.o

