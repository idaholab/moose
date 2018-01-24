//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConsistentHeatCapacityTimeDerivative.h"

template <>
InputParameters
validParams<ConsistentHeatCapacityTimeDerivative>()
{
  InputParameters params = validParams<HeatCapacityConductionTimeDerivative>();
  params.addClassDescription("Time derivative term $(C_p + T \\frac{\\partial C_p}{\\partial T}) "
                             "\\frac{\\partial T}{\\partial t}$ of "
                             "the heat equation with the heat capacity $C_p$ as an argument.");
  return params;
}

ConsistentHeatCapacityTimeDerivative::ConsistentHeatCapacityTimeDerivative(
    const InputParameters & parameters)
  : HeatCapacityConductionTimeDerivative(parameters)
{
}

Real
ConsistentHeatCapacityTimeDerivative::computeQpResidual()
{
  return (_heat_capacity[_qp] + _d_heat_capacity_dT[_qp] * _u[_qp]) *
         TimeDerivative::computeQpResidual();
}
