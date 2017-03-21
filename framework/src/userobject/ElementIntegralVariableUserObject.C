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

#include "ElementIntegralVariableUserObject.h"

template <>
InputParameters
validParams<ElementIntegralVariableUserObject>()
{
  InputParameters params = validParams<ElementIntegralUserObject>();
  params.addRequiredCoupledVar("variable", "The name of the variable that this object operates on");
  return params;
}

ElementIntegralVariableUserObject::ElementIntegralVariableUserObject(
    const InputParameters & parameters)
  : ElementIntegralUserObject(parameters),
    MooseVariableInterface(this, false),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable"))
{
  addMooseVariableDependency(mooseVariable());
}

Real
ElementIntegralVariableUserObject::computeQpIntegral()
{
  return _u[_qp];
}
