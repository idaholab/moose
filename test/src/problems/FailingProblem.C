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

FailingProblem::FailingProblem(const std::string & name, InputParameters params) :
    FEProblem(name, params),
    _failed(false),
    _fail_step(getParam<unsigned int>("fail_step"))
{}

bool
FailingProblem::converged()
{
  if(!_failed && (_t_step == static_cast<int>(_fail_step)))
  {
    _failed = true;
    return false;
  }

  return FEProblem::converged();
}
