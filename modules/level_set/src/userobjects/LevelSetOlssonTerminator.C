/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LevelSetOlssonTerminator.h"
#include "NonlinearSystem.h"

template<>
InputParameters validParams<LevelSetOlssonTerminator>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Tool for terminating the reinitialization of the level set equation based on the criteria defined by Olsson et. al. (2007).");
  params.addRequiredParam<Real>("tol", "The limit at which the reinitialization problem is considered converged.");
  params.addParam<int>("min_steps", 3, "The minimum number of time steps to consider.");
  return params;
}

LevelSetOlssonTerminator::LevelSetOlssonTerminator(const InputParameters & params) :
    GeneralUserObject(params),
    _solution_diff(_fe_problem.getNonlinearSystem().addVector("solution_diff", false, PARALLEL)),
    _tol(getParam<Real>("tol")),
    _min_t_steps(getParam<int>("min_steps"))
{
}

void
LevelSetOlssonTerminator::execute()
{
  _solution_diff  = *_fe_problem.getNonlinearSystem().currentSolution();
  _solution_diff -= _fe_problem.getNonlinearSystem().solutionOld();
  Real delta = _solution_diff.l2_norm() / _dt;
  _console << "Computed convergence criteria: " << delta << std::endl;

  if (_fe_problem.timeStep() < _min_t_steps)
    return;
  else if (delta < _tol )
    _fe_problem.terminateSolve();
}
