/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseInit.h"
#include "ParallelUniqueId.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Executioner.h"
#include "MooseRandom.h"

// PETSc
#ifdef LIBMESH_HAVE_PETSC
#include "petscsys.h"
#endif

#ifdef LIBMESH_HAVE_OPENMP
#include <omp.h>
#endif

MooseInit::MooseInit(int argc, char *argv[], MPI_Comm COMM_WORLD_IN) :
    LibMeshInit(argc, argv, COMM_WORLD_IN)
{
#ifdef LIBMESH_HAVE_PETSC
  PetscPopSignalHandler();           // get rid of Petsc error handler
#endif

  // Set the number of OpenMP threads to the same as the number of threads libMesh is going to use
#ifdef LIBMESH_HAVE_OPENMP
  omp_set_num_threads(libMesh::n_threads());
#endif

  ParallelUniqueId::initialize();

  // Make sure that any calls to the global random number generator are consistent among processes
  MooseRandom::seed(0);
}

MooseInit::~MooseInit()
{}
