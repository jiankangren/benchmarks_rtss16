/*
 * Sparse Cholesky code with little blocks at the leaves of the Quad tree
 * Keith Randall -- Aske Plaat
 *
 * This code should run with any square sparse real symmetric matrix
 * from MatrixMarket (http://math.nist.gov/MatrixMarket)
 *
 * run with `cholesky -f george-liu.mtx' for a given matrix, or
 * `cholesky -n 1000 -z 10000' for a 1000x1000 random matrix with 10000
 * nonzeros (caution: random matrices produce lots of fill).
 */
static const char *ident __attribute__((__unused__))
     = "$HeadURL: https://bradley.csail.mit.edu/svn/repos/cilk/5.4.3/examples/cholesky.cilk $ $LastChangedBy: bradley $ $Rev: 517 $ $Date: 2004-06-22 00:03:24 -0400 (Tue, 22 Jun 2004) $";
/*
 * Copyright (c) 2000 Massachusetts Institute of Technology
 * Copyright (c) 2000 Matteo Frigo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#if defined(__CILKPLUS__)
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#elif defined(__OPENMP__)
#include <omp.h>
#endif

#if defined(__CILKPLUS__)
#if defined(__CILKPROF__)
#include "cilktool.h"
#elif defined(__CILKVIEW__)
#include "cilkview.h"
#endif
#endif // __CILKPLUS__

#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "timespec_functions.h"
#include "getoptions.h"

#if HAVE_MALLOC_H
#include <malloc.h>
#endif

extern int Cilk_rand(void);
extern void Cilk_srand(unsigned int seed);

/*************************************************************\
 * Basic types
\*************************************************************/

typedef double Real;

// After exploring base case for Cilk Plus and OpenMP, the best base 
// case for Cilk Plus seems to be 16 and the best base case for OpenMP
// seems to be 32.
// Also it seems OpenMP is more sensitive to different base cases, e.g.,
// for base case of size 4, it performs very bad. On the other hand, 
// Cilk Plus performs equally good for different base case sizes.

#if defined(__CILKPLUS__)
#define BLOCK_DEPTH 5		/* logarithm base 2 of BLOCK_SIZE */
#elif defined(__OPENMP__)
#define BLOCK_DEPTH 5
#endif

#define BLOCK_SIZE  (1<<BLOCK_DEPTH)	/* 4 seems to be the optimum */

typedef Real Block[BLOCK_SIZE][BLOCK_SIZE];

#define BLOCK(B,I,J) (B[I][J])

#define _00 0
#define _01 1
#define _10 2
#define _11 3

#define TR_00 _00
#define TR_01 _10
#define TR_10 _01
#define TR_11 _11

typedef struct InternalNode {
     struct InternalNode *child[4];
} InternalNode;

typedef struct {
     Block block;
} LeafNode;

typedef InternalNode *Matrix;

/*************************************************************\
 * Linear algebra on blocks
\*************************************************************/

/*
 * elem_daxmy - Compute y' = y - ax where a is a Real and x and y are
 * vectors of Reals.
 */
static void elem_daxmy(Real a, Real * x, Real * y, int n)
{
     for (n--; n >= 0; n--)
	  y[n] -= a * x[n];
}

/*
 * block_schur - Compute Schur complement B' = B - AC.
 */
static void block_schur_full(Block B, Block A, Block C)
{
     int i, j, k;
     for (i = 0; i < BLOCK_SIZE; i++) {
	  for (j = 0; j < BLOCK_SIZE; j++) {
	       for (k = 0; k < BLOCK_SIZE; k++) {
		    BLOCK(B, i, j) -= BLOCK(A, i, k) * BLOCK(C, j, k);
	       }
	  }
     }
}

/*
 * block_schur - Compute Schur complement B' = B - AC.
 */
static void block_schur_half(Block B, Block A, Block C)
{
     int i, j, k;

     /*
      * printf("schur half\n");
      */
     /* Compute Schur complement. */
     for (i = 0; i < BLOCK_SIZE; i++) {
		 for (j = 0; j <= i /* BLOCK_SIZE */ ; j++) {
			 for (k = 0; k < BLOCK_SIZE; k++) {
				 BLOCK(B, i, j) -= BLOCK(A, i, k) * BLOCK(C, j, k);
			 }
		 }
     }
}

/*
 * block_upper_solve - Perform substitution to solve for B' in
 * B'U = B.
 */
static void block_backsub(Block B, Block U)
{
     int i, j, k;

     /* Perform backward substitution. */
     for (i = 0; i < BLOCK_SIZE; i++) {
	  for (j = 0; j < BLOCK_SIZE; j++) {
	       for (k = 0; k < i; k++) {
		    BLOCK(B, j, i) -= BLOCK(U, i, k) * BLOCK(B, j, k);	/* transpose? */
	       }
	       BLOCK(B, j, i) /= BLOCK(U, i, i);
	  }
     }
}

/*
 * block_lower_solve - Perform forward substitution to solve for B' in
 * LB' = B.
 */
static void xblock_backsub(Block B, Block L)
{
     int i, k;

     /* Perform forward substitution. */
     for (i = 0; i < BLOCK_SIZE; i++)
	  for (k = 0; k <= i; k++) {
	       BLOCK(B, i, k) /= BLOCK(L, k, k);
	       elem_daxmy(BLOCK(L, i, k), &BLOCK(B, k, 0),
			  &BLOCK(B, i, 0), BLOCK_SIZE - k);
	  }
}

