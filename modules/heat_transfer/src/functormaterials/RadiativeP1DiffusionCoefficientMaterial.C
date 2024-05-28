//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadiativeP1DiffusionCoefficientMaterial.h"

registerMooseObject("HeatTransferApp", RadiativeP1DiffusionCoefficientMaterial);
registerMooseObject("HeatTransferApp", ADRadiativeP1DiffusionCoefficientMaterial);

template <bool is_ad>
InputParameters
RadiativeP1DiffusionCoefficientMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes the P1 diffusion coefficient from the opacity and effective "
                             "scattering cross section.");
  params.addRequiredParam<MooseFunctorName>("opacity", "Opacity.");
  params.addParam<MooseFunctorName>("sigma_scat_eff", 0.0, "Effective P1 scatterig cross section.");
  params.addRequiredParam<std::string>("P1_diff_coef_name",
                                       "Name given to the P1 diffusion coefficient.");
  return params;
}

template <bool is_ad>
RadiativeP1DiffusionCoefficientMaterialTempl<is_ad>::RadiativeP1DiffusionCoefficientMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _opacity(getFunctor<GenericReal<is_ad>>("opacity")),
    _sigma_scat_eff(getFunctor<GenericReal<is_ad>>("sigma_scat_eff"))
{
  addFunctorProperty<GenericReal<is_ad>>(
      getParam<std::string>("P1_diff_coef_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto kappa = _opacity(r, t);
        const auto sigma_scat = _sigma_scat_eff(r, t);
        return 1.0 / (3.0 * kappa + sigma_scat);
      });
}

template class RadiativeP1DiffusionCoefficientMaterialTempl<false>;
template class RadiativeP1DiffusionCoefficientMaterialTempl<true>;
