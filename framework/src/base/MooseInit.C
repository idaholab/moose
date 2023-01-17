//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

// PETSc
#include "petscsys.h"

#ifdef LIBMESH_HAVE_OPENMP
#include <omp.h>
#endif

#include <unistd.h>
#include <signal.h>

void
my_sig_handler(int signum)
{
  Moose::autosave_flag = signum;
  return;
}

void
register_sig_handler()
{
  signal(SIGUSR1, my_sig_handler);
}

MooseInit::MooseInit(int argc, char * argv[], MPI_Comm COMM_WORLD_IN)
  : LibMeshInit(argc, argv, COMM_WORLD_IN)
{
  PetscPopSignalHandler(); // get rid of PETSc error handler

// Set the number of OpenMP threads to the same as the number of threads libMesh is going to use
#ifdef LIBMESH_HAVE_OPENMP
  omp_set_num_threads(libMesh::n_threads());
#endif

  ParallelUniqueId::initialize();

  // Make sure that any calls to the global random number generator are consistent among processes
  MooseRandom::seed(0);

  register_sig_handler();
}
