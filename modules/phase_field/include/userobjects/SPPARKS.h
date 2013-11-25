#include "mpi.h"

extern "C" {

  void spparks_open(int, char **, MPI_Comm, void **);
  void spparks_close(void *);
  void spparks_file(void *, char *);

  char * spparks_command(void *, char *);

  void *spparks_extract(void *, char *);
  double spparks_energy(void *);

}
