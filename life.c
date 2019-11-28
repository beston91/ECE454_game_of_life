/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/

/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
#define NUM_THREAD 1
typedef struct activeCell
{
	int curState:1;
	int nextState:1;
	int neighbor_count;
	unsigned int row;
	unsigned int col;
}activeCell;

typedef struct threadInput
{
    //THREAD SPECIFIC VARIABLES
    int _tid;    //thread id
    pthread_barrier_t * _barr;
    //NORMAL VARIABLES
    char* _outboard;
    char* _inboard;
    int _nrows;
    int _ncols;
    int _gens_max;
    int _LDA;
    activeCell* _changeList;
    int _changeListSize;
} threadInput;


// typedef struct bitmap{
//     unsigned int b:4;
// } bitmap;
/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

//mutex
pthread_mutex_t mut_e[1024 * 1024];
// static char map[] = 
// {
//   0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,//
// };

static int log_base_two(unsigned int v) {

	static const int MultiplyDeBruijnBitPosition[32] = 
	{
	  0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
	  8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
	};

	v |= v >> 1; // first round down to one less than a power of 2 
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;

	return MultiplyDeBruijnBitPosition[(unsigned int)(v * 0x07C4ACDDU) >> 27];
}
// void*
// optimised_game_of_life (void * _input)
// {   
// 	//house keeping
// 	threadInput * input = (threadInput *) _input;
//     /* HINT: in the parallel decomposition, LDA may not be equal to
//        nrows! */
// 	register int nrows = input->_nrows;
//     register int ncols = input->_ncols;
//     register int gens_max = input->_gens_max;
// 	register int tid = input->_tid;    //thread id
//     register pthread_barrier_t * barr = input->_barr;
//     //NORMAL VARIABLES
//     register char* outboard = input->_outboard;
//     register char* inboard = input->_inboard;


//     register int LDA = input->_LDA;
//     // int size = nrows * ncols * 2 / NUM_THREAD;
//     // int t1[size];
//     // int i;
//     // int j;
//     // int activeCells = 0;
//     // for(j = 0; j < ncols; j++) {
//     //     const unsigned int LDA_j = LDA *j;
//     //     for(i = 0; i < nrows; i++) {
//     //         if(inboard[i + LDA_j] == (char)1) {
//     //             // printf("%d, %d\n" ,i, j);
//     //             t1[i + LDA_j] = i;
//     //             t1[i+LDA_j + 1] = j;
//     //             activeCells++;
//     //         }
//     //     }
//     // }
    
//     int curgen, i, j;
//     // int curgen;
//     register int row_start = (nrows/NUM_THREAD) * (tid);
//     register int row_end   = (nrows/NUM_THREAD) * (tid + 1);
//     // register int col_start = (ncols/NUM_THREAD) * (tid);
//     // register int col_end   = (ncols/NUM_THREAD) * (tid + 1);


//     for (curgen = 0; curgen < gens_max; curgen++)
//     {
//         /* HINT: you'll be parallelizing these loop(s) by doing a
//            geometric decomposition of the output */
//     	for (i = row_start; i < row_end; i++) {
//                 const int inorth = i == 0 ? nrows- 1 :  i-1;
//                 const int isouth = (i == (nrows - 1)) ? 0: i + 1;
//                 const unsigned int LDA_inorth = inorth << LDA;
//             	const unsigned int LDA_isouth = isouth << LDA;
//                 const unsigned int LDA_i = i << LDA;
//                 // register bitmap bit;
//                 // if(inboard[LDA_inorth + ncols-1]) {
//                 // 	bit.b |= 1UL << 3;
//                 // }
//                 // if(inboard[LDA_isouth + ncols-1]) {
//                 // 	bit.b |= 1UL << 2;
//                 // }
//                 // if(inboard[LDA_inorth]){
//                 // 	bit.b |= 1UL << 1;
//                 // }
//                 // if(inboard[LDA_isouth]){
//                 // 	bit.b |= 1UL << 0;
//                 // }
//                 // bit.b |= 1UL << 0;
//                 char prev = 
//                 	inboard[LDA_inorth + ncols-1] +
//                 	inboard[LDA_isouth + ncols-1];
//             	char cur = 
//                 	inboard[LDA_inorth] +
//                 	inboard[LDA_isouth]; 
//                 for (j = 0; j < ncols; j++) {
           
// 		            //the mod function just wrap around
// 		            const int jwest = j == 0 ? ncols- 1 :  j-1;
// 		            const int jeast = (j == (ncols - 1)) ? 0: j + 1;
// 		            // const char count = map[bit.b];
// 		            // bit.b = bit.b << 2;
// 		            // char next = 0;
// 		            // if(inboard[LDA_inorth + jeast]) {
// 		            // 	bit.b |= 1UL << 1;
// 		            // 	next += 1;
// 		            // }
// 		            // if( inboard[LDA_isouth + jeast]) {
// 		            // 	bit.b |= 1UL << 0;
// 		            // 	next += 1; 
// 		            // }