/*
 * block_cholesky - Factor block B.
 */
static void block_cholesky(Block B)
{
     int i, j, k;

     for (k = 0; k < BLOCK_SIZE; k++) {
	  Real x;
	  if (BLOCK(B, k, k) < 0.0) {
	       printf("sqrt error: %f\n", BLOCK(B, k, k));
	       printf("matrix is probably not numerically stable\n");
	       exit(9);
	  }
	  x = sqrt(BLOCK(B, k, k));
	  for (i = k; i < BLOCK_SIZE; i++) {
	       BLOCK(B, i, k) /= x;
	  }
	  for (j = k + 1; j < BLOCK_SIZE; j++) {
	       for (i = j; i < BLOCK_SIZE; i++) {
		    BLOCK(B, i, j) -= BLOCK(B, i, k) * BLOCK(B, j, k);
		    if (j > i && BLOCK(B, i, j) != 0.0) {
			 printf("Upper not empty\n");
		    }
	       }
	  }
     }
}

/*
 * block_zero - zero block B.
 */
static void block_zero(Block B)
{
     int i, k;

     for (i = 0; i < BLOCK_SIZE; i++) {
	  for (k = 0; k < BLOCK_SIZE; k++) {
	       BLOCK(B, i, k) = 0.0;
	  }
     }
}

/*************************************************************\
 * Allocation and initialization
\*************************************************************/

/*
 * Create new leaf nodes (BLOCK_SIZE x BLOCK_SIZE submatrices)
 */
inline InternalNode *new_block_leaf(void)
{
	LeafNode *leaf = (LeafNode*) malloc(sizeof(LeafNode));
     if (leaf == NULL) {
	  printf("out of memory!\n");
	  exit(1);
     }
     return (InternalNode *) leaf;
}

/*
 * Create internal node in quadtree representation
 */
inline InternalNode *new_internal(InternalNode * a00, InternalNode * a01,
				  InternalNode * a10, InternalNode * a11)
{
	InternalNode *node = (InternalNode*) malloc(sizeof(InternalNode));
     if (node == NULL) {
	  printf("out of memory!\n");
	  exit(1);
     }
     node->child[_00] = a00;
     node->child[_01] = a01;
     node->child[_10] = a10;
     node->child[_11] = a11;
     return node;
}

/*
 * Duplicate matrix.  Resulting matrix may be laid out in memory
 * better than source matrix.
 */
Matrix copy_matrix(int depth, Matrix a)
{
     Matrix r;

     if (!a)
	  return a;

     if (depth == BLOCK_DEPTH) {
	  LeafNode *A = (LeafNode *) a;
	  LeafNode *R;
	  r = new_block_leaf();
	  R = (LeafNode *) r;
	  memcpy(R->block, A->block, sizeof(Block));
     } else {
	  Matrix r00, r01, r10, r11;

	  depth--;

#if defined(__CILKPLUS__)
	  r00 = cilk_spawn copy_matrix(depth, a->child[_00]);
	  r01 = cilk_spawn copy_matrix(depth, a->child[_01]);
	  r10 = cilk_spawn copy_matrix(depth, a->child[_10]);
	  r11 = copy_matrix(depth, a->child[_11]);

	  cilk_sync;
#elif defined(__OPENMP__)
      #pragma omp task shared(r00, depth, a)
	  r00 = copy_matrix(depth, a->child[_00]);

      #pragma omp task shared(r01, depth, a)
	  r01 = copy_matrix(depth, a->child[_01]);

      #pragma omp task shared(r10, depth, a)
	  r10 = copy_matrix(depth, a->child[_10]);

      #pragma omp task shared(r11, depth, a)
	  r11 = copy_matrix(depth, a->child[_11]);
	  
      #pragma omp taskwait
#endif

	  r = new_internal(r00, r01, r10, r11);
     }
     return r;
}

/*
 * Deallocate matrix.
 */
void free_matrix(int depth, Matrix a)
{
     if (a == NULL)
	  return;
     if (depth == BLOCK_DEPTH) {
	  free(a);
     } else {
	  depth--;
	  free_matrix(depth, a->child[_00]);
	  free_matrix(depth, a->child[_01]);
	  free_matrix(depth, a->child[_10]);
	  free_matrix(depth, a->child[_11]);
	  free(a);
     }
}

/*************************************************************\
 * Simple matrix operations
\*************************************************************/

/*
 * Get matrix element at row r, column c.
 */
Real get_matrix(int depth, Matrix a, int r, int c)
{
     assert(depth >= BLOCK_DEPTH);
     assert(r < (1 << depth));
     assert(c < (1 << depth));

     if (a == NULL)
	  return 0.0;

     if (depth == BLOCK_DEPTH) {
	  LeafNode *A = (LeafNode *) a;
	  return BLOCK(A->block, r, c);
     } else {
	  int mid;

	  depth--;
	  mid = 1 << depth;

	  if (r < mid) {
	       if (c < mid)
		    return get_matrix(depth, a->child[_00], r, c);
	       else
		    return get_matrix(depth, a->child[_01], r, c - mid);
	  } else {
	       if (c < mid)
		    return get_matrix(depth, a->child[_10], r - mid, c);
	       else
		    return get_matrix(depth, a->child[_11], r - mid, c - mid);
	  }
     }
}

/*
 * Set matrix element at row r, column c to value.
 */
