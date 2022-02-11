//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableProductIC.h"

registerMooseObject("ThermalHydraulicsApp", VariableProductIC);

InputParameters
VariableProductIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredCoupledVar("values", "The values being multiplied");
  return params;
}

VariableProductIC::VariableProductIC(const InputParameters & parameters)
  : InitialCondition(parameters), _n(coupledComponents("values"))
{
  _values.resize(_n);
  for (unsigned int i = 0; i < _n; i++)
    _values[i] = &coupledValue("values", i);
}

Real
VariableProductIC::value(const Point & /*p*/)
{
  Real val = 1;
  for (unsigned int i = 0; i < _n; i++)
    val *= (*_values[i])[_qp];
  return val;
}
