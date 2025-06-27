//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChurchillChuHTCFunctorMaterial.h"

registerMooseObject("HeatTransferApp", ChurchillChuHTCFunctorMaterial);
registerMooseObject("HeatTransferApp", ADChurchillChuHTCFunctorMaterial);

template <bool is_ad>
InputParameters
ChurchillChuHTCFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes a heat transfer coefficient using the Churchill-Chu "
                             "correlation for natural convection.");
  params.addRequiredParam<MooseFunctorName>("Pr", "Fluid Prandtl number functor");
  params.addRequiredParam<MooseFunctorName>("Gr", "Grashof number functor");
  params.addRequiredParam<MooseFunctorName>("k_fluid",
                                            "Fluid thermal conductivity functor [W/(m-K)]");
  params.addRequiredParam<Real>("diameter", "Cylinder diameter [m]");
  params.addRequiredParam<std::string>(
      "htc_name", "Name to give the heat transfer coefficient functor material property");

  return params;
}

template <bool is_ad>
ChurchillChuHTCFunctorMaterialTempl<is_ad>::ChurchillChuHTCFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _Pr(getFunctor<GenericReal<is_ad>>("Pr")),
    _Gr(getFunctor<GenericReal<is_ad>>("Gr")),
    _k_fluid(getFunctor<GenericReal<is_ad>>("k_fluid")),
    _diameter(getParam<Real>("diameter"))
{
  addFunctorProperty<GenericReal<is_ad>>(
      getParam<std::string>("htc_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto Pr = _Pr(r, t);
        const auto Gr = _Gr(r, t);
        const auto k_fluid = _k_fluid(r, t);
        const auto Ra = Gr * Pr;
        const auto numerator = 0.387 * std::pow(Ra, 1.0 / 6.0);
        const auto denominator = std::pow(1 + std::pow(0.559 / Pr, 9.0 / 16.0), 8.0 / 27.0);
        const auto root_Nu = 0.6 + numerator / denominator;
        const auto Nu = Utility::pow<2>(root_Nu);

        return Nu * k_fluid / _diameter;
      });
}

template class ChurchillChuHTCFunctorMaterialTempl<false>;
template class ChurchillChuHTCFunctorMaterialTempl<true>;
