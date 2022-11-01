//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FixedPoint.h"

#include "NonlinearSystemBase.h"

InputParameters
FixedPoint::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addParam<unsigned int>("fp_max_its", 50, "Max Fixed Point Iterations");
  params.addRangeCheckedParam<Real>(
      "fp_abs_tol", 1.0e-50, "fp_abs_tol>0", "Nonlinear Absolute Tolerance");
  params.addRangeCheckedParam<Real>(
      "fp_rel_tol", 1.0e-8, "fp_rel_tol>0", "Nonlinear Relative Tolerance");
  params.addRangeCheckedParam<Real>(
      "fp_abs_step_tol", 1.0e-50, "fp_abs_step_tol>=0", "Nonlinear Absolute step Tolerance");
  params.addRangeCheckedParam<Real>(
      "fp_rel_step_tol", 1.0e-50, "fp_rel_step_tol>=0", "Nonlinear Relative step Tolerance");
  params.addParamNamesToGroup("fp_max_its fp_abs_tol fp_rel_tol fp_abs_step_tol fp_rel_step_tol",
                              "FixedPoint");
  return params;
}

FixedPoint::FixedPoint(Executioner & ex)
  : SolveObject(ex),
    _fp_problem(dynamic_cast<FixedPointProblem &>(_problem)),
    _fp_max_its(getParam<unsigned int>("fp_max_its")),
    _fp_abs_tol(getParam<Real>("fp_abs_tol")),
    _fp_rel_tol(getParam<Real>("fp_rel_tol")),
    _fp_abs_step_tol(getParam<Real>("fp_abs_step_tol")),
    _fp_rel_step_tol(getParam<Real>("fp_rel_step_tol"))
{
}

bool
FixedPoint::solve()
{
  // evaluate kernels on previous solution
  _fp_problem.setCurrentNonlinearSystem(_nl.number());
  _fp_problem.computeFullResidual(*_nl.currentSolution(), _nl.RHS());
  Real initial_residual_norm = _nl.RHS().l2_norm();
  Real residual_norm = initial_residual_norm;
  _console << "Fixed point initial residual norm " << residual_norm << std::endl;

  unsigned int it = 0;
  for (; it < _fp_max_its; ++it)
  {
    _console << "Fixed point iteration " << it << std::endl;

    Real residual_norm_previous = residual_norm;

    if (!_inner_solve->solve())
    {
      _console << COLOR_RED << " Fixed point iteration did NOT converge!" << COLOR_DEFAULT
               << std::endl;
      // Perform the output of the current, failed time step (this only occurs if desired)
      _fp_problem.outputStep(EXEC_FAILED);
      return false;
    }

    _fp_problem.copySolution();
    _fp_problem.computeFullResidual(*_nl.currentSolution(), _nl.RHS());
    residual_norm = _nl.RHS().l2_norm();
    _console << "Fixed point residual norm " << residual_norm << std::endl;
    if (residual_norm < _fp_abs_tol)
    {
      _console << COLOR_GREEN << " Fixed point iteration converged with fp_abs_tol!"
               << COLOR_DEFAULT << std::endl;
      break;
    }
    if (residual_norm / initial_residual_norm < _fp_rel_tol)
    {
      _console << COLOR_GREEN << " Fixed point iteration converged with fp_rel_tol!"
               << COLOR_DEFAULT << std::endl;
      break;
    }
    if (std::abs(residual_norm - residual_norm_previous) < _fp_abs_step_tol)
    {
      _console << COLOR_GREEN << " Fixed point iteration converged with fp_abs_step_tol!"
               << COLOR_DEFAULT << std::endl;
      break;
    }
    if (std::abs(residual_norm - residual_norm_previous) / residual_norm < _fp_rel_step_tol)
    {
      _console << COLOR_GREEN << " Fixed point iteration converged with fp_rel_step_tol!"
               << COLOR_DEFAULT << std::endl;
      break;
    }
  }

  return (it != _fp_max_its);
}
