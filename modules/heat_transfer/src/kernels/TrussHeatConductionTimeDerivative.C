//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TrussHeatConductionTimeDerivative.h"

registerMooseObject("HeatConductionApp", TrussHeatConductionTimeDerivative);

InputParameters
TrussHeatConductionTimeDerivative::validParams()
{
  InputParameters params = HeatConductionTimeDerivative::validParams();
  params.addClassDescription("Computes time derivative term in heat equation for truss elements, "
                             "taking cross-sectional area into account");
  params.addCoupledVar("area", "Cross-sectional area of truss element");
  return params;
}

TrussHeatConductionTimeDerivative::TrussHeatConductionTimeDerivative(
    const InputParameters & parameters)
  : HeatConductionTimeDerivative(parameters), _area(coupledValue("area"))
{
}

Real
TrussHeatConductionTimeDerivative::computeQpResidual()
{
  return _area[_qp] * HeatConductionTimeDerivative::computeQpResidual();
}

Real
TrussHeatConductionTimeDerivative::computeQpJacobian()
{
  return _area[_qp] * HeatConductionTimeDerivative::computeQpJacobian();
}
