/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
    _fe_problem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*parameters.getCheckedPointerParam<SystemBase *>("_sys")),
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

TimeIntegrator::~TimeIntegrator() {}

void
TimeIntegrator::solve()
{
  _nl.system().solve();
}
