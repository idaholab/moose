/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity.h"

template <>
InputParameters
validParams<PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity>()
{
  InputParameters params = validParams<PorousFlowTotalGravitationalDensityBase>();
  params.addRequiredRangeCheckedParam<Real>(
      "rho_s", "rho_s > 0", "The density of the solid matrix");
  params.addClassDescription(
      "This Material calculates the porous medium density from the porosity, solid density "
      "(assumed "
      "constant) and fluid density, for the fully-saturated case, using a linear weighted average. "
      "density = phi * rho_f + (1 - phi) * rho_s, where phi is porosity and rho_f, rho_s are the "
      "densities of "
      "the fluid and solid phases.");
  return params;
}

PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity::
    PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity(
        const InputParameters & parameters)
  : PorousFlowTotalGravitationalDensityBase(parameters),
    _rho_s(getParam<Real>("rho_s")),
    _rho_f_qp(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _porosity_qp(getMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _drho_f_qp_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_density_qp_dvar")),
    _dporosity_qp_dvar(getMaterialProperty<std::vector<Real>>("dPorousFlow_porosity_qp_dvar"))
{
  if (_nodal_material == true)
    mooseError("PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity is only defined for "
               "at_nodes = false");
}

Real
PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity::rho_f(unsigned int ph) const
{
  return _rho_f_qp[_qp][ph];
}

Real
PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity::drho_f_dvar(unsigned int ph,
                                                                           unsigned int v) const
{
  return _drho_f_qp_dvar[_qp][ph][v];
}

void
PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity::initQpStatefulProperties()
{
  const unsigned ph = 0;
  _gravdensity[_qp] = _rho_s * (1.0 - _porosity_qp[_qp]) + rho_f(ph) * _porosity_qp[_qp];
}

void
PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity::computeQpProperties()
{
  const unsigned ph = 0;
  _gravdensity[_qp] = _rho_s * (1.0 - _porosity_qp[_qp]) + rho_f(ph) * _porosity_qp[_qp];

  _dgravdensity_dvar[_qp].resize(_num_var);
  for (unsigned int v = 0; v < _num_var; ++v)
  {
    _dgravdensity_dvar[_qp][v] =
        _dporosity_qp_dvar[_qp][v] * (rho_f(ph) - _rho_s) + drho_f_dvar(ph, v) * _porosity_qp[_qp];
  }
}
