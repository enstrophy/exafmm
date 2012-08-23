#ifndef types_h
#define types_h
#ifdef __INTEL_COMPILER
#pragma warning(disable:193 383 444 981 1572 2259)
#endif

#include <map>
#include "parameters.h"
#include <pthread.h>
#include <queue>
#include <string>
#include <utility>
#include <vector>
#include "vec.h"

#if PAPI
#include <papi.h>
#endif

#if ASSERT
#include <cassert>
#else
#define assert(x)
#endif

typedef float         real_t;                                   //!< Floating point type
typedef vec<3,real_t> vec3;                                     //!< Vector of 3 floating point types
typedef vec<4,real_t> vec4;                                     //!< Vector of 4 floating point types
typedef vec<8,int>    ivec8;                                    //!< Vector of 8 integer types
typedef std::pair<vec3,vec3> vec3Pair;                          //!< Pair of vec3

//#ifndef KERNEL
namespace {
vec3 Xperiodic = .0;                                            //!< Coordinate offset of periodic image
#if PAPI
int PAPIEVENT  = PAPI_NULL;                                     //!< PAPI event handle
#endif
}

#if COMkernel
const int MTERM = P*(P+1)*(P+2)/6-3;                            //!< Number of Cartesian mutlipole terms
#else
const int MTERM = P*(P+1)*(P+2)/6;                              //!< Number of Cartesian mutlipole terms
#endif
const int LTERM = (P+1)*(P+2)*(P+3)/6;                          //!< Number of Cartesian local terms

typedef vec<MTERM,real_t>  vecM;                                //!< Multipole coefficient type for Cartesian
typedef vec<LTERM,real_t>  vecL;                                //!< Local coefficient type for Cartesian

//! Structure for pthread based trace
struct Trace {
  pthread_t thread;                                             //!< pthread id
  double    begin;                                              //!< Begin timer of trace
  double    end;                                                //!< End timer of trace
  int       color;                                              //!< Color of trace
};
typedef std::map<pthread_t,double>             ThreadTrace;     //!< Map of pthread id to traced value
typedef std::map<pthread_t,int>                ThreadMap;       //!< Map of pthread id to thread id
typedef std::queue<Trace>                      Traces;          //!< Queue of traces
typedef std::map<std::string,double>           Timer;           //!< Map of timer event name to timed value
typedef std::map<std::string,double>::iterator TI_iter;         //!< Iterator of timer event name map

//! Structure of bodies
struct Body {
  int    IBODY;                                                 //!< Initial body numbering for sorting back
  int    IPROC;                                                 //!< Initial process numbering for partitioning back
  int    ICELL;                                                 //!< Cell index
  vec3   X;                                                     //!< Position
  real_t SRC;                                                   //!< Scalar source values
  vec4   TRG;                                                   //!< Scalar+vector target values
};
typedef std::vector<Body>            Bodies;                    //!< Vector of bodies
typedef std::vector<Body>::iterator  B_iter;                    //!< Iterator of body vector

//! Structure of cells
struct Cell {
  int       NCHILD;                                             //!< Number of child cells
  int       NCLEAF;                                             //!< Number of child leafs
  int       NDLEAF;                                             //!< Number of descendant leafs
  int       PARENT;                                             //!< Index of parent cell
  int       CHILD;                                              //!< Index of child cells
  long long ICELL;                                              //!< Cell index
  B_iter    LEAF;                                               //!< Iterator of first leaf
  vec3      X;                                                  //!< Cell center
  real_t    R;                                                  //!< Cell radius
  real_t    RMAX;                                               //!< Max cell radius
  real_t    RCRIT;                                              //!< Critical cell radius
  vecM      M;                                                  //!< Multipole coefficients
  vecL      L;                                                  //!< Local coefficients
};
typedef std::vector<Cell>           Cells;                      //!< Vector of cells
typedef std::vector<Cell>::iterator C_iter;                     //!< Iterator of cell vector
typedef std::queue<C_iter>          CellQueue;                  //!< Queue of cell iterators

#endif
