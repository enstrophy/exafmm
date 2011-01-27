#include "partition.h"
#include "dataset.h"
#include "bottomup.h"
#ifdef VTK
#include "vtk.h"
#endif

int main() {
  double tic,toc;
  int const numBodies(1000000);
  tic = get_time();
  Bodies bodies(numBodies);
  Bodies bodies2(numBodies);
  BottomUpTreeConstructor T(bodies);
  Dataset D(bodies);
  Partition mpi(bodies);
  toc = get_time();
  mpi.print("Allocate      : ",0);
  mpi.print(toc-tic);

  tic = get_time();
  srand(mpi.rank()+1);
  D.random();
  toc = get_time();
  mpi.print("Set bodies    : ",0);
  mpi.print(toc-tic);

  tic = get_time();
  T.setDomain();
  mpi.setDomain();
  toc = get_time();
  mpi.print("Set domain    : ",0);
  mpi.print(toc-tic);

  tic = get_time();
  T.setMorton();
  mpi.binPosition(T.Ibody,0);
  toc = get_time();
  mpi.print("Set Index     : ",0);
  mpi.print(toc-tic);

  tic = get_time();
  T.sort(T.Ibody,bodies,bodies2);
  toc = get_time();
  mpi.print("Sort Index    : ",0);
  mpi.print(toc-tic);

  tic = get_time();
  bigint nth = numBodies * mpi.size() / 3;
  nth = mpi.nth_element(T.Ibody,numBodies,nth);
  toc = get_time();
  mpi.print("Nth element   : ",0);
  mpi.print(toc-tic);
  mpi.print(nth);
  mpi.print(T.Ibody,numBodies-10,numBodies);
  for( B_iter B=bodies.begin(); B!=bodies.end(); ++B )
    T.Ibody[B-bodies.begin()] = T.Ibody[B-bodies.begin()] > nth;

#ifdef VTK
  if( mpi.rank() == 0 ) {
    int Ncell(0);
    vtkPlot vtk;
    vtk.setDomain(T.getR0(),T.getX0());
    vtk.setGroupOfPoints(T.Ibody,bodies,Ncell);
    vtk.plot(Ncell);
  }
#endif
}
