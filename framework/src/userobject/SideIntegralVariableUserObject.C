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

#include "SideIntegralVariableUserObject.h"

template <>
InputParameters
validParams<SideIntegralVariableUserObject>()
{
  InputParameters params = validParams<SideIntegralUserObject>();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable that this boundary condition applies to");
  return params;
}

SideIntegralVariableUserObject::SideIntegralVariableUserObject(const InputParameters & parameters)
  : SideIntegralUserObject(parameters),
    MooseVariableInterface(this, false),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable"))
{
  addMooseVariableDependency(mooseVariable());
}

Real
SideIntegralVariableUserObject::computeQpIntegral()
{
  return _u[_qp];
}
