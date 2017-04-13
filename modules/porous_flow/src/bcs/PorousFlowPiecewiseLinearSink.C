/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPiecewiseLinearSink.h"

template <>
InputParameters
validParams<PorousFlowPiecewiseLinearSink>()
{
  InputParameters params = validParams<PorousFlowSinkPTDefiner>();
  params.addRequiredParam<std::vector<Real>>(
      "pt_vals",
      "Tuple of pressure values (for the fluid_phase specified).  Must be monotonically "
      "increasing.  For heat fluxes that don't involve fluids, these are temperature "
      "values");
  params.addRequiredParam<std::vector<Real>>(
      "multipliers", "Tuple of multiplying values.  The flux values are multiplied by these.");
  return params;
}

PorousFlowPiecewiseLinearSink::PorousFlowPiecewiseLinearSink(const InputParameters & parameters)
  : PorousFlowSinkPTDefiner(parameters),
    _sink_func(getParam<std::vector<Real>>("pt_vals"), getParam<std::vector<Real>>("multipliers"))
{
}

Real
PorousFlowPiecewiseLinearSink::multiplier() const
{
  return PorousFlowSink::multiplier() * _sink_func.sample(ptVar());
}

Real
PorousFlowPiecewiseLinearSink::dmultiplier_dvar(unsigned int pvar) const
{
  return PorousFlowSink::dmultiplier_dvar(pvar) * _sink_func.sample(ptVar()) +
         PorousFlowSink::multiplier() * _sink_func.sampleDerivative(ptVar()) * dptVar(pvar);
}
