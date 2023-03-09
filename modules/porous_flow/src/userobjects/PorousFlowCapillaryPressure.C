//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowCapillaryPressure.h"

InputParameters
PorousFlowCapillaryPressure::validParams()
{
  InputParameters params = DiscreteElementUserObject::validParams();
  params.addRangeCheckedParam<Real>(
      "sat_lr",
      0.0,
      "sat_lr >= 0 & sat_lr < 1",
      "Liquid residual saturation.  Must be between 0 and 1. Default is 0");
  params.addRangeCheckedParam<Real>("pc_max",
                                    1.0e9,
                                    "pc_max > 0",
                                    "Maximum capillary pressure (Pa). Must be > 0. Default is 1e9");
  params.addParam<bool>("log_extension",
                        true,
                        "Use a logarithmic extension for low saturation to avoid capillary "
                        "pressure going to infinity. Default is true.  Set to false if your "
                        "capillary pressure depends on spatially-dependent variables other than "
                        "saturation, as the log-extension C++ code for this case has yet to be "
                        "implemented");
  params.addClassDescription("Capillary pressure base class");
  return params;
}

PorousFlowCapillaryPressure::PorousFlowCapillaryPressure(const InputParameters & parameters)
  : DiscreteElementUserObject(parameters),
    _sat_lr(getParam<Real>("sat_lr")),
    _dseff_ds(1.0 / (1.0 - _sat_lr)),
    _log_ext(getParam<bool>("log_extension")),
    _pc_max(getParam<Real>("pc_max")),
    _sat_ext(0.0),
    _pc_ext(0.0),
    _slope_ext(0.0),
    _log10(std::log(10.0))
{
}

void
PorousFlowCapillaryPressure::initialSetup()
{
  // If _log_ext = true, calculate the saturation where the the logarithmic
  // extension meets the raw capillary pressure curve.
  if (_log_ext)
  {
    _sat_ext = extensionSaturation();
    _pc_ext = capillaryPressure(_sat_ext);
    _slope_ext = (std::log10(_pc_ext) - std::log10(_pc_max)) / _sat_ext;
  }
}

Real
PorousFlowCapillaryPressure::capillaryPressure(Real saturation, unsigned qp) const
{
  if (_log_ext && saturation < _sat_ext)
    return capillaryPressureLogExt(saturation);
  else
    return capillaryPressureCurve(saturation, qp);
}

Real
PorousFlowCapillaryPressure::dCapillaryPressure(Real saturation, unsigned qp) const
{
  if (_log_ext && saturation < _sat_ext)
    return dCapillaryPressureLogExt(saturation);
  else
    return dCapillaryPressureCurve(saturation, qp);
}

Real
PorousFlowCapillaryPressure::d2CapillaryPressure(Real saturation, unsigned qp) const
{
  if (_log_ext && saturation < _sat_ext)
    return d2CapillaryPressureLogExt(saturation);
  else
    return d2CapillaryPressureCurve(saturation, qp);
}

Real
PorousFlowCapillaryPressure::effectiveSaturationFromSaturation(Real saturation) const
{
  return (saturation - _sat_lr) / (1.0 - _sat_lr);
}

Real
PorousFlowCapillaryPressure::saturation(Real pc, unsigned qp) const
{
  return effectiveSaturation(pc, qp) * (1.0 - _sat_lr) + _sat_lr;
}

DualReal
PorousFlowCapillaryPressure::saturation(const DualReal & pc, unsigned qp) const
{
  const Real s = effectiveSaturation(pc.value(), qp) * (1.0 - _sat_lr) + _sat_lr;
  const Real ds_dpc = dEffectiveSaturation(pc.value(), qp) * (1.0 - _sat_lr);

  DualReal result = s;
  result.derivatives() = pc.derivatives() * ds_dpc;

  return result;
}

Real
PorousFlowCapillaryPressure::dSaturation(Real pc, unsigned qp) const
{
  return dEffectiveSaturation(pc, qp) * (1.0 - _sat_lr);
}

Real
PorousFlowCapillaryPressure::d2Saturation(Real pc, unsigned qp) const
{
  return d2EffectiveSaturation(pc, qp) * (1.0 - _sat_lr);
}

Real
PorousFlowCapillaryPressure::capillaryPressureLogExt(Real saturation) const
{
  return _pc_ext * std::pow(10.0, _slope_ext * (saturation - _sat_ext));
}

Real
PorousFlowCapillaryPressure::dCapillaryPressureLogExt(Real saturation) const
{
  return _pc_ext * _slope_ext * _log10 * std::pow(10.0, _slope_ext * (saturation - _sat_ext));
}

Real
PorousFlowCapillaryPressure::d2CapillaryPressureLogExt(Real saturation) const
{
  return _pc_ext * _slope_ext * _slope_ext * _log10 * _log10 *
         std::pow(10.0, _slope_ext * (saturation - _sat_ext));
}

Real
PorousFlowCapillaryPressure::extensionSaturation() const
{
  // Initial guess for saturation where extension matches curve
  Real sat = _sat_lr + 0.01;

  // Calculate the saturation where the extension matches the derivative of the
  // raw capillary pressure curve
  const unsigned int max_its = 20;
  const Real nr_tol = 1.0e-8;
  unsigned int iter = 0;

  while (std::abs(interceptFunction(sat)) > nr_tol)
  {
    sat = sat - interceptFunction(sat) / interceptFunctionDeriv(sat);

    iter++;
    if (iter > max_its)
      break;
  }

  return sat;
}

Real
PorousFlowCapillaryPressure::interceptFunction(Real saturation) const
{
  Real pc = capillaryPressureCurve(saturation);
  Real dpc = dCapillaryPressureCurve(saturation);

  return std::log10(pc) - saturation * dpc / (_log10 * pc) - std::log10(_pc_max);
}

Real
PorousFlowCapillaryPressure::interceptFunctionDeriv(Real saturation) const
{
  Real pc = capillaryPressureCurve(saturation);
  Real dpc = dCapillaryPressureCurve(saturation);
  Real d2pc = d2CapillaryPressureCurve(saturation);

  return saturation * (dpc * dpc / pc - d2pc) / (_log10 * pc);
}

DualReal
PorousFlowCapillaryPressure::capillaryPressure(const DualReal & saturation, unsigned qp) const
{
  const Real Pc = capillaryPressure(saturation.value(), qp);
  const Real dPc_ds = dCapillaryPressure(saturation.value(), qp);

  DualReal result = Pc;
  result.derivatives() = saturation.derivatives() * dPc_ds;

  return result;
}
