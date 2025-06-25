//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUHeader.h"

#include "MooseInit.h"

#include "libmesh/petsc_solver_exception.h"

PetscErrorCode
finalizeGPUs()
{
  Kokkos::finalize();

  return PETSC_SUCCESS;
}

void
MooseInit::initGPUs()
{
  unsigned int num_gpus = Kokkos::num_devices();

  bool has_gpus = comm().verify(num_gpus > 0);

  if (!has_gpus)
    return;

  Kokkos::InitializationSettings settings;

  // Create a local communicator defined at each shared memory node
  Parallel::Communicator local_comm;
  comm().split_by_type(MPI_COMM_TYPE_SHARED, comm().rank(), MPI_INFO_NULL, local_comm);

  // The number of processes in each node is usually larger than the number of
  // GPU devices in the node, so multiple processes share the same GPU
  unsigned int my_gpu_id = local_comm.rank() % num_gpus;

  // Override the default GPU ID
  settings.set_device_id(my_gpu_id);

  Kokkos::initialize(settings);

  LibmeshPetscCall(PetscRegisterFinalize(finalizeGPUs));
}
