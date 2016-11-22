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

#include "FailingProblem.h"

#include "MooseApp.h"

template<>
InputParameters validParams<FailingProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addRequiredParam<unsigned int>("fail_step", "The timestep to fail");
  return params;
}

FailingProblem::FailingProblem(const InputParameters & params) :
    FEProblem(params),
    _solves_at_fail_step(0),
    _fail_step(getParam<unsigned int>("fail_step"))
{}

void
FailingProblem::solve()
{
  if (_t_step == static_cast<int>(_fail_step))
    _solves_at_fail_step++;

  FEProblem::solve();
}

bool
FailingProblem::converged()
{
  if (_solves_at_fail_step == 1 && (_t_step == static_cast<int>(_fail_step)))
  {
    _converged = false;
    return false;
  }

  return FEProblemBase::converged();
}
