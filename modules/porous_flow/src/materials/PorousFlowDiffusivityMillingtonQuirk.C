//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDiffusivityMillingtonQuirk.h"

registerMooseObject("PorousFlowApp", PorousFlowDiffusivityMillingtonQuirk);
registerMooseObject("PorousFlowApp", ADPorousFlowDiffusivityMillingtonQuirk);

template <bool is_ad>
InputParameters
PorousFlowDiffusivityMillingtonQuirkTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowDiffusivityBaseTempl<is_ad>::validParams();
  params.addClassDescription(
      "This Material provides saturation-dependent diffusivity using the Millington-Quirk model");
  return params;
}

template <bool is_ad>
PorousFlowDiffusivityMillingtonQuirkTempl<is_ad>::PorousFlowDiffusivityMillingtonQuirkTempl(
    const InputParameters & parameters)
  : PorousFlowDiffusivityBaseTempl<is_ad>(parameters),
    _porosity_qp(this->template getGenericMaterialProperty<Real, is_ad>("PorousFlow_porosity_qp")),
    _dporosity_qp_dvar(is_ad ? nullptr
                             : &this->template getMaterialProperty<std::vector<Real>>(
                                   "dPorousFlow_porosity_qp_dvar")),
    _saturation_qp(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_saturation_qp")),
    _dsaturation_qp_dvar(is_ad
                             ? nullptr
                             : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                                   "dPorousFlow_saturation_qp_dvar"))
{
}

template <bool is_ad>
void
PorousFlowDiffusivityMillingtonQuirkTempl<is_ad>::computeQpProperties()
{
  PorousFlowDiffusivityBaseTempl<is_ad>::computeQpProperties();

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _tortuosity[_qp][ph] =
        std::cbrt(_porosity_qp[_qp]) * std::pow(_saturation_qp[_qp][ph], 10.0 / 3.0);

    if (!is_ad)
      for (unsigned int var = 0; var < _num_var; ++var)
        (*_dtortuosity_dvar)[_qp][ph][var] = MetaPhysicL::raw_value(
            1.0 / 3.0 * std::cbrt(_porosity_qp[_qp]) *
            std::pow(_saturation_qp[_qp][ph], 7.0 / 3.0) *
            (_saturation_qp[_qp][ph] / _porosity_qp[_qp] * (*_dporosity_qp_dvar)[_qp][var] +
             10.0 * (*_dsaturation_qp_dvar)[_qp][ph][var]));
  }
}

template class PorousFlowDiffusivityMillingtonQuirkTempl<false>;
template class PorousFlowDiffusivityMillingtonQuirkTempl<true>;
