//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NGSSNESExecutor.h"
#include "FEProblemBase.h"

#include "libmesh/petsc_solver_exception.h"
#include <petscsnes.h>

registerMooseObject("MooseApp", NGSSNESExecutor);

InputParameters
NGSSNESExecutor::validParams()
{
  InputParameters params = SNESExecutor::validParams();
  params.addClassDescription(
      "Nonlinear Gauss-Seidel preconditioner (SNESNGS) for a single monolithic nonlinear system. "
      "Attach via nl_preconditioning on a NewtonSNESExecutor; PETSc propagates the outer "
      "SNES's DM to this SNES and drives the NGS sweep internally.");
  params.addRequiredParam<NonlinearSystemName>(
      "nonlinear_system_name",
      "The nonlinear system that nonlinear Gauss-Seidel (NGS) should be applied to. Note that NGS "
      "is not typically robust as a standalone solver; it's better to use as a preconditioner.");
  return params;
}

NGSSNESExecutor::NGSSNESExecutor(const InputParameters & params)
  : SNESExecutor(params),
    _nl_sys_num(_fe_problem.nlSysNum(getParam<NonlinearSystemName>("nonlinear_system_name")))
{
}

void
NGSSNESExecutor::setupSNES()
{
  LibmeshPetscCallA(this->comm().get(), SNESCreate(this->comm().get(), &_snes));
  LibmeshPetscCallA(this->comm().get(), SNESSetType(_snes, SNESNGS));
  _snes_setup_done = true;
}

Executor::Result
NGSSNESExecutor::run()
{
  _fe_problem.solve(_nl_sys_num);
  auto & result = newResult();
  if (_fe_problem.solverSystemConverged(_nl_sys_num))
    result.pass("solver converged");
  else
    result.fail("solver diverged");
  return result;
}
