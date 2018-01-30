//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeIntegrator.h"
#include "FEProblem.h"
#include "SystemBase.h"
#include "NonlinearSystem.h"

template <>
InputParameters
validParams<TimeIntegrator>()
{
  InputParameters params = validParams<MooseObject>();
  params.registerBase("TimeIntegrator");
  return params;
}

TimeIntegrator::TimeIntegrator(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(parameters, "TimeIntegrators"),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _nl(_fe_problem.getNonlinearSystemBase()),
    _u_dot(_sys.solutionUDot()),
    _du_dot_du(_sys.duDotDu()),
    _solution(_sys.currentSolution()),
    _solution_old(_sys.solutionOld()),
    _solution_older(_sys.solutionOlder()),
    _t_step(_fe_problem.timeStep()),
    _dt(_fe_problem.dt()),
    _dt_old(_fe_problem.dtOld()),
    _Re_time(_nl.residualVector(Moose::KT_TIME)),
    _Re_non_time(_nl.residualVector(Moose::KT_NONTIME))
{
}

void
TimeIntegrator::solve()
{
  _nl.system().solve();
}
