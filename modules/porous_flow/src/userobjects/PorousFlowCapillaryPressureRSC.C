//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowCapillaryPressureRSC.h"
#include "PorousFlowRogersStallybrassClements.h"

registerMooseObject("PorousFlowApp", PorousFlowCapillaryPressureRSC);

InputParameters
PorousFlowCapillaryPressureRSC::validParams()
{
  InputParameters params = PorousFlowCapillaryPressure::validParams();
  params.addParam<Real>("oil_viscosity",
                        "Viscosity of oil (gas) phase.  It is assumed this is "
                        "double the water-phase viscosity.  (Note that this "
                        "effective saturation is mostly useful for 2-phase, not "
                        "single-phase.)");
  params.addParam<Real>("scale_ratio",
                        "This is porosity / permeability / beta^2, where beta may "
                        "be chosen by the user.  It has dimensions [time]");
  params.addParam<Real>("shift", "effective saturation is a function of (Pc - shift)");
  params.addClassDescription("Rogers-Stallybrass-Clements version of effective saturation for the "
                             "water phase, valid for residual saturations = 0, and viscosityOil = "
                             "2 * viscosityWater.  seff_water = 1 / sqrt(1 + exp((Pc - shift) / "
                             "scale)), where scale = 0.25 * scale_ratio * oil_viscosity.");
  return params;
}

PorousFlowCapillaryPressureRSC::PorousFlowCapillaryPressureRSC(const InputParameters & parameters)
  : PorousFlowCapillaryPressure(parameters),
    _oil_viscosity(getParam<Real>("oil_viscosity")),
    _scale_ratio(getParam<Real>("scale_ratio")),
    _shift(getParam<Real>("shift")),
    _scale(0.25 * _scale_ratio * _oil_viscosity)
{
  // Set _log_ext to false as no capillary pressure curves are implmented in this class
  _log_ext = false;
}

Real
PorousFlowCapillaryPressureRSC::capillaryPressureCurve(Real /*saturation*/, unsigned /*qp*/) const
{
  mooseError("PorousFlowCapillaryPressureRSC::capillaryPressure not implemented");
  return 0.0;
}

Real
PorousFlowCapillaryPressureRSC::dCapillaryPressureCurve(Real /*saturation*/, unsigned /*qp*/) const
{
  mooseError("PorousFlowCapillaryPressureRSC::dCapillaryPressure not implemented");
  return 0.0;
}

Real
PorousFlowCapillaryPressureRSC::d2CapillaryPressureCurve(Real /*saturation*/, unsigned /*qp*/) const
{
  mooseError("PorousFlowCapillaryPressureRSC::d2CapillaryPressure not implemented");
  return 0.0;
}

Real
PorousFlowCapillaryPressureRSC::effectiveSaturation(Real pc, unsigned /*qp*/) const
{
  return PorousFlowRogersStallybrassClements::effectiveSaturation(-pc, _shift, _scale);
}

Real
PorousFlowCapillaryPressureRSC::dEffectiveSaturation(Real pc, unsigned /*qp*/) const
{
  return -PorousFlowRogersStallybrassClements::dEffectiveSaturation(-pc, _shift, _scale);
}

Real
PorousFlowCapillaryPressureRSC::d2EffectiveSaturation(Real pc, unsigned /*qp*/) const
{
  return PorousFlowRogersStallybrassClements::d2EffectiveSaturation(-pc, _shift, _scale);
}
