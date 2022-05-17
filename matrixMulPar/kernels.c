#include "pmsis.h"


#if NUM_CORES == 1
// Sequential version
void matMul(int * pSrcA, int  * pSrcB, int * pDstC, int m, int n, int o) {
    uint32_t i, j, k;

    for (k = 0; k < o; k++) {
        for (i = 0; i < m; i++) {
            int32_t sum = 0;
            for (j = 0; j < n; j++) {
                int32_t a = pSrcA[i * n + j];
                int32_t b = pSrcB[j * o + k];
                sum += a * b;
            }
            pDstC[i * o + k] = sum;
        }
    }
}
#else
// Parallel version
void matMul(int * pSrcA, int  * pSrcB, int * pDstC, int m, int n, int o) {
    uint32_t i, j, k;

    int core_id = pi_core_id();

    for (k = core_id; k < o; k+=NUM_CORES) {
        for (i = 0; i < m; i++) {
            int32_t sum = 0;
            for (j = 0; j < n; j++) {
                int32_t a = pSrcA[i * n + j];
                int32_t b = pSrcB[j * o + k];
                sum += a * b;
            }
            pDstC[i * o + k] = sum;
        }
    }

    pi_cl_team_barrier();
}
#endif
