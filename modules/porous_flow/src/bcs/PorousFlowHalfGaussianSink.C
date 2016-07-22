/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowHalfGaussianSink.h"

template<>
InputParameters validParams<PorousFlowHalfGaussianSink>()
{
  InputParameters params = validParams<PorousFlowSink>();
  params.addRequiredParam<Real>("max", "Maximum of the Gaussian flux multiplier.  Flux out is multiplied by max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and by = max for p>centre.  Here p is the nodal porepressure for the fluid_phase specified.");
  params.addRequiredParam<Real>("sd", "Standard deviation of the Gaussian flux multiplier (measured in Pa).");
  params.addRequiredParam<Real>("centre", "Centre of the Gaussian flux multiplier (measured in Pa).");
  return params;
}

PorousFlowHalfGaussianSink::PorousFlowHalfGaussianSink(const InputParameters & parameters) :
    PorousFlowSink(parameters),
    _maximum(getParam<Real>("max")),
    _sd(getParam<Real>("sd")),
    _centre(getParam<Real>("centre")),
    _pp(getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure_nodal")),
    _dpp_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_nodal_dvar"))
{
}

Real
PorousFlowHalfGaussianSink::multiplier()
{
  if (_pp[_qp_map[_i]][_ph] >= _centre)
    return PorousFlowSink::multiplier() * _maximum;
  return PorousFlowSink::multiplier() * _maximum * std::exp(-0.5 * std::pow((_pp[_qp_map[_i]][_ph] - _centre) / _sd, 2));
}

Real
PorousFlowHalfGaussianSink::dmultiplier_dvar(unsigned int pvar)
{
  if (_pp[_qp_map[_i]][_ph] >= _centre)
    return PorousFlowSink::dmultiplier_dvar(pvar) * _maximum;
  const Real str = _maximum * std::exp(-0.5 * std::pow((_pp[_qp_map[_i]][_ph] - _centre) / _sd, 2));
  return PorousFlowSink::dmultiplier_dvar(pvar) * str + PorousFlowSink::multiplier() * str * (_centre - _pp[_qp_map[_i]][_ph]) / std::pow(_sd, 2) * _dpp_dvar[_qp_map[_i]][_ph][pvar];
}