// 		            const char next1 = 
// 	                    inboard[LDA_inorth + jeast] +
// 	                    inboard[LDA_isouth + jeast];
// 	                // printf("%d, %d\n", next, next1);
// 	                // Optimization Note : Reducing pointer usage with prev/cur
// 	                const char neighbor_count =
// 	                    prev +
// 	                    cur+
// 	                    next1 +
// 	                    inboard[LDA_i + jwest] +
// 	                    inboard[LDA_i + jeast];

// 	                prev = cur;
// 	                cur = next1;


// 	                unsigned char alivep = 0;
// 	                if(neighbor_count == 3)
// 	                    alivep = 1;
// 	                else if(neighbor_count == 2 && inboard[j + LDA_i])
// 	                    alivep = 1;
// 	                outboard[j + LDA_i] = alivep; 

//          		}
//      	}
//         // for (j = 0; j < ncols; j++)
//         // {
           
//         //     //the mod function just wrap around
//         //     const int jwest = j == 0 ? ncols- 1 :  j-1;
//         //     const int jeast = (j == (ncols - 1)) ? 0: j + 1;
//         //     const unsigned int LDA_j = LDA *j;
//         //     //  printf("%d\n", inboard[i + LDA_j]);
//         //     for (i = row_start; i < row_end; i++)
//         //     {
//         //         const int inorth = i == 0 ? nrows- 1 :  i-1;
//         //         const int isouth = (i == (nrows - 1)) ? 0: i + 1;

//         //         const char neighbor_count = 
//         //             BOARD (inboard, inorth, jwest) + 
//         //             BOARD (inboard, inorth, j) + 
//         //             BOARD (inboard, inorth, jeast) + 
//         //             BOARD (inboard, i, jwest) +
//         //             BOARD (inboard, i, jeast) + 
//         //             BOARD (inboard, isouth, jwest) +
//         //             BOARD (inboard, isouth, j) + 
//         //             BOARD (inboard, isouth, jeast);
//         //         unsigned char alivep = 0;
//         //         if(neighbor_count == 3)
//         //             alivep = 1;
//         //         else if(neighbor_count == 2 && inboard[i + LDA_j])
//         //             alivep = 1;
//         //         outboard[i + LDA_j] = alivep; 

//         //     }
//         // }
//         // int x;
//         // for(x = 0; x < activeCells; x= x +2) {
//         //     //living cell
//         //     if(i < 0 || j <0) continue;
//         //     int i = t1[x];
//         //     int j = t1[x+1];
//         //     const unsigned int LDA_j = LDA *j;
//         //     int k;
//         //     int l;
//         //     if(inboard[i + LDA_j] == (char) 1) {
//         //         for(k = -1; k <= 1; k++) {
//         //             for(l = -1; l <= 1; l++) {
//         //                 t1[activeCells] = i + k;
//         //                 t1[activeCells + 1] = j+l;
//         //                 activeCells++;
//         //             }
//         //         }
//         //     }
//         //     const int jwest = j == 0 ? ncols- 1 :  j-1;
//         //     const int jeast = (j == (ncols - 1)) ? 0: j + 1;
//         //     const int inorth = i == 0 ? nrows- 1 :  i-1;
//         //     const int isouth = (i == (nrows - 1)) ? 0: i + 1;
//         //     const char neighbor_count = 
//         //     BOARD (inboard, inorth, jwest) + 
//         //     BOARD (inboard, inorth, j) + 
//         //     BOARD (inboard, inorth, jeast) + 
//         //     BOARD (inboard, i, jwest) +
//         //     BOARD (inboard, i, jeast) + 
//         //     BOARD (inboard, isouth, jwest) +
//         //     BOARD (inboard, isouth, j) + 
//         //     BOARD (inboard, isouth, jeast);
//         //     unsigned char alivep = 0;
//         //     if(neighbor_count == 3)
//         //         alivep = 1;
//         //     else if(neighbor_count == 2 && inboard[i + LDA_j])
//         //         alivep = 1;
            
//         //     outboard[i + LDA_j] = alivep; 
            
//         // }
//         pthread_barrier_wait (barr);
//         SWAP_BOARDS( outboard, inboard );
//     }

