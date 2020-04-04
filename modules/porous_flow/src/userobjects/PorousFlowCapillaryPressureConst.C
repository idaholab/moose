//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowCapillaryPressureConst.h"

registerMooseObject("PorousFlowApp", PorousFlowCapillaryPressureConst);

InputParameters
PorousFlowCapillaryPressureConst::validParams()
{
  InputParameters params = PorousFlowCapillaryPressure::validParams();
  params.addRangeCheckedParam<Real>(
      "pc", 0.0, "pc >= 0", "Constant capillary pressure (Pa). Default is 0");
  params.addClassDescription("Constant capillary pressure");
  return params;
}

PorousFlowCapillaryPressureConst::PorousFlowCapillaryPressureConst(
    const InputParameters & parameters)
  : PorousFlowCapillaryPressure(parameters), _pc(getParam<Real>("pc"))
{
  // Set _log_ext to false as the logarithmic extension is not necessary in this object
  _log_ext = false;
}

Real
PorousFlowCapillaryPressureConst::effectiveSaturation(Real /*pc*/, unsigned /*qp*/) const
{
  return 1.0;
}

Real
PorousFlowCapillaryPressureConst::dEffectiveSaturation(Real /*pc*/, unsigned /*qp*/) const
{
  return 0.0;
}

Real
PorousFlowCapillaryPressureConst::d2EffectiveSaturation(Real /*pc*/, unsigned /*qp*/) const
{
  return 0.0;
}

Real
PorousFlowCapillaryPressureConst::capillaryPressureCurve(Real /*saturation*/, unsigned /*qp*/) const
{
  return _pc;
}

Real
PorousFlowCapillaryPressureConst::dCapillaryPressureCurve(Real /*saturation*/,
                                                          unsigned /*qp*/) const
{
  return 0.0;
}

Real
PorousFlowCapillaryPressureConst::d2CapillaryPressureCurve(Real /*saturation*/,
                                                           unsigned /*qp*/) const
{
  return 0.0;
}
