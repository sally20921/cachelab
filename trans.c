//2016-12146 stu224@sp2.snucse.org

//there are 10 points for coding style. The course staff will inspect 
//your code in part 2 for illegal arrays and excessive local variables. 

/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 

/*
cache s = 5, E = 1, b = 5
-direct mapped cache, immediate eviction
-32 sets 
-32 byte per line */

#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    //you are allowed to define at most 12 local variables of type int
    //per transpose function.

    int sets_num = M/8;
    int sets_remainder = M%8;

    int i, j, k, l; 
    int f, g, h;

    int tmp, same;
    //int same; 
    /*blocking: divide the matrix into sub-matrices.
		size of sub-matrix depends on cache block size, cache size, 
		input matrix size.*/

/**32 X 32 ***************************************************************/
    if (M == 32 && N == 32){ 
	//divide the matrix into 8*8 block and be careful for the i == j eviction
        for(l=0; l < 4; l++){  
            for(k=0; k < 4; k++){ 
                /*block size 8 * 8 */
                for (i = l*8; i < l*8+8; i++){
                    for (j = k*8; j < k*8+8; j++) { 
                        if(i!=j) {
                            B[j][i]=A[i][j];
			} //special handling for same block 
                        else{
			//B[j][i] = A[i][j] for the same i, j will cause cache miss.
			//=> do the calculation at the end
                            tmp=A[j][j];  
                            same = j;
                        }
                    }
                    if(k==l){ 
			B[same][same] = tmp;
		    }
		/**************************/
                }
            }    
        }
    } 
/**32 X 32 ***************************************************************/

/**64 X 64 ***************************************************************/    
        else if (M==64 && N == 64){
           /* divide the matrix into 8*8 blocks, 
		do the calculation separately for upper half and lower half */
	//variables don't mean anything here. Just reusing because limited.
            for(i=0; i<64; i+=8){
                for(j=0; j<64; j+=8){
		   
                    for(k=0; k<4; k++){ 
                        l = A[i+k][j+0];
                        tmp = A[i+k][j+1];
                        same = A[i+k][j+2];

                        sets_remainder = A[i+k][j+3];
                        sets_num = A[i+k][j+4];

                        f = A[i+k][j+5];
                        g = A[i+k][j+6];
                        h = A[i+k][j+7];

                        B[j+0][i+k+0] = l;
                        B[j+0][i+k+4] = f;
                        B[j+1][i+k+0] = tmp;
                        B[j+1][i+k+4] = g;
                        B[j+2][i+k+0] = same;
                        B[j+2][i+k+4] = h;
                        B[j+3][i+k+0] = sets_remainder;
                        B[j+3][i+k+4] = sets_num;
                    }

                    l = A[i+4][j+4];
                    tmp = A[i+5][j+4];
                    same = A[i+6][j+4];

                    sets_remainder = A[i+7][j+4];
                    sets_num = A[i+4][j+3];

                    f = A[i+5][j+3];
                    g = A[i+6][j+3];
                    h = A[i+7][j+3];

                    B[j+4][i+0] = B[j+3][i+4];
                    B[j+4][i+4] = l;
                    B[j+3][i+4] = sets_num;
                    B[j+4][i+1] = B[j+3][i+5];
                    B[j+4][i+5] = tmp;
                    B[j+3][i+5] = f;
                    B[j+4][i+2] = B[j+3][i+6];
                    B[j+4][i+6] = same;
                    B[j+3][i+6] = g;
                    B[j+4][i+3] = B[j+3][i+7];
                    B[j+4][i+7] = sets_remainder;
                    B[j+3][i+7] = h;

                    for(k=0;k<3;k++){

                        l = A[i+4][j+5+k];
                        tmp = A[i+5][j+5+k];
                        same = A[i+6][j+5+k];
                        sets_remainder = A[i+7][j+5+k];
                        sets_num = A[i+4][j+k];
                        f = A[i+5][j+k];
                        g = A[i+6][j+k];
                        h = A[i+7][j+k];

                        B[j+5+k][i+0] = B[j+k][i+4];
                        B[j+5+k][i+4] = l;
                        B[j+k][i+4] = sets_num;
                        B[j+5+k][i+1] = B[j+k][i+5];
                        B[j+5+k][i+5] = tmp;
                        B[j+k][i+5] = f;
                        B[j+5+k][i+2] = B[j+k][i+6];
                        B[j+5+k][i+6] = same;
                        B[j+k][i+6] = g;
                        B[j+5+k][i+3] = B[j+k][i+7];
                        B[j+5+k][i+7] = sets_remainder;
                        B[j+k][i+7] = h;

                    }		
                     
                }
            }
        } 
/**64 X 64 ***************************************************************/  

/**61 X 67 ***************************************************************/
        else{  //naive approach works 
            /*for(k=0; k < sets_num; k++){
                for (i = 0; i < N; i++) {
                    for (j = 0; j < 8; j++) {
                        B[j+count][i]=A[i][j+count];
                    }
                }
                count +=8;
            }
            for (i = 0; i < N; i++) { 
                for (j = 0; j < sets_remainder; j++) {
                    tmp = A[i][j+count];
                    B[j+count][i] = tmp;
                }
            }
            count +=8;*/
	    for (i = 0; i < N; i+=8) {
		for (j = 0; j < M; j+= 8) {
			for (k = j;(k<j+8) && (k<M);++k) {
				for (l = i; (l<i+8)&&(l<N); ++l) {
					B[k][l] = A[l][k];
				}
			}
		}
	    }
        }
/**61 X 67 ***************************************************************/    
} 

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */

//to help you get started, we have given you an example transpose function 
//in trans.c that computes the transpose of NXM matrix A
//and stores the results in MXN matrix B
//char trans_desc[] = "Simple row-wise scan transpose";
//void trans(int M, int N, int A[N][M], int B[M][N])
//{
	/*int i, j, tmp;

	for (i = 0; i < N; i++) {
        	for (j = 0; j < M; j++) {
            		tmp = A[i][j];
           		 B[j][i] = tmp;
        	}
    	}*/   
//}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    registerTransFunction(transpose_submit, transpose_submit_desc);

    //registerTransFunction(trans, trans_desc);

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
