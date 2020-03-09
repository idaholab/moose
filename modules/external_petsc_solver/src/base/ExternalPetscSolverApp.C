//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExternalPetscSolverApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "PETScDiffusionFDM.h"
#include "libmesh/petsc_vector.h"

InputParameters
ExternalPetscSolverApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // By default, use preset BCs
  params.set<bool>("use_legacy_dirichlet_bc") = false;
  return params;
}

ExternalPetscSolverApp::ExternalPetscSolverApp(InputParameters parameters) : MooseApp(parameters)
{
  ExternalPetscSolverApp::registerAll(_factory, _action_factory, _syntax);

#if LIBMESH_HAVE_PETSC
  PETScExternalSolverCreate(_comm->get(), &_ts);
  DM da;
  TSGetDM(_ts, &da);

  DMCreateGlobalVector(da, &_petsc_sol);

  // This is required because libMesh incorrectly treats the PETSc parallel vector as a ghost vector
  // We should be able to remove this line of code once libMesh is updated
  VecMPISetGhost(_petsc_sol, 0, nullptr);
#else
  mooseError("You need to have PETSc installed to use ExternalPETScApp");
#endif
}

ExternalPetscSolverApp::~ExternalPetscSolverApp()
{
#if LIBMESH_HAVE_PETSC
  PETScExternalSolverDestroy(_ts);
#endif
}

void
ExternalPetscSolverApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"ExternalPetscSolverApp"});
  Registry::registerActionsTo(af, {"ExternalPetscSolverApp"});

  /* register custom execute flags, action syntax, etc. here */
}

std::shared_ptr<Backup>
ExternalPetscSolverApp::backup()
{
  auto backup = MooseApp::backup();

  // Backup current solution
  PetscVector<Number> petsc_sol(_petsc_sol, comm());
  dataStore(backup->_system_data, static_cast<NumericVector<Number> &>(petsc_sol), nullptr);

  return backup;
}

void
ExternalPetscSolverApp::restore(std::shared_ptr<Backup> backup, bool for_restart)
{
  MooseApp::restore(backup, for_restart);

  // Restore previous solution
  PetscVector<Number> petsc_sol(_petsc_sol, comm());
  dataLoad(backup->_system_data, static_cast<NumericVector<Number> &>(petsc_sol), nullptr);
}

void
ExternalPetscSolverApp::registerApps()
{
  registerApp(ExternalPetscSolverApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
ExternalPetscSolverApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ExternalPetscSolverApp::registerAll(f, af, s);
}
extern "C" void
ExternalPetscSolverApp__registerApps()
{
  ExternalPetscSolverApp::registerApps();
}
