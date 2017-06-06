/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
