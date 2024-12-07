//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetOlssonTerminator.h"
#include "NonlinearSystem.h"

registerMooseObject("LevelSetApp", LevelSetOlssonTerminator);

InputParameters
LevelSetOlssonTerminator::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Tool for terminating the reinitialization of the level set equation "
                             "based on the criteria defined by Olsson et. al. (2007).");
  params.addRequiredParam<Real>(
      "tol", "The limit at which the reinitialization problem is considered converged.");
  params.addParam<int>("min_steps", 3, "The minimum number of time steps to consider.");
  return params;
}

LevelSetOlssonTerminator::LevelSetOlssonTerminator(const InputParameters & params)
  : GeneralUserObject(params),
    _solution_diff(
        _fe_problem.getNonlinearSystem(0).addVector("solution_diff", false, libMesh::PARALLEL)),
    _tol(getParam<Real>("tol")),
    _min_t_steps(getParam<int>("min_steps"))
{
}

void
LevelSetOlssonTerminator::execute()
{
  _solution_diff = *_fe_problem.getNonlinearSystem(0).currentSolution();
  _solution_diff -= _fe_problem.getNonlinearSystem(0).solutionOld();
  Real delta = _solution_diff.l2_norm() / _dt;
  _console << "Computed convergence criteria: " << delta << std::endl;

  if (_fe_problem.timeStep() < _min_t_steps)
    return;
  else if (delta < _tol)
    _fe_problem.terminateSolve();
}
