//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectiveHeatFlux1PhaseAux.h"

registerMooseObject("ThermalHydraulicsApp", ADConvectiveHeatFlux1PhaseAux);

InputParameters
ADConvectiveHeatFlux1PhaseAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes convective heat flux for 1-phase flow.");
  params.addRequiredCoupledVar("T_wall", "Wall temperature");
  params.addRequiredParam<MaterialPropertyName>("T", "Material property name of fluid temperature");
  params.addRequiredParam<MaterialPropertyName>(
      "Hw", "Material property name of wall heat transfer coefficient");
  params.addParam<Real>("scaling_factor", 1., "Scaling factor");
  return params;
}

ADConvectiveHeatFlux1PhaseAux::ADConvectiveHeatFlux1PhaseAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _T_wall(coupledValue("T_wall")),
    _T(getADMaterialProperty<Real>("T")),
    _Hw(getADMaterialProperty<Real>("Hw")),
    _scaling_factor(getParam<Real>("scaling_factor"))
{
}

Real
ADConvectiveHeatFlux1PhaseAux::computeValue()
{
  return MetaPhysicL::raw_value(_Hw[_qp]) * (_T_wall[_qp] - MetaPhysicL::raw_value(_T[_qp])) *
         _scaling_factor;
}
