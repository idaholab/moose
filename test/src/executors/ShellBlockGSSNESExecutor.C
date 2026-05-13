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
  params.addRequiredParam<std::vector<NonlinearSystemName>>(
      "inner_nl_sys_names",
      "Names of the nonlinear systems to sweep in each shell solve application, in order.");
  return params;
}

ShellBlockGSSNESExecutor::ShellBlockGSSNESExecutor(const InputParameters & params)
  : SNESExecutor(params)
{
  for (const auto & name : getParam<std::vector<NonlinearSystemName>>("inner_nl_sys_names"))
    _inner_sys_nums.push_back(_fe_problem.nlSysNum(name));
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
  for (const auto sys_num : _inner_sys_nums)
    _fe_problem.solve(sys_num);
  result.pass("block GS sweep complete");
  return result;
}

PetscErrorCode
ShellBlockGSSNESExecutor::shellSolveCallback(SNES snes, Vec /*x*/)
{
  PetscFunctionBegin;
  void * ctx;
  PetscCall(SNESGetApplicationContext(snes, &ctx));
  auto * ex = static_cast<ShellBlockGSSNESExecutor *>(ctx);
  for (const auto sys_num : ex->_inner_sys_nums)
    ex->_fe_problem.solve(sys_num);
  PetscFunctionReturn(PETSC_SUCCESS);
}
