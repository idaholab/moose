/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "TotalMineralVolumeFraction.h"

template <>
InputParameters
validParams<TotalMineralVolumeFraction>()
{
  InputParameters params = validParams<ElementAverageValue>();
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