//     pthread_exit(NULL);
// }
void*
optimised_game_of_life (void * _input)
{   
	//house keeping
	threadInput * input = (threadInput *) _input;
    /* HINT: in the parallel decomposition, LDA may not be equal to
       nrows! */
	register int nrows = input->_nrows;
    register int ncols = input->_ncols;
    register int gens_max = input->_gens_max;
	register int tid = input->_tid;    //thread id
    register pthread_barrier_t * barr = input->_barr;
    //NORMAL VARIABLES
    register char* outboard = input->_outboard;
    register char* inboard = input->_inboard;
    register activeCell* changeList = input->_changeList;
    register int changeListSize = input->_changeListSize;

    register int LDA = input->_LDA;
    
    int curgen, x;
    // int curgen;
    register int row_start = (nrows/NUM_THREAD) * (tid);
    register int row_end   = (nrows/NUM_THREAD) * (tid + 1);

    // activeCell *curChangeList = malloc(nrows * ncols * sizeof(activeCell));
    activeCell *nextChangeList = malloc(nrows * ncols * sizeof(activeCell));
    int nextChangeListSize = 0;
    printf("here\n");
    for (curgen = 0; curgen < gens_max; curgen++)
    {	
    	
    	//first pass
    	for(x = 0; x < changeListSize; x++) {
    		int i = changeList[x].row;
    		int j = changeList[x].col;
    		int nextState = changeList[x].nextState;
    		  		// printf("%d, %d \n", i, j);
		    // check the 8 nieghbour
		    int m,n;
		    for(m = -1; m <=1; m++) {
		    	for(n = -1; n <= 1; n++){
		    		if(m ==0 && n == 0) continue;
		    		int curRow, curCol;
		    		if(m == -1)
		    			curRow = i + m == 0 ? nrows- 1: i + m;
		    		else
		    			curRow = i + m ==(nrows - 1) ? 0 : i + m;

		    		if(n == -1)
		    			curCol = j + n == 0 ? ncols - 1: j + n;
		    		else
		    			curCol = j + n == (ncols-1) ? 0 : j + n;

		    		const int inorth = curRow == 0 ? nrows- 1 :  curRow-1;
		            const int isouth = (curRow == (nrows - 1)) ? 0: curRow + 1;
		            const unsigned int LDA_inorth = inorth << LDA;
		        	const unsigned int LDA_isouth = isouth << LDA;
		            const unsigned int LDA_i = curRow << LDA;
		            const int jwest = curCol == 0 ? ncols- 1 :  curCol-1;
				    const int jeast = (curCol == (ncols - 1)) ? 0: curCol + 1;
				    const char neighbor_count = 
				    inboard[LDA_inorth + ncols-1] +
                	inboard[LDA_isouth + ncols-1] +
                	inboard[LDA_inorth] +
                	inboard[LDA_isouth] + 
                	inboard[LDA_inorth + jeast] +
	                inboard[LDA_isouth + jeast] + 
	                inboard[LDA_i + jwest]+
	                inboard[LDA_i + jeast] +
	                nextState;

	                unsigned char alivep = 0;
	                if(neighbor_count == 3)
	                    alivep = 1;
	                else if(neighbor_count == 2 && inboard[j + LDA_i])
	                    alivep = 1;

	                char curState = inboard[LDA_i + curCol];
	                printf("**%d, %d\n", curRow, curCol);
		        	if(curState != alivep) {
		        		//push to changeList
		        		// printf("%d, %d\n", curRow, curCol);
		        		activeCell ap;
		        		ap.curState = (int)curState;
		        		ap.nextState = alivep;
		        		ap.neighbor_count = neighbor_count;
		        		ap.row = i;
		        		ap.col = j;
		        		// nextChangeList[nextChangeListSize] = ap;
		        		// printf("%d, %d\n", i, j);
		        		nextChangeListSize = nextChangeListSize + 1;
		        	}
		    	}
		    }


    	}
        /* HINT: you'll be parallelizing these loop(s) by doing a
           geometric decomposition of the output */
    	// for (i = row_start; i < row_end; i++) {
     //            const int inorth = i == 0 ? nrows- 1 :  i-1;
     //            const int isouth = (i == (nrows - 1)) ? 0: i + 1;
     //            const unsigned int LDA_inorth = inorth << LDA;
     //        	const unsigned int LDA_isouth = isouth << LDA;
     //            const unsigned int LDA_i = i << LDA;
     //            char prev = 
     //            	inboard[LDA_inorth + ncols-1] +
     //            	inboard[LDA_isouth + ncols-1];
     //        	char cur = 
     //            	inboard[LDA_inorth] +
     //            	inboard[LDA_isouth]; 
     //            for (j = 0; j < ncols; j++) {
           
		   //          //the mod function just wrap around
		   //          const int jwest = j == 0 ? ncols- 1 :  j-1;
		   //          const int jeast = (j == (ncols - 1)) ? 0: j + 1;

		   //          const char next1 = 
	    //                 inboard[LDA_inorth + jeast] +
	    //                 inboard[LDA_isouth + jeast];
	    //             // printf("%d, %d\n", next, next1);
	    //             // Optimization Note : Reducing pointer usage with prev/cur
	    //             const char neighbor_count =
	    //                 prev +
	    //                 cur+
	    //                 next1 +
	    //                 inboard[LDA_i + jwest] +
	    //                 inboard[LDA_i + jeast];

	    //             prev = cur;
	    //             cur = next1;


	    //             unsigned char alivep = 0;
	    //             if(neighbor_count == 3)
	    //                 alivep = 1;
	    //             else if(neighbor_count == 2 && inboard[j + LDA_i])
	    //                 alivep = 1;
	    //             outboard[j + LDA_i] = alivep; 

     //     		}
     // 	}

        pthread_barrier_wait (barr);
        SWAP_BOARDS( outboard, inboard );
    }

    pthread_exit(NULL);
}


