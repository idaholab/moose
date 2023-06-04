//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WeightedAverageAux.h"

registerMooseObject("ThermalHydraulicsApp", WeightedAverageAux);

InputParameters
WeightedAverageAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Weighted average of variables using other variables as weights");

  params.addRequiredCoupledVar("values", "Vector of values to average");
  params.addRequiredCoupledVar("weights", "Vector of weights for each value");

  return params;
}

WeightedAverageAux::WeightedAverageAux(const InputParameters & parameters)
  : AuxKernel(parameters), _n_values(coupledComponents("values"))
{
  // make sure that number of weights equals the number of values
  if (coupledComponents("weights") != _n_values)
    mooseError(name(), ": The number of weights must equal the number of values");

  // get all of the variable values
  for (unsigned int i = 0; i < _n_values; i++)
  {
    _values.push_back(&coupledValue("values", i));
    _weights.push_back(&coupledValue("weights", i));
  }
}

Real
WeightedAverageAux::computeValue()
{
  Real weight_total = 0;
  for (unsigned int i = 0; i < _n_values; i++)
    weight_total += (*(_weights[i]))[_qp];

  Real weighted_sum = 0;
  for (unsigned int i = 0; i < _n_values; i++)
    weighted_sum += (*(_weights[i]))[_qp] * (*(_values[i]))[_qp];

  return weighted_sum / weight_total;
}
