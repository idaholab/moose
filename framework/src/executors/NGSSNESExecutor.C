//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NGSSNESExecutor.h"
#include "FEProblem.h"

#include "libmesh/petsc_solver_exception.h"
#include <petscsnes.h>

registerMooseObject("MooseApp", NGSSNESExecutor);

InputParameters
NGSSNESExecutor::validParams()
{
  InputParameters params = SNESExecutor::validParams();
  params.addClassDescription(
      "Nonlinear Gauss-Seidel (SNESNGS) executor. Sweeps through the listed inner nonlinear "
      "systems in order. Can be used as a standalone solver or as the nl_preconditioning "
      "for a NewtonSNESExecutor.");
  params.addRequiredParam<std::vector<NonlinearSystemName>>(
      "inner_nl_sys_names",
      "Names of the nonlinear systems to sweep in each NGS application, in order.");
  return params;
}

NGSSNESExecutor::NGSSNESExecutor(const InputParameters & params) : SNESExecutor(params)
{
  for (const auto & name : getParam<std::vector<NonlinearSystemName>>("inner_nl_sys_names"))
    _inner_sys_nums.push_back(_fe_problem.nlSysNum(name));
}

void
NGSSNESExecutor::setupSNES()
{
  if (_fe_problem.numNonlinearSystems() < 2)
    mooseError("NGSSNESExecutor requires at least two nonlinear systems on the problem; "
               "NGS field-splitting requires distinct per-physics systems.");

  LibmeshPetscCallA(this->comm().get(), SNESCreate(this->comm().get(), &_snes));
  LibmeshPetscCallA(this->comm().get(), SNESSetType(_snes, SNESNGS));
  LibmeshPetscCallA(this->comm().get(), SNESSetNGS(_snes, ngsCallback, this));

  // When used as an NPC, the outer SNES judges convergence; skip norm computation.
  if (_is_npc)
    LibmeshPetscCallA(this->comm().get(), SNESSetNormSchedule(_snes, SNES_NORM_NONE));

  _snes_setup_done = true;
}

Executor::Result
NGSSNESExecutor::run()
{
  // As a standalone solver, perform a single NGS sweep.
  // When used as an NPC, the outer SNES drives the sweep via ngsCallback; run() is not called.
  auto & result = newResult();

  for (const auto sys_num : _inner_sys_nums)
    _fe_problem.solve(sys_num);

  result.pass("NGS sweep complete");
  return result;
}

PetscErrorCode
NGSSNESExecutor::ngsCallback(SNES /*snes*/, Vec /*x*/, Vec /*b*/, void * ctx)
{
  PetscFunctionBegin;
  auto * ex = static_cast<NGSSNESExecutor *>(ctx);
  for (const auto sys_num : ex->_inner_sys_nums)
    ex->_fe_problem.solve(sys_num);
  PetscFunctionReturn(PETSC_SUCCESS);
}
