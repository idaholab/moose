//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FinEnhancementFactorFunctorMaterial.h"

registerMooseObject("HeatConductionApp", FinEnhancementFactorFunctorMaterial);
registerMooseObject("HeatConductionApp", ADFinEnhancementFactorFunctorMaterial);

template <bool is_ad>
InputParameters
FinEnhancementFactorFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes a heat transfer enhancement factor for fins.");
  params.addRequiredParam<MooseFunctorName>("fin_efficiency", "Fin efficiency functor");
  params.addRequiredParam<MooseFunctorName>(
      "fin_area_fraction",
      "Functor for the fraction of the total surface area corresponding to fins");
  params.addRequiredParam<MooseFunctorName>(
      "area_increase_factor",
      "Functor for the ratio of the total surface area with fins to the base surface area");
  params.addParam<MooseFunctorName>(
      "fin_enhancement_factor_name",
      "fin_enhancement_factor",
      "Name to give the fin enhancement factor functor material property");
  return params;
}

template <bool is_ad>
FinEnhancementFactorFunctorMaterialTempl<is_ad>::FinEnhancementFactorFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _fin_efficiency(getFunctor<GenericReal<is_ad>>("fin_efficiency")),
    _fin_area_fraction(getFunctor<GenericReal<is_ad>>("fin_area_fraction")),
    _area_increase_factor(getFunctor<GenericReal<is_ad>>("area_increase_factor"))
{
  addFunctorProperty<GenericReal<is_ad>>(
      getParam<MooseFunctorName>("fin_enhancement_factor_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto fin_efficiency = _fin_efficiency(r, t);
        const auto fin_area_fraction = _fin_area_fraction(r, t);
        const auto total_efficiency = 1.0 - (1.0 - fin_efficiency) * fin_area_fraction;
        const auto area_increase_factor = _area_increase_factor(r, t);
        return total_efficiency * area_increase_factor;
      });
}

template class FinEnhancementFactorFunctorMaterialTempl<false>;
template class FinEnhancementFactorFunctorMaterialTempl<true>;
