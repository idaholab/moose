//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowCapillaryPressureBC.h"
#include "PorousFlowBrooksCorey.h"

registerMooseObject("PorousFlowApp", PorousFlowCapillaryPressureBC);

InputParameters
PorousFlowCapillaryPressureBC::validParams()
{
  InputParameters params = PorousFlowCapillaryPressure::validParams();
  params.addRequiredParam<Real>("lambda", "Brooks-Corey exponent lambda");
  params.addRequiredRangeCheckedParam<Real>(
      "pe", "pe > 0", "Brooks-Corey entry pressure. Must be positive");
  params.addClassDescription("Brooks-Corey capillary pressure");
  return params;
}

PorousFlowCapillaryPressureBC::PorousFlowCapillaryPressureBC(const InputParameters & parameters)
  : PorousFlowCapillaryPressure(parameters),
    _lambda(getParam<Real>("lambda")),
    _pe(getParam<Real>("pe"))
{
}

Real
PorousFlowCapillaryPressureBC::capillaryPressureCurve(Real saturation, unsigned /*qp*/) const
{
  Real seff = effectiveSaturationFromSaturation(saturation);
  return PorousFlowBrooksCorey::capillaryPressure(seff, _pe, _lambda, _pc_max);
}

Real
PorousFlowCapillaryPressureBC::dCapillaryPressureCurve(Real saturation, unsigned /*qp*/) const
{
  Real seff = effectiveSaturationFromSaturation(saturation);
  return PorousFlowBrooksCorey::dCapillaryPressure(seff, _pe, _lambda, _pc_max) * _dseff_ds;
}

Real
PorousFlowCapillaryPressureBC::d2CapillaryPressureCurve(Real saturation, unsigned /*qp*/) const
{
  Real seff = effectiveSaturationFromSaturation(saturation);
  return PorousFlowBrooksCorey::d2CapillaryPressure(seff, _pe, _lambda, _pc_max) * _dseff_ds *
         _dseff_ds;
}

Real
PorousFlowCapillaryPressureBC::effectiveSaturation(Real pc, unsigned /*qp*/) const
{
  return PorousFlowBrooksCorey::effectiveSaturation(pc, _pe, _lambda);
}

Real
PorousFlowCapillaryPressureBC::dEffectiveSaturation(Real pc, unsigned /*qp*/) const
{
  return PorousFlowBrooksCorey::dEffectiveSaturation(pc, _pe, _lambda);
}

Real
PorousFlowCapillaryPressureBC::d2EffectiveSaturation(Real pc, unsigned /*qp*/) const
{
  return PorousFlowBrooksCorey::d2EffectiveSaturation(pc, _pe, _lambda);
}
