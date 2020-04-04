//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalMineralVolumeFraction.h"

registerMooseObject("ChemicalReactionsApp", TotalMineralVolumeFraction);

InputParameters
TotalMineralVolumeFraction::validParams()
{
  InputParameters params = ElementAverageValue::validParams();
  params.addRequiredParam<Real>("molar_volume", "Molar volume of coupled mineral species");
  params.addClassDescription("Total volume fraction of coupled mineral species");
  return params;
}

TotalMineralVolumeFraction::TotalMineralVolumeFraction(const InputParameters & parameters)
  : ElementAverageValue(parameters), _molar_volume(getParam<Real>("molar_volume"))
{
}

Real
TotalMineralVolumeFraction::computeQpIntegral()
{
  return _molar_volume * _u[_qp];
}
