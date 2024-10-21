//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SegregatedSolverBase.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"
#include "KernelBase.h"
#include "INSFVMomentumPressure.h"
#include "libmesh/enum_point_locator_type.h"
#include "PetscVectorReader.h"

#include "libmesh/petsc_nonlinear_solver.h"
#include <petscerror.h>
#include <petscsys.h>
#include <petscksp.h>

InputParameters
SegregatedSolverBase::validParams()
{
  InputParameters params = SIMPLESolveBase::validParams();
  params += Executioner::validParams();

  return params;
}

SegregatedSolverBase::SegregatedSolverBase(const InputParameters & parameters)
  : Executioner(parameters),
    _problem(_fe_problem),
    _time_step(_problem.timeStep()),
    _time(_problem.time())
{
}

bool
SegregatedSolverBase::hasMultiAppError(const ExecFlagEnum & flags)
{
  bool has_error = false;
  for (const auto & flag : flags)
    if (_problem.hasMultiApps(flag))
    {
      _console << "\nCannot use SegregatedSolverBase solves with MultiApps set to execute on "
               << flag.name() << "!\nExiting...\n"
               << std::endl;
      has_error = true;
    }

  return has_error;
}

bool
SegregatedSolverBase::hasTransferError(const ExecFlagEnum & flags)
{
  for (const auto & flag : flags)
    if (_problem.getTransfers(flag, MultiAppTransfer::TO_MULTIAPP).size() ||
        _problem.getTransfers(flag, MultiAppTransfer::FROM_MULTIAPP).size() ||
        _problem.getTransfers(flag, MultiAppTransfer::BETWEEN_MULTIAPP).size())
    {
      _console
          << "\nCannot use SegregatedSolverBase solves with MultiAppTransfers set to execute on "
          << flag.name() << "!\nExiting...\n"
          << std::endl;
      return true;
    }

  return false;
}

void
SegregatedSolverBase::init()
{
  _problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _problem.initialSetup();
}
