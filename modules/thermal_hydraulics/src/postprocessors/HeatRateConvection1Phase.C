//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatRateConvection1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateConvection1Phase);

InputParameters
HeatRateConvection1Phase::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();

  params.addParam<MaterialPropertyName>(
      "T_wall", FlowModelSinglePhase::TEMPERATURE_WALL, "Wall temperature");
  params.addParam<MaterialPropertyName>(
      "T", FlowModelSinglePhase::TEMPERATURE, "Temperature of the fluid on the slave side");
  params.addParam<MaterialPropertyName>(
      "Hw", FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL, "Wall heat transfer coefficient");
  params.addRequiredCoupledVar("P_hf", "heat flux perimeter");

  params.addClassDescription("Computes convective heat rate into a 1-phase flow channel");

  return params;
}

HeatRateConvection1Phase::HeatRateConvection1Phase(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),

    _T_wall(getMaterialProperty<Real>("T_wall")),
    _T(getMaterialProperty<Real>("T")),
    _Hw(getMaterialProperty<Real>("Hw")),
    _P_hf(coupledValue("P_hf"))
{
}

Real
HeatRateConvection1Phase::computeQpIntegral()
{
  return -_Hw[_qp] * _P_hf[_qp] * (_T[_qp] - _T_wall[_qp]);
}