Matrix set_matrix(int depth, Matrix a, int r, int c, Real value)
{
     assert(depth >= BLOCK_DEPTH);
     assert(r < (1 << depth));
     assert(c < (1 << depth));

     if (depth == BLOCK_DEPTH) {
	  LeafNode *A;
	  if (a == NULL) {
	       a = new_block_leaf();
	       A = (LeafNode *) a;
	       block_zero(A->block);
	  } else {
	       A = (LeafNode *) a;
	  }
	  BLOCK(A->block, r, c) = value;
     } else {
	  int mid;

	  if (a == NULL)
	       a = new_internal(NULL, NULL, NULL, NULL);

	  depth--;
	  mid = 1 << depth;

	  if (r < mid) {
	       if (c < mid)
		    a->child[_00] = set_matrix(depth, a->child[_00],
					       r, c, value);
	       else
		    a->child[_01] = set_matrix(depth, a->child[_01],
					       r, c - mid, value);
	  } else {
	       if (c < mid)
		    a->child[_10] = set_matrix(depth, a->child[_10],
					       r - mid, c, value);
	       else
		    a->child[_11] = set_matrix(depth, a->child[_11],
					       r - mid, c - mid, value);
	  }
     }
     return a;
}

void print_matrix_aux(int depth, Matrix a, int r, int c)
{
     if (a == NULL)
	  return;

     if (depth == BLOCK_DEPTH) {
	  LeafNode *A = (LeafNode *) a;
	  int i, j;
	  for (i = 0; i < BLOCK_SIZE; i++)
	       for (j = 0; j < BLOCK_SIZE; j++)
		    printf("%6d %6d: %12f\n", r + i, c + j, BLOCK(A->block, i, j));
     } else {
	  int mid;
	  depth--;
	  mid = 1 << depth;
	  print_matrix_aux(depth, a->child[_00], r, c);
	  print_matrix_aux(depth, a->child[_01], r, c + mid);
	  print_matrix_aux(depth, a->child[_10], r + mid, c);
	  print_matrix_aux(depth, a->child[_11], r + mid, c + mid);
     }
}

/*
 * Print matrix
 */
void print_matrix(int depth, Matrix a)
{
     print_matrix_aux(depth, a, 0, 0);
}

/*
 * Count number of blocks (leaves) in matrix representation
 */
int num_blocks(int depth, Matrix a)
{
     int res;
     if (a == NULL)
	  return 0;
     if (depth == BLOCK_DEPTH)
	  return 1;

     depth--;
     res = 0;
     res += num_blocks(depth, a->child[_00]);
     res += num_blocks(depth, a->child[_01]);
     res += num_blocks(depth, a->child[_10]);
     res += num_blocks(depth, a->child[_11]);
     return res;
}

/*
 * Count number of nonzeros in matrix
 */
int num_nonzeros(int depth, Matrix a)
{
     int res;
     if (a == NULL)
	  return 0;
     if (depth == BLOCK_DEPTH) {
	  LeafNode *A = (LeafNode *) a;
	  int i, j;
	  res = 0;
	  for (i = 0; i < BLOCK_SIZE; i++) {
	       for (j = 0; j < BLOCK_SIZE; j++) {
		    if (BLOCK(A->block, i, j) != 0.0)
			 res++;
	       }
	  }
	  return res;
     }
     depth--;
     res = 0;
     res += num_nonzeros(depth, a->child[_00]);
     res += num_nonzeros(depth, a->child[_01]);
     res += num_nonzeros(depth, a->child[_10]);
     res += num_nonzeros(depth, a->child[_11]);
     return res;
}

/*
 * Compute sum of squares of elements of matrix
 */
Real mag(int depth, Matrix a)
{
     Real res = 0.0;
     if (!a)
	  return res;

     if (depth == BLOCK_DEPTH) {
	  LeafNode *A = (LeafNode *) a;
	  int i, j;
	  for (i = 0; i < BLOCK_SIZE; i++)
	       for (j = 0; j < BLOCK_SIZE; j++)
		    res += BLOCK(A->block, i, j) * BLOCK(A->block, i, j);
     } else {
	  depth--;
	  res += mag(depth, a->child[_00]);
	  res += mag(depth, a->child[_01]);
	  res += mag(depth, a->child[_10]);
	  res += mag(depth, a->child[_11]);
     }
     return res;
}

/*************************************************************\
 * Cholesky algorithm
\*************************************************************/

/*
 * Perform R -= A * Transpose(B)
 * if lower==1, update only lower-triangular part of R
 */
