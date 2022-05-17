#include "pmsis.h"
#include "perf.h"

#define M 32
#define N 32
#define O 32


/* 
  Data Stored in L2 memory
*/
PI_L2 int matA[M*N];
PI_L2 int matB[N*O];
PI_L2 int matC[M*O];

/* 
  Allocate space for data in L1
*/
PI_L1 int matA_cl[M*N];
PI_L1 int matB_cl[N*O];
PI_L1 int matC_cl[M*O];

void matMul(int * pSrcA, int  * pSrcB, int * pDstC, int m, int n, int o);


// Helper functions

void matrix_init(int * A, int * B, int * C) {
  if(pi_core_id() == 0)
  {

    for (int i = 0; i < M; i++) {
      for (int j = 0; j < N; j++) {
        A[i*N+j] = (int)(j+1);
      }
    }

    for (int i = 0; i <N; i++) {
      for (int j = 0; j < O; j++) {
        B[i*O+j] = i+1;
      }
    }

    for (int i = 0; i < M; i++) {
      for (int j = 0; j < O; j++) {
        C[i*O+j] = 0;
      }
    }

  }

  pi_cl_team_barrier();
}

unsigned int matrix_check(int * C) {
  unsigned int errors = 0;
  // check
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < O; j++) {
      int ref_val = ((N)*(N+1)*(2*(N)+1))/6;
      if (C[i*O+j] != ref_val) {
        printf("Error at index (%d,%d): %d instead of %d\n", i, j, C[i*N+j], ref_val);
        errors++;
      }
    }
  }
  return errors;
}


void cluster_fn() {
  volatile int m = M;
  volatile int n = N;
  volatile int o = O;

  pi_cl_dma_cmd_t command1, command2;

  // init performance counters
  INIT_STATS();

  // set initial values (not considered by performance counters)
  matrix_init(matA, matB, matC);

  // coy data
  RESET_STATS();
  START_STATS();

  for(int i=pi_core_id(); i<M*N; i+=NUM_CORES)
    matA_cl[i] = matA[i];
  for(int i=pi_core_id(); i<N*O; i+=NUM_CORES)
    matB_cl[i] = matB[i];

  pi_cl_team_barrier();
  STOP_STATS();
  printf("Copy data from L2 to L1\n");
  PRINT_STATS();

  // workload
  RESET_STATS();
  START_STATS();
  matMul(matA_cl, matB_cl, matC_cl, m, n, o);
  STOP_STATS();
  printf("Matrix Multiplication\n");
  PRINT_STATS();

  // copy results to L2
  RESET_STATS();
  START_STATS();

  for(int i=pi_core_id(); i<M*O; i+=NUM_CORES)
    matC[i] = matC_cl[i];

  pi_cl_team_barrier();
  STOP_STATS();
  printf("Copy result from L2 to L1\n");
  PRINT_STATS();

#ifdef DEBUG  
  // check the result (optional)
  if(pi_core_id() == 0){
    if(matrix_check(matC) == 0)
      printf("Checksum OK!\n");
    else
      printf("Checksum not OK!\n");

  }
#endif  
}
