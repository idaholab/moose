//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledScalarAux.h"

registerMooseObject("MooseTestApp", CoupledScalarAux);

InputParameters
CoupledScalarAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addCoupledVar("coupled", 2.71828, "Coupled Scalar Value for Calculation");

  params.addParam<unsigned int>(
      "component", 0, "The individual component of the scalar variable to output");

  MooseEnum lag("CURRENT OLD OLDER", "CURRENT", false);
  params.addParam<MooseEnum>("lag", lag, "Determine the time level of the coupled value");

  return params;
}

CoupledScalarAux::CoupledScalarAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _lag(getParam<MooseEnum>("lag")),
    _coupled_val(_lag == 0 ? coupledScalarValue("coupled")
                           : (_lag == 1 ? coupledScalarValueOld("coupled")
                                        : coupledScalarValueOlder("coupled"))),
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
