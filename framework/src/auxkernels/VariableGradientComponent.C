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
#include "VariableGradientComponent.h"

template <>
InputParameters
validParams<VariableGradientComponent>()
{
  MooseEnum component("x=0 y=1 z=2");
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("gradient_variable",
                               "The variable from which to compute the gradient component");
  params.addParam<MooseEnum>("component", component, "The gradient component to compute");
  return params;
}

VariableGradientComponent::VariableGradientComponent(const InputParameters & parameters)
  : AuxKernel(parameters),
    _gradient(coupledGradient("gradient_variable")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
VariableGradientComponent::computeValue()
{
  return _gradient[_qp](_component);
}
