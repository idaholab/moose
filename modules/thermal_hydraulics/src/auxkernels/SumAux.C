//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SumAux.h"

registerMooseObject("ThermalHydraulicsApp", SumAux);

InputParameters
SumAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Sum of nonlinear or auxiliary variables");

  params.addRequiredCoupledVar("values", "Vector of values to sum");

  return params;
}

SumAux::SumAux(const InputParameters & parameters)
  : AuxKernel(parameters), _n_values(coupledComponents("values"))
{
  for (unsigned int i = 0; i < _n_values; i++)
    _values.push_back(&coupledValue("values", i));
}

Real
SumAux::computeValue()
{
  Real sum = 0;
  for (unsigned int i = 0; i < _n_values; i++)
    sum += (*(_values[i]))[_qp];

  return sum;
}
