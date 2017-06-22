/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowCapillaryPressureConst.h"

template <>
InputParameters
validParams<PorousFlowCapillaryPressureConst>()
{
  InputParameters params = validParams<PorousFlowCapillaryPressure>();
  params.addRangeCheckedParam<Real>(
      "pc",
      0.0,
      "pc <= 0",
      "Constant capillary pressure (Pa). Default is 0. Note: capillary pressure must be negative");
  params.addClassDescription("Constant capillary pressure");
  return params;
}

PorousFlowCapillaryPressureConst::PorousFlowCapillaryPressureConst(
    const InputParameters & parameters)
  : PorousFlowCapillaryPressure(parameters), _pc(getParam<Real>("pc"))
{
}

Real PorousFlowCapillaryPressureConst::capillaryPressure(Real /*saturation*/) const { return _pc; }

Real PorousFlowCapillaryPressureConst::dCapillaryPressure(Real /*saturation*/) const { return 0.0; }

Real PorousFlowCapillaryPressureConst::d2CapillaryPressure(Real /*saturation*/) const
{
  return 0.0;
}

Real PorousFlowCapillaryPressureConst::effectiveSaturation(Real /*pc*/) const { return 1.0; }

Real PorousFlowCapillaryPressureConst::dEffectiveSaturation(Real /*pc*/) const { return 0.0; }

Real PorousFlowCapillaryPressureConst::d2EffectiveSaturation(Real /*pc*/) const { return 0.0; }