Matrix mul_and_subT(int depth, int lower, Matrix a, Matrix b, Matrix r)
{
     assert(a != NULL && b != NULL);

     if (depth == BLOCK_DEPTH) {
	  LeafNode *A = (LeafNode *) a;
	  LeafNode *B = (LeafNode *) b;
	  LeafNode *R;

	  if (r == NULL) {
	       r = new_block_leaf();
	       R = (LeafNode *) r;
	       block_zero(R->block);
	  } else
	       R = (LeafNode *) r;

	  if (lower)
	       block_schur_half(R->block, A->block, B->block);
	  else
	       block_schur_full(R->block, A->block, B->block);
     } else {
	  Matrix r00, r01, r10, r11;

	  depth--;

	  if (r != NULL) {
	       r00 = r->child[_00];
	       r01 = r->child[_01];
	       r10 = r->child[_10];
	       r11 = r->child[_11];
	  } else {
	       r00 = NULL;
	       r01 = NULL;
	       r10 = NULL;
	       r11 = NULL;
	  }

	  if (a->child[_00] && b->child[TR_00]) {
#if defined(__CILKPLUS__)
		  r00 = cilk_spawn mul_and_subT(depth, lower, a->child[_00], b->child[TR_00], r00);
#elif defined(__OPENMP__)
#pragma omp task shared(r00, depth, lower, a, b)
		  r00 = mul_and_subT(depth, lower, a->child[_00], b->child[TR_00], r00);
#endif
	  }

	  if (!lower && a->child[_00] && b->child[TR_01]) {
#if defined(__CILKPLUS__)
		  r01 = cilk_spawn mul_and_subT(depth, 0, a->child[_00], b->child[TR_01], r01);
#elif defined(__OPENMP__)
#pragma omp task shared(r01, depth, a, b)
		  r01 = mul_and_subT(depth, 0, a->child[_00], b->child[TR_01], r01);
#endif
	  }

	  if (a->child[_10] && b->child[TR_00]) {
#if defined (__CILKPLUS__)
		  r10 = cilk_spawn mul_and_subT(depth, 0, a->child[_10], b->child[TR_00], r10);
#elif defined(__OPENMP__)
#pragma omp task shared(r10, depth, a, b)
		  r10 = mul_and_subT(depth, 0, a->child[_10], b->child[TR_00], r10);
#endif
	  }

	  if (a->child[_10] && b->child[TR_01]) {
#if defined(__CILKPLUS__)
	       r11 = mul_and_subT(depth, lower, a->child[_10], b->child[TR_01],	r11);
		   //cilk_sync;
#elif defined(__OPENMP__)
#pragma omp task shared(r11, depth, lower, a, b)
		   r11 = mul_and_subT(depth, lower, a->child[_10], b->child[TR_01],	r11);
		   //#pragma omp taskwait
#endif
	  }

#if defined(__CILKPLUS__)
	  cilk_sync;
#elif defined(__OPENMP__)
#pragma omp taskwait
#endif


	  if (a->child[_01] && b->child[TR_10]) {
#if defined(__CILKPLUS__)
	       r00 = cilk_spawn mul_and_subT(depth, lower, a->child[_01], b->child[TR_10], r00);
#elif defined(__OPENMP__)
#pragma omp task shared(r00, depth, lower, a, b)
		   r00 = mul_and_subT(depth, lower, a->child[_01], b->child[TR_10], r00);
#endif
	  }

	  if (!lower && a->child[_01] && b->child[TR_11]) {
#if defined(__CILKPLUS__)
	       r01 = cilk_spawn mul_and_subT(depth, 0, a->child[_01], b->child[TR_11], r01);
#elif defined(__OPENMP__)
#pragma omp task shared(r01, depth, a, b)
		   r01 = mul_and_subT(depth, 0, a->child[_01], b->child[TR_11], r01);
#endif
	  }

	  if (a->child[_11] && b->child[TR_10]) {
#if defined(__CILKPLUS__)
	       r10 = cilk_spawn mul_and_subT(depth, 0, a->child[_11], b->child[TR_10], r10);
#elif defined(__OPENMP__)
#pragma omp task shared(r10, depth, a, b)
		   r10 = mul_and_subT(depth, 0, a->child[_11], b->child[TR_10], r10);
#endif
	  }

	  if (a->child[_11] && b->child[TR_11]) {
#if defined(__CILKPLUS__)
	       r11 = mul_and_subT(depth, lower,	a->child[_11], b->child[TR_11],	r11);
		   //cilk_sync;
#elif defined(__OPENMP__)
#pragma omp task shared(r11, depth, lower, a, b)
		   r11 = mul_and_subT(depth, lower,	a->child[_11], b->child[TR_11],	r11);
		   //#pragma omp taskwait
#endif
	  }

#if defined(__CILKPLUS__)
	  cilk_sync;
#elif defined(__OPENMP__)
#pragma omp taskwait
#endif

	  if (r == NULL) {
	       if (r00 || r01 || r10 || r11)
		    r = new_internal(r00, r01, r10, r11);
	  } else {
	       assert(r->child[_00] == NULL || r->child[_00] == r00);
	       assert(r->child[_01] == NULL || r->child[_01] == r01);
	       assert(r->child[_10] == NULL || r->child[_10] == r10);
	       assert(r->child[_11] == NULL || r->child[_11] == r11);
	       r->child[_00] = r00;
	       r->child[_01] = r01;
	       r->child[_10] = r10;
	       r->child[_11] = r11;
	  }
     }
     return r;
}

/*
 * Perform substitution to solve for B in BL = A
 * Returns B in place of A.
 */
