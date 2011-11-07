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
#include "Moose.h"
#include "ParallelUniqueId.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "ProblemFactory.h"
#include "Executioner.h"

// PETSc
#ifdef LIBMESH_HAVE_PETSC
#include "petscsys.h"
#endif

MooseInit::MooseInit(int argc, char *argv[]) :
	LibMeshInit(argc, argv)
{
#ifdef LIBMESH_HAVE_PETSC
  PetscPopSignalHandler();           // get rid off Petsc error handler
#endif
  ParallelUniqueId::initialize();

  std::cout << "Using " << libMesh::n_threads() << " thread(s)" << std::endl;

  Moose::command_line = new GetPot(argc, argv);
  Moose::executioner = NULL;

  Moose::registerObjects();
}

MooseInit::~MooseInit()
{
  Moose::action_warehouse.clear();

  delete Moose::command_line;
  delete Moose::executioner;
  Factory::release();
  ActionFactory::release();
  ProblemFactory::release();
}

namespace Moose
{

GetPot *command_line = NULL;

}
