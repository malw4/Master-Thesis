# Master-Thesis

[PL] Praca dotyczy optymalizacji operacji mnożenia macierz-wektor dla macierzy rzadkich, co stanowi istotny element wielu algorytmów numerycznych, szczególnie w kontekście rozwiązywania równań liniowych. Celem badań było porównanie wydajności jednowątkowych, wielowątkowych (OpenMP) oraz równoległych (MPI) implementacji mnożenia macierz-wektor, przy użyciu różnych formatów przechowywania macierzy rzadkich, takich jak CSR i SELL.

W części teoretycznej omówiona została charakterystyka macierzy rzadkich, formaty ich przechowywania oraz algorytmy dedykowane dla obliczeń równoległych. Przedstawiono również budowę współczesnych procesorów wielordzeniowych, systemy wieloprocesorowe, oraz techniki obliczeń równoległych i wektoryzacji kodu, a także miary wydajności oraz narzędzia analizy, takie jak profilery czy liczniki zdarzeń sprzętowych (perf).

W części eksperymentalnej dokonano implementacji algorytmów dla wybranych formatów przechowywania macierzy, analizując generowany kod asemblera w celu zrozumienia sposobu wykonywania operacji na poziomie sprzętowym. Wykorzystano narzędzie perf do monitorowania parametrów wydajności, takich jak liczba operacji zmiennoprzecinkowych czy cache misses. Eksperymenty przeprowadzono na serwerze Honorata i superkomputerze Ares. Wyniki badań przedstawiono w formie wykresów i tabel porównując różne konfiguracje wykonania.

[ENG]
The work focuses on optimizing the matrix-vector multiplication operation for sparse matrices, a crucial component of many numerical algorithms, particularly in solving linear equations. The research aimed to compare the performance of single-threaded, multi-threaded (OpenMP), and parallel (MPI) implementations of matrix-vector multiplication using various sparse matrix storage formats, such as CSR and SELL.

The theoretical part discusses the characteristics of sparse matrices, storage formats, and algorithms designed for parallel computations. It also covers the architecture of modern multi-core processors, multi-processor systems, parallel computing techniques, code vectorization, performance metrics, and analysis tools such as profilers and hardware performance counters (e.g., perf).

In the experimental part, algorithms were implemented for selected sparse matrix storage formats. The generated assembly code was analyzed to understand the hardware-level execution of operations. The tools were used to monitor performance parameters, such as the number of floating-point operations and cache misses. Experiments were conducted on the Honorata server and the Ares supercomputer. The results are presented in the form of graphs and tables, comparing various configurations.
