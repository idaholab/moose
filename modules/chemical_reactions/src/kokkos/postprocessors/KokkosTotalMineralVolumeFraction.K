//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosTotalMineralVolumeFraction.h"

registerKokkosUserObject("ChemicalReactionsApp", KokkosTotalMineralVolumeFraction);

InputParameters
KokkosTotalMineralVolumeFraction::validParams()
{
  InputParameters params = KokkosElementAverageValue::validParams();
  params.addRequiredParam<Real>("molar_volume", "Molar volume of coupled mineral species");
  params.addClassDescription("Total volume fraction of coupled mineral species");
  return params;
}

KokkosTotalMineralVolumeFraction::KokkosTotalMineralVolumeFraction(
    const InputParameters & parameters)
  : KokkosElementAverageValue(parameters), _molar_volume(getParam<Real>("molar_volume"))
{
}
