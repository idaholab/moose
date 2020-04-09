#include <mpi.h>
#include <iostream>

int main(int argc, char *argv[])
{
  MPI::Init();

  int size = MPI::COMM_WORLD.Get_size();
  int rank = MPI::COMM_WORLD.Get_rank();
  int len; char name[MPI_MAX_PROCESSOR_NAME];
  MPI::Get_processor_name(name, len);

  std::cout <<
    "Hello, World! " <<
    "I am process "  << rank <<
    " of "           << size <<
    " on  "          << name <<
    "."              << std::endl;

  MPI::Finalize();
  return 0;
}
