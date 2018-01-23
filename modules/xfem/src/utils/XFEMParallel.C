/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMParallel.h"
#include "MooseError.h"

namespace XFEMUtils
{

void
allgatherStringBuffers(const Parallel::Communicator & comm,
                       std::string & send_buffer,
                       std::vector<std::string> & recv_buffers)
{
  // serial case
  if (comm.size() < 2)
  {
    recv_buffers.resize(1);
    recv_buffers[0] = send_buffer;
    return;
  }

  std::vector<int> sendlengths(comm.size(), 0);
  std::vector<int> displacements(comm.size(), 0);

  recv_buffers.assign(comm.size(), "");

  // first comm step to determine buffer sizes from all processors
  const int mysize = static_cast<int>(send_buffer.size());
  comm.allgather(mysize, sendlengths);

  // Find the total size of the final array and
  // set up the displacement offsets for each processor
  unsigned int globalsize = 0;
  for (unsigned int i = 0; i != comm.size(); ++i)
  {
    displacements[i] = globalsize;
    globalsize += sendlengths[i];
  }

  // Check for quick return
  if (globalsize == 0)
    return;

  // monolithic receive buffer
  std::string recv(globalsize, 0);

  // and get the data from the remote processors.
  libmesh_call_mpi(MPI_Allgatherv(reinterpret_cast<void *>(&send_buffer[0]),
                                  mysize,
                                  MPI_UNSIGNED_CHAR,
                                  reinterpret_cast<void *>(&recv[0]),
                                  sendlengths.data(),
                                  displacements.data(),
                                  MPI_UNSIGNED_CHAR,
                                  comm.get()));

  // slice receive buffer up
  for (unsigned int i = 0; i != comm.size(); ++i)
    recv_buffers[i] = recv.substr(displacements[i], sendlengths[i]);
}
}
