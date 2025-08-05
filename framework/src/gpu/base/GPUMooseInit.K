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

#include "libmesh/parallel_implementation.h"
#include "libmesh/petsc_solver_exception.h"

PetscErrorCode
finalizeKokkos()
{
  PetscFunctionBegin;
  Kokkos::finalize();
  PetscFunctionReturn(PETSC_SUCCESS);
}

void
MooseInit::initKokkos()
{
  Kokkos::InitializationSettings settings;

#ifdef MOOSE_ENABLE_KOKKOS_GPU

  unsigned int num_kokkos_gpus = Kokkos::num_devices();

  comm().min(num_kokkos_gpus);

  if (num_kokkos_gpus)
  {
    // Create a local communicator defined at each shared memory node
    Parallel::Communicator local_comm;
    comm().split_by_type(MPI_COMM_TYPE_SHARED, comm().rank(), MPI_INFO_NULL, local_comm);

    // The number of processes in each node is usually larger than the number of
    // GPU devices in the node, so multiple processes share the same GPU
    unsigned int kokkos_gpu_id = local_comm.rank() % num_kokkos_gpus;

    // Override the default GPU ID
    settings.set_device_id(kokkos_gpu_id);
  }

#endif

  Kokkos::initialize(settings);

  LibmeshPetscCall(PetscRegisterFinalize(finalizeKokkos));
}
