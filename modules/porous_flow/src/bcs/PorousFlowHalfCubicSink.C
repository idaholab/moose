/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowHalfCubicSink.h"

template<>
InputParameters validParams<PorousFlowHalfCubicSink>()
{
  InputParameters params = validParams<PorousFlowSink>();
  params.addRequiredParam<Real>("max", "Maximum of the cubic flux multiplier.  Denote x = porepressure - centre.  Then Flux out is multiplied by (max/cutoff^3)*(2x + cutoff)(x - cutoff)^2 for cutoff < x < 0.  Flux out is multiplied by max for x >= 0.  Flux out is multiplied by 0 for x <= cutoff.");
  params.addRequiredParam<FunctionName>("cutoff", "Cutoff of the cubic (measured in Pa).  This needs to be less than zero.");
  params.addRequiredParam<Real>("centre", "Centre of the cubic flux multiplier (measured in Pa).");
  return params;
}

PorousFlowHalfCubicSink::PorousFlowHalfCubicSink(const InputParameters & parameters) :
    PorousFlowSink(parameters),
    _maximum(getParam<Real>("max")),
    _cutoff(getFunction("cutoff")),
    _centre(getParam<Real>("centre")),
    _pp(getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure_nodal")),
    _dpp_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_nodal_dvar"))
{
}

Real
PorousFlowHalfCubicSink::multiplier()
{
  const Real x = _pp[_qp_map[_i]][_ph] - _centre;

  if (x >= 0)
    return PorousFlowSink::multiplier() * _maximum;

  const Real cutoff = _cutoff.value(_t, _q_point[_qp]);
  if (x <= cutoff)
    return 0.0;

  return PorousFlowSink::multiplier() * _maximum * (2 * x + cutoff) * (x - cutoff) * (x - cutoff) / std::pow(cutoff, 3);
}

Real
PorousFlowHalfCubicSink::dmultiplier_dvar(unsigned int pvar)
{
  const Real x = _pp[_qp_map[_i]][_ph] - _centre;

  if (x >= 0)
    return PorousFlowSink::dmultiplier_dvar(pvar) * _maximum;

  const Real cutoff = _cutoff.value(_t, _q_point[_qp]);
  if (x <= cutoff)
    return 0.0;

  const Real str = _maximum * (2 * x + cutoff) * (x - cutoff) * (x - cutoff) / std::pow(cutoff, 3);
  const Real deriv = _maximum * 6 * x * (x - cutoff) / std::pow(cutoff, 3);
  return PorousFlowSink::dmultiplier_dvar(pvar) * str + PorousFlowSink::multiplier() * deriv * _dpp_dvar[_qp_map[_i]][_ph][pvar];
}