int initialisation(char* inboard, const int nrows, const int ncols, activeCell *changeList, int LDA) {
	int i, j;
	int count = 0;
	for(i = 0; i< nrows; i ++) {
		const int inorth = i == 0 ? nrows- 1 :  i-1;
        const int isouth = (i == (nrows - 1)) ? 0: i + 1;
        const unsigned int LDA_inorth = inorth << LDA;
    	const unsigned int LDA_isouth = isouth << LDA;
        const unsigned int LDA_i = i << LDA;
        char prev = 
        	inboard[LDA_inorth + ncols-1] +
        	inboard[LDA_isouth + ncols-1];
    	char cur = 
        	inboard[LDA_inorth] +
        	inboard[LDA_isouth]; 
        for (j = 0; j < ncols; j++) {
   
            //the mod function just wrap around
            const int jwest = j == 0 ? ncols- 1 :  j-1;
            const int jeast = (j == (ncols - 1)) ? 0: j + 1;

            const char next1 = 
                inboard[LDA_inorth + jeast] +
                inboard[LDA_isouth + jeast];
            const char neighbor_count =
                prev +
                cur+
                next1 +
                inboard[LDA_i + jwest] +
                inboard[LDA_i + jeast];

            prev = cur;
            cur = next1;

            unsigned char alivep = 0;
            if(neighbor_count == 3) {
                alivep = 1;
            }
            else if(neighbor_count == 2 && inboard[j + LDA_i]){
                alivep = 1;
            }

        	char curState = inboard[LDA_i + j];

        	if(curState != alivep) {
        		//push to changeList
        		activeCell ap;
        		ap.curState = (int)curState;
        		ap.nextState = alivep;
        		ap.neighbor_count = neighbor_count;
        		ap.row = i;
        		ap.col = j;
        		changeList[count] = ap;
        		// printf("%d, %d\n", i, j);
        		count = count + 1;
        	}

 		}
	}
	return count;

}

char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
	// house keeping
  	pthread_barrier_t   barr;
  	pthread_barrier_init (&barr, NULL, NUM_THREAD);
  	// printf("here\n");
  	pthread_t tid[NUM_THREAD];
  	threadInput tInput[NUM_THREAD];

  	int x = 0;
  	for( x = 0; x <nrows * ncols; x++) {
  		pthread_mutex_init(&mut_e[x], NULL); 
  	}

  	int i;
  	int ll = log_base_two(nrows);
  	activeCell *changeList = malloc(nrows * ncols * sizeof(activeCell));
  	int changeListSize = initialisation(inboard, nrows, ncols, changeList, ll);


  	for(i = 0; i < changeListSize; i++) {
  		printf("&&&&%d, %d \n", changeList[i].row, changeList[i].col);
  	}
  	for(i = 0; i < NUM_THREAD; i++) { 
  		tInput[i]._tid = i;
  		tInput[i]._barr = &barr;

  		tInput[i]._nrows = nrows;
  		tInput[i]._ncols = ncols;
  		tInput[i]._outboard = outboard;
  		tInput[i]._inboard = inboard;
  		tInput[i]._gens_max = gens_max;
  		tInput[i]._LDA = ll;
  		tInput[i]._changeList = changeList;
  		tInput[i]._changeListSize = changeListSize;
  	}

  	for(i = 0; i < NUM_THREAD; i++) {
  		pthread_create(&tid[i], NULL, &optimised_game_of_life, (void*) &tInput[i]);
  	}

  	for(i = 0; i < NUM_THREAD; i++) {
  		pthread_join(tid[i], NULL);
  	}
  	for( x = 0; x <nrows * ncols; x++) {
  		pthread_mutex_destroy(&mut_e[x]); 
  	}
  	return inboard;
}
