#ifndef types_h
#define types_h
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <iostream>
#include <stack>
#include <vector>
#include "vec.h"
#ifndef KERNEL
#include "gettime.h"
int MPIRANK = 0;                                                // MPI rank (for debugging serial class in MPI run)
int MPISIZE = 1;                                                // MPI size (for debugging serial class in MPI run)
#else
extern int MPIRANK;
extern int MPISIZE;
#endif
typedef long                 bigint;                            // Big integer type
typedef float                real;                              // Real number type
typedef std::complex<double> complex;                           // Complex number type

int  const P     = 3;                                           // Order of expansions
//int  const NCOEF = P*(P+1)*(P+2)/6;                             // Number of coefficients for Taylor expansion
int  const NCOEF = P*(P+1)/2;                                   // Number of coefficients for spherical harmonics
int  const NCRIT = 100;                                         // Number of bodies per cell
real const THETA = 0.5;                                         // Box opening criteria
real const EPS2  = 1e-4;                                        // Softening parameter

typedef vec<3,real>                   vect;                     // 3-D vector type
//typedef vec<NCOEF,real>               coef;                     // Multipole coefficient type for Taylor expansion
typedef vec<NCOEF,complex>            coef;                     // Multipole coefficient type for spherical harmonics
typedef std::vector<bigint>           Bigints;                  // Vector of big integer types
typedef std::vector<bigint>::iterator BI_iter;                  // Vector of big integer types

struct JBody {                                                  // Source properties of a body (stuff to send)
  bigint I;                                                     // Cell index
  vect   pos;                                                   // Position
  real   scal;                                                  // Mass/charge
};
struct Body : JBody {                                           // All properties of a body
  vect acc;                                                     // Acceleration
  real pot;                                                     // Potential
};
typedef std::vector<Body>             Bodies;                   // Vector of bodies
typedef std::vector<Body>::iterator   B_iter;                   // Iterator for body vector
typedef std::vector<JBody>            JBodies;                  // Vector of source bodies
typedef std::vector<JBody>::iterator  JB_iter;                  // Iterator for source body vector

struct JCell {                                                  // Source properties of a cell (stuff to send)
  bigint I;                                                     // Cell index
  coef   M;                                                     // Multipole coefficients
};
struct Cell : JCell {                                           // All properties of a cell
  int    NCHILD;                                                // Number of child cells
  int    NLEAF;                                                 // Number of leafs
  int    PARENT;                                                // Iterator offset of parent cell
  int    CHILD[8];                                              // Iterator offset of child cells
  B_iter LEAF;                                                  // Iterator of first leaf
  vect   X;                                                     // Cell center
  real   R;                                                     // Cell radius
  coef   L;                                                     // Local coefficients
};
typedef std::vector<Cell>             Cells;                    // Vector of cells
typedef std::vector<Cell>::iterator   C_iter;                   // Iterator for cell vector
typedef std::vector<JCell>            JCells;                   // Vector of source cells
typedef std::vector<JCell>::iterator  JC_iter;                  // Iterator for source cell vector

struct Pair {                                                   // Structure for pair of interacting cells
  C_iter CI;                                                    // Target cell iterator
  C_iter CJ;                                                    // Source cell iterator
  Pair(C_iter ci, C_iter cj) : CI(ci), CJ(cj) {}                // Constructor
};
typedef std::stack<Pair>              Pairs;                    // Stack of interacting cells

#endif
