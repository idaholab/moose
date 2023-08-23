//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FinEfficiencyFunctorMaterial.h"

registerMooseObject("HeatConductionApp", FinEfficiencyFunctorMaterial);
registerMooseObject("HeatConductionApp", ADFinEfficiencyFunctorMaterial);

template <bool is_ad>
InputParameters
FinEfficiencyFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes fin efficiency.");
  params.addRequiredParam<MooseFunctorName>("heat_transfer_coefficient",
                                            "Heat transfer coefficient functor [W/(m^2-K)]");
  params.addRequiredParam<MooseFunctorName>("thermal_conductivity",
                                            "Thermal conductivity functor [W/(m-K)]");
  params.addRequiredParam<MooseFunctorName>("fin_height", "Fin height functor [m]");
  params.addRequiredParam<MooseFunctorName>(
      "fin_perimeter_area_ratio",
      "Functor for the ratio of the fin perimeter to its cross-sectional area [1/m]");
  params.addParam<MooseFunctorName>("fin_efficiency_name",
                                    "fin_efficiency",
                                    "Name to give the fin efficiency functor material property");
  return params;
}

template <bool is_ad>
FinEfficiencyFunctorMaterialTempl<is_ad>::FinEfficiencyFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _htc(getFunctor<GenericReal<is_ad>>("heat_transfer_coefficient")),
    _k(getFunctor<GenericReal<is_ad>>("thermal_conductivity")),
    _L(getFunctor<GenericReal<is_ad>>("fin_height")),
    _P_over_Ac(getFunctor<GenericReal<is_ad>>("fin_perimeter_area_ratio"))
{
  addFunctorProperty<GenericReal<is_ad>>(
      getParam<MooseFunctorName>("fin_efficiency_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto htc = _htc(r, t);
        const auto k = _k(r, t);
        const auto L = _L(r, t);
        const auto P_over_Ac = _P_over_Ac(r, t);
        const auto m = std::sqrt(htc * P_over_Ac / k);
        const auto mL = m * L;
        return std::tanh(mL) / mL;
      });
}

template class FinEfficiencyFunctorMaterialTempl<false>;
template class FinEfficiencyFunctorMaterialTempl<true>;
