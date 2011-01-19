#include "bottomup.h"
#ifdef VTK
#include "vtk.h"
#endif

int main()
{
  double tic,toc;
  int const Nbody=1000000;
  tic = get_time();
  Bodies bodies(Nbody);
  Bodies bodies2(Nbody);
  BottomUpTreeConstructor T(bodies);
  toc = get_time();
  std::cout << "Allocate      : " << toc-tic << std::endl;

  tic = get_time();
  for( B_iter B=bodies.begin(); B!=bodies.end(); ++B ) {        // Loop over all bodies
    for( int d=0; d!=3; ++d )                                   //  Loop over each dimension
      B->pos[d] = rand()/(1.+RAND_MAX)*2-1;                     //   Initialize positions
    real r = sqrt(B->pos[0]*B->pos[0]+B->pos[1]*B->pos[1]+B->pos[2]*B->pos[2]);
    for( int d=0; d!=3; ++d )                                   //  Loop over each dimension
      B->pos[d] /= r;                                           //   Normalize positions
    B->scal = 1./bodies.size();                                 //  Initialize source value
  }                                                             // End loop over all bodies
  toc = get_time();
  std::cout << "Initialize    : " << toc-tic << std::endl;

  tic = get_time();
  T.setDomain();
  toc = get_time();
  std::cout << "Set domain    : " << toc-tic << std::endl;

  tic = get_time();
  T.setMorton();
  toc = get_time();
  std::cout << "Set Morton    : " << toc-tic << std::endl;

  tic = get_time();
  T.sort(T.Ibody,bodies,bodies2);
  toc = get_time();
  std::cout << "Sort Morton   : " << toc-tic << std::endl;

  bigint oldIndex(T.Ibody[0]);
  int b=0;
  for( B_iter B=bodies.begin(); B!=bodies.end(); ++B,++b ) {
    assert( oldIndex <= T.Ibody[b] );
    oldIndex = T.Ibody[b];
  }

#ifdef VTK
  int Ncell(0);
  vtkPlot vtk;
  vtk.setDomain(T.getR0(),T.getX0());
  vtk.setGroupOfPoints(T.Ibody,bodies,Ncell);
  vtk.plot(Ncell);
#endif
}
