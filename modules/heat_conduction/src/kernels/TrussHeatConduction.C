//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TrussHeatConduction.h"
#include "MooseMesh.h"

registerMooseObject("HeatConductionApp", TrussHeatConduction);

InputParameters
TrussHeatConduction::validParams()
{
  InputParameters params = HeatConductionKernel::validParams();
  params.addClassDescription("Computes conduction term in heat equation for truss elements, taking "
                             "cross-sectional area into account");
  params.addCoupledVar("area", "Cross-sectional area of truss element");
  return params;
}

TrussHeatConduction::TrussHeatConduction(const InputParameters & parameters)
  : HeatConductionKernel(parameters), _area(coupledValue("area"))
{
}

Real
TrussHeatConduction::computeQpResidual()
{
  return _area[_qp] * HeatConductionKernel::computeQpResidual();
}

Real
TrussHeatConduction::computeQpJacobian()
{
  return _area[_qp] * HeatConductionKernel::computeQpJacobian();
}
