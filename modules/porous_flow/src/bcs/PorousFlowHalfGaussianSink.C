/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowHalfGaussianSink.h"

template <>
InputParameters
validParams<PorousFlowHalfGaussianSink>()
{
  InputParameters params = validParams<PorousFlowSinkPTDefiner>();
  params.addRequiredParam<Real>("max",
                                "Maximum of the Gaussian flux multiplier.  Flux out is "
                                "multiplied by max*exp((-0.5*(p - center)/sd)^2) for "
                                "p<center, and by = max for p>center.  Here p is the nodal "
                                "porepressure for the fluid_phase specified (or, for heat "
                                "fluxes, it is the temperature).");
  params.addRequiredParam<Real>("sd",
                                "Standard deviation of the Gaussian flux multiplier "
                                "(measured in Pa (or K for heat fluxes)).");
  params.addRequiredParam<Real>(
      "center", "Center of the Gaussian flux multiplier (measured in Pa (or K for heat fluxes)).");
  return params;
}

PorousFlowHalfGaussianSink::PorousFlowHalfGaussianSink(const InputParameters & parameters)
  : PorousFlowSinkPTDefiner(parameters),
    _maximum(getParam<Real>("max")),
    _sd(getParam<Real>("sd")),
    _center(getParam<Real>("center"))
{
}

Real
PorousFlowHalfGaussianSink::multiplier() const
{
  if (ptVar() >= _center)
    return PorousFlowSink::multiplier() * _maximum;
  return PorousFlowSink::multiplier() * _maximum *
         std::exp(-0.5 * std::pow((ptVar() - _center) / _sd, 2));
}

Real
PorousFlowHalfGaussianSink::dmultiplier_dvar(unsigned int pvar) const
{
  if (ptVar() >= _center)
    return PorousFlowSink::dmultiplier_dvar(pvar) * _maximum;
  const Real str = _maximum * std::exp(-0.5 * std::pow((ptVar() - _center) / _sd, 2));
  return PorousFlowSink::dmultiplier_dvar(pvar) * str +
         PorousFlowSink::multiplier() * str * (_center - ptVar()) / std::pow(_sd, 2) * dptVar(pvar);
}