Matrix backsub(int depth, Matrix a, Matrix l)
{
     assert(a != NULL);
     assert(l != NULL);

     if (depth == BLOCK_DEPTH) {
	  LeafNode *A = (LeafNode *) a;
	  LeafNode *L = (LeafNode *) l;
	  block_backsub(A->block, L->block);
     } else {
	  Matrix a00, a01, a10, a11;
	  Matrix l00, l10, l11;

	  depth--;

	  a00 = a->child[_00];
	  a01 = a->child[_01];
	  a10 = a->child[_10];
	  a11 = a->child[_11];

	  l00 = l->child[_00];
	  l10 = l->child[_10];
	  l11 = l->child[_11];

	  assert(l00 && l11);

	  if (a00) {
#if defined(__CILKPLUS__)
	       a00 = cilk_spawn backsub(depth, a00, l00);
#elif defined(__OPENMP__)
#pragma omp task shared(a00, depth)
		   a00 = backsub(depth, a00, l00);
#endif
	  }

	  if (a10) {
#if defined(__CILKPLUS__)
		  a10 = backsub(depth, a10, l00);
		  //cilk_sync;
#elif defined(__OPENMP__)
#pragma omp task shared(a10, depth)
		  a10 = backsub(depth, a10, l00);
		  //#pragma omp taskwait
#endif
	  }

#if defined(__CILKPLUS__)
	  cilk_sync;
#elif defined(__OPENMP__)
#pragma omp taskwait
#endif	  


	  if (a00 && l10) {
#if defined(__CILKPLUS__)
		  a01 = cilk_spawn mul_and_subT(depth, 0, a00, l10, a01);
#elif defined(__OPENMP__)
#pragma omp task shared(a01, depth, a00, l10)
		  a01 = mul_and_subT(depth, 0, a00, l10, a01);
#endif
	  }
	  
	  if (a10 && l10) {
#if defined(__CILKPLUS__)
		  a11 = mul_and_subT(depth, 0, a10, l10, a11);
		  //cilk_sync;
#elif defined(__OPENMP__)
#pragma omp task shared(a11, depth, a10, l10)
		  a11 = mul_and_subT(depth, 0, a10, l10, a11);
		  //#pragma omp taskwait
#endif
	  }

#if defined(__CILKPLUS__)
	  cilk_sync;
#elif defined(__OPENMP__)
#pragma omp taskwait
#endif	  
	  
	  if (a01) {
#if defined(__CILKPLUS__)
		  a01 = cilk_spawn backsub(depth, a01, l11);
#elif defined(__OPENMP__)
#pragma omp task shared(a01, depth, l11)
		  a01 = backsub(depth, a01, l11);
#endif
	  }
	  
	  if (a11) {
#if defined(__CILKPLUS__)
		  a11 = backsub(depth, a11, l11);
		  //cilk_sync;
#elif defined(__OPENMP__)
#pragma omp task shared(a11, l11, depth)
		  a11 = backsub(depth, a11, l11);
		  //#pragma omp taskwait
#endif
	  }

#if defined(__CILKPLUS__)
	  cilk_sync;
#elif defined(__OPENMP__)
#pragma omp taskwait
#endif	  

	  a->child[_00] = a00;
	  a->child[_01] = a01;
	  a->child[_10] = a10;
	  a->child[_11] = a11;
     }

     return a;
}

/*
 * Compute Cholesky factorization of A.
 */
Matrix cholesky(int depth, Matrix a)
{
	assert(a != NULL);
	
	if (depth == BLOCK_DEPTH) {
		LeafNode *A = (LeafNode *) a;
		block_cholesky(A->block);
	} else {
		Matrix a00, a10, a11;
		
		depth--;
		
		a00 = a->child[_00];
		a10 = a->child[_10];
		a11 = a->child[_11];
		
		assert(a00);
		
		if (!a10) {
			assert(a11);
#if defined(__CILKPLUS__)
			a00 = cilk_spawn cholesky(depth, a00);
			a11 = cholesky(depth, a11);
			cilk_sync;
#elif defined(__OPENMP__)
#pragma omp task shared(a00, depth)
			a00 = cholesky(depth, a00);
#pragma omp task shared(a11, depth)
			a11 = cholesky(depth, a11);
#pragma omp taskwait
#endif
		} else {
			a00 = cholesky(depth, a00);
			assert(a00);
			
			a10 = backsub(depth, a10, a00);
			assert(a10);
			
			a11 = mul_and_subT(depth, 1, a10, a10, a11);
			assert(a11);
			
			a11 = cholesky(depth, a11);
			assert(a11);
		}
		a->child[_00] = a00;
		a->child[_10] = a10;
		a->child[_11] = a11;
	}
	return a;
}

int logarithm(int size)
{
     int k = 0;

     while ((1 << k) < size)
	  k++;
     return k;
}

int usage(void)
{
     fprintf(stderr, 
	     "\nUsage: cholesky [<cilk-options>] [-n size] [-z nonzeros]\n"
     "                [-f filename] [-benchmark] [-h]\n\n"
     "Default: cholesky -n 500 -z 1000\n\n"
     "This program performs a divide and conquer Cholesky factorization of a\n"
     "sparse symmetric positive definite matrix (A=LL^T).  Using the fact\n"
     "that the matrix is symmetric, Cholesky does half the number of\n"
     "operations of LU.  The method used is the same as with LU, with work\n"
     "Theta(n^3) and critical path Theta(n lg(n)) for the dense case.  A\n"
     );fprintf(stderr,	                                                               /* break the string into smaller pieces.  ISO requires C89 compilers to support at least 509-characterstrings */
     "quad-tree is used to store the nonzero entries of the sparse\n"
     "matrix. Actual work and critical path are influenced by the sparsity\n"
     "pattern of the matrix.\n\n"
     "The input matrix is either read from the provided file or generated\n"
     "randomly with size and nonzero-elements as specified.\n\n");
     return 1;
}

char *specifiers[] =
{"n", "-z", "-f", "-benchmark", "-h", 0};
int opt_types[] =
{INTARG, INTARG, STRINGARG, BENCHMARK, BOOLARG, 0};


// Set of global variables
Matrix A, R;
int size, depth, nonzeros, i, benchmark, help;
int input_nonzeros, input_blocks, output_nonzeros, output_blocks;
Real error;
char buf[1000], filename[100];
int sizex, sizey;
FILE *f;


