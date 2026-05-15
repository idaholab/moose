//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShellBlockGSSNESExecutor.h"
#include "FEProblem.h"

#include "libmesh/petsc_solver_exception.h"
#include <petscsnes.h>

registerMooseObject("MooseTestApp", ShellBlockGSSNESExecutor);

InputParameters
ShellBlockGSSNESExecutor::validParams()
{
  InputParameters params = SNESExecutor::validParams();
  params.addClassDescription(
      "SNESSHELL-based block Gauss-Seidel executor for testing the multi-system Case 3 "
      "path of NewtonSNESExecutor.  Sweeps the listed nonlinear systems in order, calling "
      "_fe_problem.solve() for each.");
  params.addRequiredParam<std::vector<ExecutorName>>(
      "sub_snes_executors", "The sub-SNES executors who we will sweep over");
  return params;
}

ShellBlockGSSNESExecutor::ShellBlockGSSNESExecutor(const InputParameters & params)
  : SNESExecutor(params)
{
}

void
ShellBlockGSSNESExecutor::initialSetup()
{
  SNESExecutor::initialSetup();

  for (const auto & name : getParam<std::vector<ExecutorName>>("sub_snes_executors"))
    _sub_snes.push_back(&getExecutorByName<SNESExecutor>(name));
}

void
ShellBlockGSSNESExecutor::setupSNES()
{
  LibmeshPetscCallA(this->comm().get(), SNESCreate(this->comm().get(), &_snes));
  LibmeshPetscCallA(this->comm().get(), SNESSetType(_snes, SNESSHELL));
  LibmeshPetscCallA(this->comm().get(), SNESSetApplicationContext(_snes, this));
  LibmeshPetscCallA(this->comm().get(), SNESShellSetSolve(_snes, shellSolveCallback));
  _snes_setup_done = true;
}

Executor::Result
ShellBlockGSSNESExecutor::run()
{
  auto & result = newResult();
  for (auto * const sub_snes : _sub_snes)
    result.record(sub_snes->name(), sub_snes->exec());
  return result;
}

// Will be called when this is a preconditioner
PetscErrorCode
ShellBlockGSSNESExecutor::shellSolveCallback(SNES snes, Vec /*x*/)
{
  PetscFunctionBegin;
  void * ctx;
  PetscCall(SNESGetApplicationContext(snes, &ctx));
  auto * const ex = static_cast<ShellBlockGSSNESExecutor *>(ctx);
  ex->run();
  PetscFunctionReturn(PETSC_SUCCESS);
}
