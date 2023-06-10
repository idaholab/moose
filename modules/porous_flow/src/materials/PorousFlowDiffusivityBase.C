//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDiffusivityBase.h"

template <bool is_ad>
InputParameters
PorousFlowDiffusivityBaseTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addRequiredParam<std::vector<Real>>(
      "diffusion_coeff",
      "List of diffusion coefficients.  Order is i) component 0 in phase 0; ii) "
      "component 1 in phase 0 ...; component 0 in phase 1; ... component k in "
      "phase n (m^2/s");
  params.addClassDescription("Base class for effective diffusivity for each phase");
  params.set<bool>("at_nodes") = false;
  params.addPrivateParam<std::string>("pf_material_type", "diffusivity");
  return params;
}

template <bool is_ad>
PorousFlowDiffusivityBaseTempl<is_ad>::PorousFlowDiffusivityBaseTempl(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),

    _tortuosity(declareGenericProperty<std::vector<Real>, is_ad>("PorousFlow_tortuosity_qp")),
    _dtortuosity_dvar(
        is_ad ? nullptr
              : &declareProperty<std::vector<std::vector<Real>>>("dPorousFlow_tortuosity_qp_dvar")),
    _diffusion_coeff(
        declareProperty<std::vector<std::vector<Real>>>("PorousFlow_diffusion_coeff_qp")),
    _ddiffusion_coeff_dvar(is_ad ? nullptr
                                 : &declareProperty<std::vector<std::vector<std::vector<Real>>>>(
                                       "dPorousFlow_diffusion_coeff_qp_dvar")),
    _input_diffusion_coeff(getParam<std::vector<Real>>("diffusion_coeff"))
{
  // Also, the number of diffusion coefficients must be equal to the num_phases * num_components
  if (_input_diffusion_coeff.size() != _num_phases * _num_components)
    this->template paramError(
        "diffusion_coeff",
        "The number of diffusion coefficients entered is not equal to the number of phases "
        "multiplied by the number of fluid components");

  if (_nodal_material == true)
    mooseError("PorousFlowRelativeDiffusivity classes are only defined for at_nodes = false");
}

template <bool is_ad>
void
PorousFlowDiffusivityBaseTempl<is_ad>::computeQpProperties()
{
  _diffusion_coeff[_qp].resize(_num_phases);
  _tortuosity[_qp].resize(_num_phases);

  if (!is_ad)
  {
    (*_ddiffusion_coeff_dvar)[_qp].resize(_num_phases);
    (*_dtortuosity_dvar)[_qp].resize(_num_phases);
  }

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _diffusion_coeff[_qp][ph].resize(_num_components);

    if (!is_ad)
    {
      (*_ddiffusion_coeff_dvar)[_qp][ph].resize(_num_components);
      (*_dtortuosity_dvar)[_qp][ph].assign(_num_var, 0.0);
    }

    for (unsigned int comp = 0; comp < _num_components; ++comp)
    {
      _diffusion_coeff[_qp][ph][comp] = _input_diffusion_coeff[ph + comp];

      if (!is_ad)
        (*_ddiffusion_coeff_dvar)[_qp][ph][comp].assign(_num_var, 0.0);
    }
  }
}

template class PorousFlowDiffusivityBaseTempl<false>;
template class PorousFlowDiffusivityBaseTempl<true>;