// Process command-line arguments:
// - Read the matrix from file if presented
// - Or generate a random matrix with a given size
// And setup CilkPlus or OpenMP environment
int init(int argc, char *argv[]) {

	// At least the program name & number of cores
	if (argc < 2) {
		printf("At least indicate the number of cores to run !\n");
		return -1;
	}
	
	// Get the number of threads for Cilk and OMP
	if (atoi(argv[1]) < 1 || atoi(argv[1]) > 32) {
		printf("Number of cors must be from 1 to 32 !\n");
		return -1;
	}

	char *num_cores = argv[1];
	
     A = NULL;

     /* standard benchmark options */
     filename[0] = 0;
     size = 1000;
     nonzeros = 10000;

	 // The first argument is the number of threads
	 // The original arguments start from the second one
     get_options(argc-1, argv+1, specifiers, opt_types,
		 &size, &nonzeros, filename, &benchmark, &help);

     if (help)
		 return usage();

     if (benchmark) {
		 switch (benchmark) {
		 case 1:		/* short benchmark options -- a little work */
			 filename[0] = 0;
			 size = 128;
			 nonzeros = 100;
			 break;
		 case 2:		/* standard benchmark options */
		   filename[0] = 0;
		   size = 1000;
		   nonzeros = 10000;
		   break;
	      case 3:		/* long benchmark options -- a lot of work */
		   filename[0] = 0;
		   size = 2000;
		   nonzeros = 10000;
		   break;
		 }
     }
     if (filename[0]) {
		 f = fopen(filename, "r");
		 if (f == NULL) {
			 printf("\nFile not found!\n\n");
			 return 1;
		 }
		 /* throw away comment lines */
		 do
			 fgets(buf, 1000, f);
		 while (buf[0] == '%');
		 
		 sscanf(buf, "%d %d", &sizex, &sizey);
		 assert(sizex == sizey);
		 size = sizex;
		 
		 depth = logarithm(size);
		 
		 srand(61066);
		 nonzeros = 0;
		 
		 while (!feof(f)) {
			 double fr, fc;
			 int r, c;
			 Real val;
			 int res;
			 
			 fgets(buf, 1000, f);
			 
			 res = sscanf(buf, "%lf %lf %lf", &fr, &fc, &val);
			 r = fr;
			 c = fc;
			 if (res <= 0)
				 break;
			 /*
			  * some Matrix Market Matrices have no values, only
			  * patterns. Then generate values randomly with a
			  * nice big fat diagonal for Cholesky
			  */
			 if (res == 2) {
				 double rnd = ((double)rand()) / (double)RAND_MAX;
				 val = (r == c ? 100000.0 * rnd : rnd);
			 }

			 r--;
			 c--;
			 if (r < c) {
				 int t = r;
				 r = c;
				 c = t;
			 }
			 assert(r >= c);
			 assert(r < size);
			 assert(c < size);
			 A = set_matrix(depth, A, r, c, val);
			 nonzeros++;
		 }
     } else {
		 /* generate random matrix */
		 
		 depth = logarithm(size);
		 
		 /* diagonal elements */
		 for (i = 0; i < size; i++)
			 A = set_matrix(depth, A, i, i, 1.0);
		 
		 /* off-diagonal elements */
		 for (i = 0; i < nonzeros - size; i++) {
			 int r, c;
	     again:
			 r = Cilk_rand() % size;
			 c = Cilk_rand() % size;
			 if (r <= c)
				 goto again;
			 if (get_matrix(depth, A, r, c) != 0.0)
				 goto again;
			 A = set_matrix(depth, A, r, c, 0.1);
		 }
     }
	 
     /* extend to power of two size with identity matrix */
     for (i = size; i < (1 << depth); i++) {
		 A = set_matrix(depth, A, i, i, 1.0);
     }

     R = copy_matrix(depth, A);


	 // Bind the task to a set of cores &
	 // Setup runtime environment
	 int cores = atoi(num_cores);
	cpu_set_t mask;
	CPU_ZERO(&mask);
	for (int i=0; i<cores; i++) {
		CPU_SET(i, &mask);
	}

	int ret = sched_setaffinity(getpid(), sizeof(mask), &mask);
	if (ret != 0) {
		printf("Set affinity failed !\n");
		return -1;
	}

#if defined(__CILKPLUS__)
	__cilkrts_end_cilk();
	__cilkrts_set_param("nworkers", num_cores);
#elif defined(__OPENMP__)
	 omp_set_dynamic(0);
	 omp_set_num_threads(atoi(num_cores));
#endif

	 return 0;
}

// Run the Cholesky function multiple times
#define NUM_ITERS 1//00//0
#define USEC_IN_SEC 1000000

