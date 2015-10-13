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

#include "ConstantRate.h"

template<>
InputParameters validParams<ConstantRate>()
{
  InputParameters params = validParams<NodalKernel>();
  params.addRequiredParam<Real>("rate", "The constant rate in 'du/dt = rate'");
  return params;
}

ConstantRate::ConstantRate(const InputParameters & parameters) :
    NodalKernel(parameters),
    _rate(getParam<Real>("rate"))
{
}

Real
ConstantRate::computeQpResidual()
{
  return _u_dot[_qp] - _rate;
}
