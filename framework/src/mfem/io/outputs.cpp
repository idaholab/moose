#include "outputs.hpp"

namespace hephaestus
{

Outputs::Outputs()
{
  MPI_Comm_size(_my_comm, &_n_ranks);
  MPI_Comm_rank(_my_comm, &_my_rank);
}

Outputs::Outputs(hephaestus::GridFunctions & gridfunctions) : _gridfunctions(&gridfunctions)
{
  MPI_Comm_size(_my_comm, &_n_ranks);
  MPI_Comm_rank(_my_comm, &_my_rank);
}

Outputs::~Outputs()
{
  for (const auto & [name, socket_stream] : _socks)
  {
    delete socket_stream;
  }
}

} // namespace hephaestus
