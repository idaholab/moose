/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowCapillaryPressure.h"

template <>
InputParameters
validParams<PorousFlowCapillaryPressure>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRangeCheckedParam<Real>(
      "sat_lr",
      0.0,
      "sat_lr >= 0 & sat_lr <= 1",
      "Liquid residual saturation.  Must be between 0 and 1. Default is 0");
  params.addClassDescription("Capillary pressure base class");
  return params;
}

PorousFlowCapillaryPressure::PorousFlowCapillaryPressure(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _sat_lr(getParam<Real>("sat_lr")),
    _dseff_ds(1.0 / (1.0 - _sat_lr))
{
}

Real
PorousFlowCapillaryPressure::effectiveSaturationFromSaturation(Real saturation) const
{
  return (saturation - _sat_lr) / (1.0 - _sat_lr);
}
