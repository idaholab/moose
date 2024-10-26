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
#include "ExternalPETScProblem.h"
#include "Executioner.h"

#include "libmesh/petsc_solver_exception.h"

InputParameters
ExternalPetscSolverApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

ExternalPetscSolverApp::ExternalPetscSolverApp(InputParameters parameters)
  : MooseApp(parameters), _ts(nullptr), _is_petsc_app(false)
{
  ExternalPetscSolverApp::registerAll(_factory, _action_factory, _syntax);
}

TS &
ExternalPetscSolverApp::getPetscTS()
{
  if (!_ts)
  {
    // Create an external PETSc solver
    LibmeshPetscCall(PETScExternalSolverCreate(_comm->get(), &_ts));
    _is_petsc_app = true;
  }
  return _ts;
}

ExternalPetscSolverApp::~ExternalPetscSolverApp()
{
  // Destroy PETSc solver
  auto ierr = PETScExternalSolverDestroy(_ts);
  libmesh_ignore(ierr);
}

void
ExternalPetscSolverApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"ExternalPetscSolverApp"});
  Registry::registerActionsTo(af, {"ExternalPetscSolverApp"});

  /* register custom execute flags, action syntax, etc. here */
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
