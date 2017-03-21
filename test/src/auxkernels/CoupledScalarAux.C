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

#include "CoupledScalarAux.h"

template <>
InputParameters
validParams<CoupledScalarAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addCoupledVar("coupled", 2.71828, "Coupled Scalar Value for Calculation");

  params.addParam<unsigned int>(
      "component", 0, "The individual component of the scalar variable to output");

  return params;
}

CoupledScalarAux::CoupledScalarAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _coupled_val(coupledScalarValue("coupled")),
    _component(getParam<unsigned int>("component"))
{
  if (_component >= coupledScalarOrder("coupled"))
    mooseError("component is higher than or equal to the scalar variable order");
}

Real
CoupledScalarAux::computeValue()
{
  return _coupled_val[_component];
}
