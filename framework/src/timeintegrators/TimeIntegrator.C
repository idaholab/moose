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
#include "NonlinearSystem.h"

template<>
InputParameters validParams<TimeIntegrator>()
{
  InputParameters params = validParams<MooseObject>();

  return params;
}

TimeIntegrator::TimeIntegrator(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    _fe_problem(*getParam<FEProblem *>("_fe_problem")),
    _nl(_fe_problem.getNonlinearSystem()),
    _u_dot(_nl.solutionUDot()),
    _du_dot_du(_nl.solutionDuDotDu()),
    _t_step(_fe_problem.timeStep()),
    _dt(_fe_problem.dt()),
    _dt_old(_fe_problem.dtOld()),
    _Re_time(_nl.residualVector(Moose::KT_TIME)),
    _Re_non_time(_nl.residualVector(Moose::KT_NONTIME))
{
}

TimeIntegrator::~TimeIntegrator()
{
}

void
TimeIntegrator::solve()
{
  _nl.sys().solve();
}
