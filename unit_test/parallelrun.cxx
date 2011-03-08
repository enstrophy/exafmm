#include "let.h"
#include "dataset.h"
#ifdef VTK
#include "vtk.h"
#endif

int main() {
  const int numBodies = 10000;
  Bodies bodies(numBodies);
  Cells cells;
  Dataset D;
  LocalEssentialTree T;
  Evaluator E;
  if( T.commRank() == 0 ) T.printNow = true;

  T.startTimer("Set bodies   ");
  D.sphere(bodies,T.commRank()+1);
  T.stopTimer("Set bodies   ",T.printNow);

  T.startTimer("Set domain   ");
  T.setGlobDomain(bodies);
  T.stopTimer("Set domain   ",T.printNow);

  T.bisection(bodies);

#ifdef TOPDOWN
  T.topdown(bodies,cells);
#else
  T.bottomup(bodies,cells);
#endif

  T.commBodies(cells);

  Bodies bodies2 = bodies;
  Cells jcells = cells;
  T.commCells(bodies2,jcells);

  T.downward(cells,jcells,1);

  T.startTimer("Direct sum   ");
  bodies2 = bodies;
  for( B_iter B=bodies2.begin(); B!=bodies2.end(); ++B ) {
    B->pot = -B->scal  / std::sqrt(EPS2);
  }
  for( int i=0; i!=T.commSize(); ++i ) {
    T.shiftBodies(bodies);
    E.evalP2P(bodies2,bodies);
    if(T.printNow) std::cout << "Direct loop   : " << i+1 << "/" << T.commSize() << std::endl;
  }
  T.stopTimer("Direct sum   ",T.printNow);

  B_iter B  = bodies.begin();
  B_iter B2 = bodies2.begin();
  real err = 0, rel = 0, err2, rel2;
  for( int i=0; i!=int(bodies.size()); ++i,++B,++B2 ) {
    B->pot -= B->scal  / std::sqrt(EPS2);
#ifdef DEBUG
    if(MPIRANK==0) std::cout << B->I << " " << B->pot << " " << B2->pot << std::endl;
#endif
    err += (B->pot - B2->pot) * (B->pot - B2->pot);
    rel += B2->pot * B2->pot;
  }
  MPI_Datatype MPI_TYPE = T.getType(err);
  MPI_Reduce(&err,&err2,1,MPI_TYPE,MPI_SUM,0,MPI_COMM_WORLD);
  MPI_Reduce(&rel,&rel2,1,MPI_TYPE,MPI_SUM,0,MPI_COMM_WORLD);
  if(T.printNow) std::cout << "Error         : " << std::sqrt(err2/rel2) << std::endl;
#ifdef DEBUG
  T.print(std::sqrt(err/rel));
#endif

#ifdef VTK
  for( B=bodies.begin(); B!=bodies.end(); ++B ) B->I = 0;
  for( C_iter C=jcells.begin(); C!=jcells.end(); ++C ) {
    Body body;
    body.I = 1;
    body.pos  = C->X;
    body.scal = 0;
    bodies.push_back(body);
  }

  int Ncell = 0;
  vtkPlot vtk;
  if( T.commRank() == 0 ) {
    vtk.setDomain(T.getR0(),T.getX0());
    vtk.setGroupOfPoints(bodies,Ncell);
  }
  for( int i=1; i!=T.commSize(); ++i ) {
    T.shiftBodies(bodies);
    if( T.commRank() == 0 ) {
      vtk.setGroupOfPoints(bodies,Ncell);
    }
  }
  if( T.commRank() == 0 ) {
    vtk.plot(Ncell);
  }
#endif
}
