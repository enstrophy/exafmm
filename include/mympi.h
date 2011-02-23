#ifndef mympi_h
#define mympi_h
#include <mpi.h>
#include <typeinfo>
#include "types.h"

class MyMPI {                                                   // My own MPI utilities
protected:
  const int WAIT;                                               // Waiting time between output of different ranks
  int       SIZE;                                               // Number of MPI processes
  int       RANK;                                               // Index of current MPI process
  int       SIZES;                                              // Number of MPI processes for split communicator
  int       RANKS;                                              // Index of current MPI process for split communicator
public:
  MyMPI() : WAIT(100) {                                         // Constructor, initialize WAIT time
    int argc(0);                                                // Dummy argument count
    char **argv;                                                // Dummy argument value
    MPI_Init(&argc,&argv);                                      // Initialize MPI communicator
    MPI_Comm_size(MPI_COMM_WORLD,&SIZE);                        // Get number of MPI processes
    MPI_Comm_rank(MPI_COMM_WORLD,&RANK);                        // Get index of current MPI process
    MPISIZE = SIZE;                                             // Set global variable MPISIZE
    MPIRANK = RANK;                                             // Set global variable MPIRANK
  }

  ~MyMPI() {                                                    // Destructor
    MPI_Finalize();                                             // Finalize MPI communicator
  }

  int commSize() { return SIZE; }                               // Number of MPI processes
  int commRank() { return RANK; }                               // Index of current MPI process

  bool isPowerOfTwo(const int n) {                              // If n is power of two return true
    return ((n != 0) && !(n & (n - 1)));                        // Decrement and compare bits
  }

  void splitRange(int &begin, int &end, int iSplit, int numSplit) {// Split range and return partial range
    int size = end - begin;                                     // Size of range
    int increment = size / numSplit;                            // Increment of splitting
    int remainder = size % numSplit;                            // Remainder of splitting
    begin += iSplit * increment + std::min(iSplit,remainder);   // Increment the begin counter
    end = begin + increment;                                    // Increment the end counter
    if( remainder > iSplit ) end++;                             // Adjust the end counter for remainder
  }

  template<typename T>
  int getType(T object) {                                       // Get MPI data type
    int type;                                                   // MPI data type
    if       ( typeid(object) == typeid(char) ) {               // If data type is char
      type = MPI_CHAR;                                          //  use MPI_CHAR
    } else if( typeid(object) == typeid(short) ) {              // If data type is short
      type = MPI_SHORT;                                         //  use MPI_SHORT
    } else if( typeid(object) == typeid(int) ) {                // If data type is int
      type = MPI_INT;                                           //  use MPI_INT
    } else if( typeid(object) == typeid(long) ) {               // If data type is long
      type = MPI_LONG;                                          //  use MPI_LONG
    } else if( typeid(object) == typeid(long long) ) {          // If data type is long long
      type = MPI_LONG_LONG;                                     //  use MPI_LONG_LONG
    } else if( typeid(object) == typeid(unsigned char) ) {      // If data type is unsigned char
      type = MPI_UNSIGNED_CHAR;                                 //  use MPI_UNSIGNED_CHAR
    } else if( typeid(object) == typeid(unsigned short) ) {     // If data type is unsigned short
      type = MPI_UNSIGNED_SHORT;                                //  use MPI_UNSIGNED_SHORT
    } else if( typeid(object) == typeid(unsigned int) ) {       // If data type is unsigned int
      type = MPI_UNSIGNED;                                      //  use MPI_UNSIGNED
    } else if( typeid(object) == typeid(unsigned long) ) {      // If data type is unsigned long
      type = MPI_UNSIGNED_LONG;                                 //  use MPI_UNSIGNED_LONG
    } else if( typeid(object) == typeid(unsigned long long) ) { // If data type is unsigned long long
      type = MPI_UNSIGNED_LONG_LONG;                            //  use MPI_UNSIGNED_LONG_LONG
    } else if( typeid(object) == typeid(float) ) {              // If data type is float
      type = MPI_FLOAT;                                         //  use MPI_FLOAT
    } else if( typeid(object) == typeid(double) ) {             // If data type is double
      type = MPI_DOUBLE;                                        //  use MPI_DOUBLE
    } else if( typeid(object) == typeid(long double) ) {        // If data type is long double
      type = MPI_LONG_DOUBLE;                                   //  use MPI_LONG_DOUBLE
    } else if( typeid(object) == typeid(std::complex<float>) ) {// If data type is complex<float>
      type = MPI_COMPLEX;                                       //  use MPI_COMPLEX
    } else if( typeid(object) == typeid(std::complex<double>) ) {// If data type is compelx<double>
      type = MPI_DOUBLE_COMPLEX;                                //  use MPI_DOUBLE_COMPLEX
    }                                                           // Endif for data type
    return type;                                                // Return MPI data type
  }

  template<typename T>
  void print(T data) {                                          // Print a scalar value on all ranks
    for( int irank=0; irank!=SIZE; ++irank ) {                  // Loop over ranks
      MPI_Barrier(MPI_COMM_WORLD);                              //  Sync processes
      usleep(WAIT);                                             //  Wait "WAIT" milliseconds
      if( RANK == irank ) std::cout << data << " ";             //  If it's my turn print "data"
    }                                                           // End loop over ranks
    MPI_Barrier(MPI_COMM_WORLD);                                // Sync processes
    usleep(WAIT);                                               // Wait "WAIT" milliseconds
    if( RANK == 0 ) std::cout << std::endl;                     // New line
  }

  template<typename T>
  void print(T data, const int irank) {                         // Print a scalar value on irank
    MPI_Barrier(MPI_COMM_WORLD);                                // Sync processes
    usleep(WAIT);                                               // Wait "WAIT" milliseconds
    if( RANK == irank ) std::cout << data;                      // If it's my rank print "data"
  }

  template<typename T>
  void print(T *data, const int begin, const int end) {         // Print a vector value on all ranks
    for( int irank=0; irank!=SIZE; ++irank ) {                  // Loop over ranks
      MPI_Barrier(MPI_COMM_WORLD);                              //  Sync processes
      usleep(WAIT);                                             //  Wait "WAIT" milliseconds
      if( RANK == irank ) {                                     //  If it's my turn to print
        std::cout << RANK << " : ";                             //   Print rank
        for( int i=begin; i!=end; ++i ) {                       //   Loop over data
          std::cout << data[i] << " ";                          //    Print data[i]
        }                                                       //   End loop over data
        std::cout << std::endl;                                 //   New line
      }                                                         //  Endif for my turn
    }                                                           // End loop over ranks
  }

  template<typename T>
  void print(T *data, const int begin, const int end, const int irank) {// Print a vector value on irank
    MPI_Barrier(MPI_COMM_WORLD);                                // Sync processes
    usleep(WAIT);                                               // Wait "WAIT" milliseconds
    if( RANK == irank ) {                                       // If it's my rank
      std::cout << RANK << " : ";                               //  Print rank
      for( int i=begin; i!=end; ++i ) {                         //  Loop over data
        std::cout << data[i] << " ";                            //   Print data[i]
      }                                                         //  End loop over data
      std::cout << std::endl;                                   //  New line
    }                                                           // Endif for my rank
  }

};

#endif
