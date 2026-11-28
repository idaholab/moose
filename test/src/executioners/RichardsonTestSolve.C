//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsonTestSolve.h"
#include "SolveObjectExecutor.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"

using RichardsonTestSolveExecutor = SolveObjectExecutor<RichardsonTestSolve>;
registerMooseObject("MooseTestApp", RichardsonTestSolveExecutor);

InputParameters
RichardsonTestSolve::validParams()
{
  InputParameters params = NonlinearSolveObject::validParams();
  params.addClassDescription("Solves the first nonlinear system with a Richardson iteration");
  params.addRequiredRangeCheckedParam<Real>(
      "step_size", "step_size>0", "The factor of the residual in the solution update");
  params.addParam<unsigned int>("richardson_max_its", 1000, "The maximum number of iterations");
  params.addRangeCheckedParam<Real>(
      "richardson_abs_tol", 1e-50, "richardson_abs_tol>0", "The absolute residual tolerance");
  params.addRangeCheckedParam<Real>(
      "richardson_rel_tol", 1e-10, "richardson_rel_tol>0", "The relative residual tolerance");
  return params;
}

RichardsonTestSolve::RichardsonTestSolve(Executioner & ex)
  : NonlinearSolveObject(ex),
    _step_size(getParam<Real>("step_size")),
    _max_its(getParam<unsigned int>("richardson_max_its")),
    _abs_tol(getParam<Real>("richardson_abs_tol")),
    _rel_tol(getParam<Real>("richardson_rel_tol"))
{
}

bool
RichardsonTestSolve::solve()
{
  auto & residual = _nl.RHS();
  _problem.computeResidual(*_nl.currentSolution(), residual, _nl.number());
  const Real initial_norm = residual.l2_norm();

  for (const auto it : make_range(_max_its + 1))
  {
    const Real norm = residual.l2_norm();
    _console << "Richardson iteration " << it << ", residual norm " << norm << std::endl;
    if (norm < _abs_tol || norm < _rel_tol * initial_norm)
      return true;
    if (it == _max_its)
      break;

    _nl.solution().add(-_step_size, residual);
    _nl.solution().close();
    _nl.update();
    _problem.computeResidual(*_nl.currentSolution(), residual, _nl.number());
  }

  return false;
}
