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
  params.addRequiredParam<MooseFunctorName>("htc", "Heat transfer coefficient functor");
  params.addRequiredParam<MooseFunctorName>("T_solid", "Solid temperature functor");
  params.addRequiredParam<MooseFunctorName>("T_fluid", "Fluid temperature functor");
  params.addRequiredParam<std::string>("heat_flux_name",
                                       "Name to give the heat flux functor material property");
  return params;
}

template <bool is_ad>
ChurchillChuHTCFunctorMaterialTempl<is_ad>::ChurchillChuHTCFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _htc(getFunctor<GenericReal<is_ad>>("htc")),
    _T_solid(getFunctor<GenericReal<is_ad>>("T_solid")),
    _T_fluid(getFunctor<GenericReal<is_ad>>("T_fluid"))
{
  addFunctorProperty<GenericReal<is_ad>>(
      getParam<std::string>("heat_flux_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto htc = _htc(r, t);
        const auto T_solid = _T_solid(r, t);
        const auto T_fluid = _T_fluid(r, t);
        return htc * (T_solid - T_fluid);
      });
}

template class ChurchillChuHTCFunctorMaterialTempl<false>;
template class ChurchillChuHTCFunctorMaterialTempl<true>;
