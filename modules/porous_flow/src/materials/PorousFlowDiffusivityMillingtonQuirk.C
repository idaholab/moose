/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowDiffusivityMillingtonQuirk.h"

template <>
InputParameters
validParams<PorousFlowDiffusivityMillingtonQuirk>()
{
  InputParameters params = validParams<PorousFlowDiffusivityBase>();
  params.addClassDescription(
      "This Material provides saturation-dependent diffusivity using the Millington-Quirk model");
  return params;
}

PorousFlowDiffusivityMillingtonQuirk::PorousFlowDiffusivityMillingtonQuirk(
    const InputParameters & parameters)
  : PorousFlowDiffusivityBase(parameters),
    _porosity_qp(getMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _dporosity_qp_dvar(getMaterialProperty<std::vector<Real>>("dPorousFlow_porosity_qp_dvar")),
    _saturation_qp(getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp")),
    _dsaturation_qp_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_saturation_qp_dvar"))
{
}

void
PorousFlowDiffusivityMillingtonQuirk::computeQpProperties()
{
  PorousFlowDiffusivityBase::computeQpProperties();

  _tortuosity[_qp].resize(_num_phases);
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _tortuosity[_qp][ph] =
        std::cbrt(_porosity_qp[_qp]) * std::pow(_saturation_qp[_qp][ph], 10.0 / 3.0);
    for (unsigned int var = 0; var < _num_var; ++var)
      _dtortuosity_dvar[_qp][ph][var] =
          1.0 / 3.0 * std::cbrt(_porosity_qp[_qp]) * std::pow(_saturation_qp[_qp][ph], 7.0 / 3.0) *
          (_saturation_qp[_qp][ph] / _porosity_qp[_qp] * _dporosity_qp_dvar[_qp][var] +
           10.0 * _dsaturation_qp_dvar[_qp][ph][var]);
  }
}
