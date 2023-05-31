//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SumIC.h"

registerMooseObject("ThermalHydraulicsApp", SumIC);

InputParameters
SumIC::validParams()
{
  InputParameters params = InitialCondition::validParams();

  params.addClassDescription("Sets the initial condition as the sum of other variables");

  params.addRequiredCoupledVar("values", "Vector of values to sum");

  return params;
}

SumIC::SumIC(const InputParameters & parameters)
  : InitialCondition(parameters), _n_values(coupledComponents("values"))
{
  for (unsigned int i = 0; i < _n_values; i++)
    _values.push_back(&coupledValue("values", i));
}

Real
SumIC::value(const Point & /*p*/)
{
  Real sum = 0;
  for (unsigned int i = 0; i < _n_values; i++)
    sum += (*(_values[i]))[_qp];

  return sum;
}