// The work goes here
// Before cholesky() is called, R is a copy of 
// the input matrix A. After the call, R is the 
// factorized matrix of A.
int run(int argc, char *argv[]) {

	// Open output file
	std::string data_folder = "/export/shakespeare/home/sonndinh/codes/fss_rtss16_code/dag_benchmarks/cholesky/data2/";
	std::stringstream ss;

#if defined(__CILKPLUS__)
	ss << data_folder << "cilk_" << "n=" << size << "_" << argv[1] << "cores.dat";
#elif defined(__OPENMP__)
	ss << data_folder << "omp_" << "n=" << size << "_" << argv[1] << "cores.dat";
#endif

	std::ofstream ofs(ss.str().c_str());

	//	input_blocks = num_blocks(depth, R);
	//	input_nonzeros = num_nonzeros(depth, R);
	
	/* Timing. "Start" timers */
	//     cp_begin = Cilk_user_critical_path;
	//     wk_begin = Cilk_user_work;
	//     tm_begin = Cilk_get_wall_time();


	timespec start, end, diff;

	for (int i=0; i<NUM_ITERS; i++) {

		clock_gettime(CLOCK_MONOTONIC, &start);

#if defined(__CILKPLUS__)
		
		//	printf("Number of Cilk workers: %d\n", __cilkrts_get_nworkers());
		R = cholesky(depth, R);
		
#elif defined(__OPENMP__)
		
#pragma omp parallel shared(R)
		{
			//	printf("Number of OMP threads: %d\n", omp_get_num_threads());
#pragma omp single
			R = cholesky(depth, R);
		}
		
#endif

		clock_gettime(CLOCK_MONOTONIC, &end);
		
		// Take elapsed time
		ts_diff(start, end, diff);

		// Write to output file
		ofs << (diff.tv_sec * USEC_IN_SEC + diff.tv_nsec/1000) << std::endl;
		
		//		std::cout << (diff.tv_sec * USEC_IN_SEC + diff.tv_nsec/1000) << std::endl;

		// Free matrix R and make a new copy for the next iteration
		// Note that we are not recording time for this part of code
		// --- we only record timings of the real Cholesky computations
		free_matrix(depth, R);

		// Matrix R of the last iteration will be freed by finalize()
		R = copy_matrix(depth, A);
		
	}
     /* Timing. "Stop" timers */
	 //     tm_elapsed = Cilk_get_wall_time() - tm_begin;
	 //     wk_elapsed = Cilk_user_work - wk_begin;
	 //     cp_elapsed = Cilk_user_critical_path - cp_begin;

	//	output_blocks = num_blocks(depth, R);
	//	output_nonzeros = num_nonzeros(depth, R);

	ofs.close();

	return 0;
}


// Clean up memory and stuff
// We abort the error checking and printing out the resutls
// because we don't need these things
int finalize(int argc, char *argv[]) {

	/*
	A = mul_and_subT(depth, 1, R, R, A);
	
	error = mag(depth, A);

     printf("\nCilk Example: cholesky\n");
	 //     printf("	      running on %d processor%s\n\n",
	 //	    Cilk_active_size, Cilk_active_size > 1 ? "s" : "");
     printf("Error: %f\n\n", error);
     printf("Options: original size     = %d\n", size);
     printf("         original nonzeros = %d\n", nonzeros);
     printf("         input nonzeros    = %d\n", input_nonzeros);
     printf("         input blocks      = %d\n", input_blocks);
     printf("         output nonzeros   = %d\n", output_nonzeros);
     printf("         output blocks     = %d\n\n", output_blocks);
	 //     printf("Running time  = %4f s\n", Cilk_wall_time_to_sec(tm_elapsed));
	 //     printf("Work          = %4f s\n", Cilk_time_to_sec(wk_elapsed));
	 //     printf("Critical path = %4f s\n\n", Cilk_time_to_sec(cp_elapsed));
	 */

     free_matrix(depth, A);
     free_matrix(depth, R);

	 return 0;
}


// The main function can call run() multiple times
int main(int argc, char *argv[]) {

	// Record the whole elapsed time
	//	timespec start, end, diff;

	int ret;

	//	cilk_tool_init();
	//	clock_gettime(CLOCK_MONOTONIC, &start);
	ret = init(argc, argv);

	if (ret != 0)
		return ret;

	// Run actually run Cholesky computation multiple times
#if defined(__CILKPROF__)
	cilk_tool_init();
#elif defined(__CILKVIEW__)
	cilkview_data_t start_data;
	__cilkview_query(start_data);
	/*
	if (start_data.status != 1) {
		printf("Initialize cilkview failed!\n");
	}
	*/
#endif

	ret = run(argc, argv);
#if defined(__CILKPROF__)
	cilk_tool_print();
	cilk_tool_destroy();
#elif defined(__CILKVIEW__)
	__cilkview_report(&start_data, NULL, "test_cholesky", CV_REPORT_WRITE_TO_LOG | CV_REPORT_WRITE_TO_RESULTS);
#endif

	if (ret != 0)
		return ret;

	ret = finalize(argc, argv);
	//	cilk_tool_print();
	//	cilk_tool_destroy();
	//	clock_gettime(CLOCK_MONOTONIC, &end);

	//	ts_diff(start, end, diff);

	/*
	// Append the whole elapsed time to the recording file
	std::string data_folder = "/export/shakespeare/home/sonndinh/codes/fss_rtss16_code/dag_benchmarks/cholesky/data2/";
	std::stringstream ss;
	
#if defined(__CILKPLUS__)
	ss << data_folder << "cilk_" << "n=" << size << "_" << argv[1] << "cores.dat";
#elif defined(__OPENMP__)
	ss << data_folder << "omp_" << "n=" << size << "_" << argv[1] << "cores.dat";
#endif
	
	std::ofstream ofs(ss.str().c_str(), std::ios_base::app);
	ofs << (diff.tv_sec * USEC_IN_SEC + diff.tv_nsec/1000) << std::endl;
	ofs.close();
	*/

	//	std::cout << (diff.tv_sec * USEC_IN_SEC + diff.tv_nsec/1000) << std::endl;	

	return ret;
}



