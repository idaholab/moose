//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseInit.h"
#include "ParallelUniqueId.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Executioner.h"
#include "MooseRandom.h"

#include "libmesh/petsc_solver_exception.h"
#include "libmesh/reference_counter.h"

// PETSc
#include "petscsys.h"

#ifdef LIBMESH_HAVE_OPENMP
#include <omp.h>
#endif

#ifdef MOOSE_LIBTORCH_ENABLED
#include <ATen/Parallel.h>
#endif

#include <unistd.h>
#include <signal.h>
#include <charconv>

#ifdef PETSC_HAVE_CUDA
#include "cuda_runtime.h"
#endif

void
SigHandler(int signum)
{
  Moose::interrupt_signal_number = signum;
  return;
}

void
RegisterSigHandler()
{
  signal(SIGUSR1, SigHandler);
}

int
getLocalRank()
{
  static constexpr std::array local_rank_vars{
      "OMPI_COMM_WORLD_LOCAL_RANK", // Open MPI
      "MV2_COMM_WORLD_LOCAL_RANK",  // MVAPICH2
      "MPI_LOCALRANKID",            // Intel MPI / some MPICH environments
      "PMI_LOCAL_RANK",             // PMI-based launchers
      "PALS_LOCAL_RANKID",          // HPE Cray PALS
      "SLURM_LOCALID",              // Slurm
      "FLUX_TASK_LOCAL_ID"          // Flux
  };

  for (const char * variable : local_rank_vars)
  {
    const char * value = std::getenv(variable);
    if (!value)
      continue;

    int rank;
    const char * end = value + std::char_traits<char>::length(value);
    const auto [ptr, error] = std::from_chars(value, end, rank);

    if (error != std::errc{} || ptr != end || rank < 0)
      throw std::runtime_error(std::string("Invalid local rank in ") + variable + ": " + value);

    return rank;
  }

  // Not launched through a recognized MPI/process-manager environment.
  return 0;
}

#ifdef PETSC_HAVE_CUDA
void
initializeCuda()
{
  int local_rank = getLocalRank();

  /**
   * NOTE: Some GPU-aware MPI implementations initialize accelerator-related resources during
   * MPI_Init() and may associate those resources with the device that is current at that time.
   * Therefore, select the GPU device before MPI initialization.
   *
   * Open MPI explicitly recommends selecting the accelerator before MPI_Init(), using
   * OMPI_COMM_WORLD_LOCAL_RANK for per-node device assignment. In particular, we have observed that
   * failing to do so can prevent initialization of the smcuda BTL (the transfer component for fast
   * shared-memory GPU communication), causing GPU communication to fall back to a much slower PCI
   * path instead of using NVLink, with over 100x performance difference in some cases. MVAPICH2
   * also supports selecting the CUDA device before MPI_Init() using MV2_COMM_WORLD_LOCAL_RANK.
   * MPICH does not appear to require early device selection, but initializing the GPU runtime
   * before MPI_Init() is expected to be harmless.
   *
   * The mapping used here (local_rank % num_gpus) is intended to establish the device used by
   * Kokkos. It is not yet known whether this mapping is appropriate for other GPU packages
   * supported by MOOSE.
   *
   * TODO: Add HIP and SYCL device selection if needed.
   */
  int num_gpus = 0;
  cudaGetDeviceCount(&num_gpus);

  if (num_gpus)
  {
    int device = local_rank % num_gpus;
    cudaSetDevice(device);
  }
}
#endif

MooseDeviceInit::MooseDeviceInit()
{
#ifdef PETSC_HAVE_CUDA
  initializeCuda();
#endif
}

MooseInit::MooseInit(int argc, char * argv[], MPI_Comm COMM_WORLD_IN)
  : MooseDeviceInit(), LibMeshInit(argc, argv, COMM_WORLD_IN)
{
  LibmeshPetscCallA(COMM_WORLD_IN, PetscPopSignalHandler()); // get rid of PETSc error handler

// Set the number of OpenMP threads to the same as the number of threads libMesh is going to use
#ifdef LIBMESH_HAVE_OPENMP
  omp_set_num_threads(libMesh::n_threads());
#endif

#ifdef MOOSE_LIBTORCH_ENABLED
  at::set_num_threads(libMesh::n_threads());
  at::set_num_interop_threads(libMesh::n_threads());
#endif

  if (!libMesh::on_command_line("--enable-refcount-printing"))
    libMesh::ReferenceCounter::disable_print_counter_info();

  ParallelUniqueId::initialize();

  // Make sure that any calls to the global random number generator are consistent among processes
  MooseRandom::seed(0);

  RegisterSigHandler();

#ifdef MOOSE_KOKKOS_ENABLED
  initKokkos();
#endif
}
