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

InputParameters
ExternalPetscSolverApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("use_legacy_material_output") = false;
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
    PETScExternalSolverCreate(_comm->get(), &_ts);
    _is_petsc_app = true;
  }
  return _ts;
}

ExternalPetscSolverApp::~ExternalPetscSolverApp()
{
  // Destroy PETSc solver
  PETScExternalSolverDestroy(_ts);
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

  // We only need to do these backups when external petsc solver is used
  if (_is_petsc_app)
  {
    ExternalPETScProblem & external_petsc_problem =
        static_cast<ExternalPETScProblem &>(_executioner->feProblem());

    // Backup current solution
    dataStore(backup->_system_data, external_petsc_problem.currentSolution(), nullptr);

    // Backup the old solution
    dataStore(backup->_system_data, external_petsc_problem.solutionOld(), nullptr);

    // Backup Udot
    dataStore(backup->_system_data, external_petsc_problem.udot(), nullptr);
  }

  return backup;
}

void
ExternalPetscSolverApp::restore(std::shared_ptr<Backup> backup, bool for_restart)
{
  MooseApp::restore(backup, for_restart);

  // We only need to do these backups when external petsc solver is used
  if (_is_petsc_app)
  {
    ExternalPETScProblem & external_petsc_problem =
        static_cast<ExternalPETScProblem &>(_executioner->feProblem());

    // Restore previous solution
    dataLoad(backup->_system_data, external_petsc_problem.currentSolution(), nullptr);

    // Restore the solution at the previous time step
    dataLoad(backup->_system_data, external_petsc_problem.solutionOld(), nullptr);

    // Restore udot
    dataLoad(backup->_system_data, external_petsc_problem.udot(), nullptr);
  }
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
