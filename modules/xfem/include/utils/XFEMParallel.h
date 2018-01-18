/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMPARALLEL_H
#define XFEMPARALLEL_H

#include "Moose.h"
#include "libmesh/parallel.h"
#include <vector>

// Don't allow non-MPI builds for now. In the future a simple workaround would be a plain bufer
// copy.
#ifndef LIBMESH_HAVE_MPI
#error XFEM needs libMesh compiled with MPI.
#endif

namespace XFEMUtils
{
/**
 * Specialized replacement for the libMesh packed range communication that allows unlimited
 * string buffer sizes.
 * @param comm MPI communicator
 * @param send_buffer Buffer with data to be sent to all processors
 * @param recv_buffers Vector of buffers filled with data received from other processors
 */
void allgatherStringBuffers(const Parallel::Communicator & comm,
                            std::string & send_buffer,
                            std::vector<std::string> & recv_buffers);
}

#endif // XFEMPARALLEL_H