////////////////////////////////////////////////////
// Comment out the original main function
////////////////////////////////////////////////////
#if 0
int main(int argc, char *argv[])
{
     Matrix A, R;
     int size, depth, nonzeros, i, benchmark, help;
     int input_nonzeros, input_blocks, output_nonzeros, output_blocks;
	 //     Cilk_time tm_begin, tm_elapsed;
	 //     Cilk_time wk_begin, wk_elapsed;
	 //     Cilk_time cp_begin, cp_elapsed;
     Real error;
     char buf[1000], filename[100];
     int sizex, sizey;
     FILE *f;

     A = NULL;

     /* standard benchmark options */
     filename[0] = 0;
     size = 1000;
     nonzeros = 10000;

     get_options(argc, argv, specifiers, opt_types,
		 &size, &nonzeros, filename, &benchmark, &help);

     if (help)
		 return usage();

     if (benchmark) {
		 switch (benchmark) {
		 case 1:		/* short benchmark options -- a little work */
			 filename[0] = 0;
			 size = 128;
			 nonzeros = 100;
			 break;
		 case 2:		/* standard benchmark options */
		   filename[0] = 0;
		   size = 1000;
		   nonzeros = 10000;
		   break;
	      case 3:		/* long benchmark options -- a lot of work */
		   filename[0] = 0;
		   size = 2000;
		   nonzeros = 10000;
		   break;
		 }
     }
     if (filename[0]) {
		 f = fopen(filename, "r");
		 if (f == NULL) {
			 printf("\nFile not found!\n\n");
			 return 1;
		 }
		 /* throw away comment lines */
		 do
			 fgets(buf, 1000, f);
		 while (buf[0] == '%');
		 
		 sscanf(buf, "%d %d", &sizex, &sizey);
		 assert(sizex == sizey);
		 size = sizex;
		 
		 depth = logarithm(size);
		 
		 srand(61066);
		 nonzeros = 0;
		 
		 while (!feof(f)) {
			 double fr, fc;
			 int r, c;
			 Real val;
			 int res;
			 
			 fgets(buf, 1000, f);
			 
			 res = sscanf(buf, "%lf %lf %lf", &fr, &fc, &val);
			 r = fr;
			 c = fc;
			 if (res <= 0)
				 break;
			 /*
			  * some Matrix Market Matrices have no values, only
			  * patterns. Then generate values randomly with a
			  * nice big fat diagonal for Cholesky
			  */
			 if (res == 2) {
				 double rnd = ((double)rand()) / (double)RAND_MAX;
				 val = (r == c ? 100000.0 * rnd : rnd);
			 }

			 r--;
			 c--;
			 if (r < c) {
				 int t = r;
				 r = c;
				 c = t;
			 }
			 assert(r >= c);
			 assert(r < size);
			 assert(c < size);
			 A = set_matrix(depth, A, r, c, val);
			 nonzeros++;
		 }
     } else {
		 /* generate random matrix */
		 
		 depth = logarithm(size);
		 
		 /* diagonal elements */
		 for (i = 0; i < size; i++)
			 A = set_matrix(depth, A, i, i, 1.0);
		 
		 /* off-diagonal elements */
		 for (i = 0; i < nonzeros - size; i++) {
			 int r, c;
	     again:
			 r = Cilk_rand() % size;
			 c = Cilk_rand() % size;
			 if (r <= c)
				 goto again;
			 if (get_matrix(depth, A, r, c) != 0.0)
				 goto again;
			 A = set_matrix(depth, A, r, c, 0.1);
		 }
     }
	 
     /* extend to power of two size with identity matrix */
     for (i = size; i < (1 << depth); i++) {
		 A = set_matrix(depth, A, i, i, 1.0);
     }
	 
     R = copy_matrix(depth, A);
	 
     input_blocks = num_blocks(depth, R);
     input_nonzeros = num_nonzeros(depth, R);
	 
     /* Timing. "Start" timers */
	 //     cp_begin = Cilk_user_critical_path;
	 //     wk_begin = Cilk_user_work;
	 //     tm_begin = Cilk_get_wall_time();
	 
     R = cholesky(depth, R);
	 
     /* Timing. "Stop" timers */
	 //     tm_elapsed = Cilk_get_wall_time() - tm_begin;
	 //     wk_elapsed = Cilk_user_work - wk_begin;
	 //     cp_elapsed = Cilk_user_critical_path - cp_begin;

     output_blocks = num_blocks(depth, R);
     output_nonzeros = num_nonzeros(depth, R);

     /* test - make sure R * Transpose(R) == A */
     /* compute || A - R * Transpose(R) ||    */

     A = mul_and_subT(depth, 1, R, R, A);

     error = mag(depth, A);

     printf("\nCilk Example: cholesky\n");
	 //     printf("	      running on %d processor%s\n\n",
	 //	    Cilk_active_size, Cilk_active_size > 1 ? "s" : "");
     printf("Error: %f\n\n", error);
     printf("Options: original size     = %d\n", size);
     printf("         original nonzeros = %d\n", nonzeros);
     printf("         input nonzeros    = %d\n", input_nonzeros);
     printf("         input blocks      = %d\n", input_blocks);
     printf("         output nonzeros   = %d\n", output_nonzeros);
     printf("         output blocks     = %d\n\n", output_blocks);
	 //     printf("Running time  = %4f s\n", Cilk_wall_time_to_sec(tm_elapsed));
	 //     printf("Work          = %4f s\n", Cilk_time_to_sec(wk_elapsed));
	 //     printf("Critical path = %4f s\n\n", Cilk_time_to_sec(cp_elapsed));

     free_matrix(depth, A);
     free_matrix(depth, R);

     return 0;
}
#endif // ORIGINAL MAIN FUNCTION
