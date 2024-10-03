#include "outputs.h"

namespace platypus
{

Outputs::Outputs()
{
  MPI_Comm_size(_my_comm, &_n_ranks);
  MPI_Comm_rank(_my_comm, &_my_rank);
}

} // namespace platypus
