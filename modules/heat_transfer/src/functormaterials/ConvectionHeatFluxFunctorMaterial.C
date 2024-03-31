//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvectionHeatFluxFunctorMaterial.h"

registerMooseObject("HeatTransferApp", ConvectionHeatFluxFunctorMaterial);
registerMooseObject("HeatTransferApp", ADConvectionHeatFluxFunctorMaterial);

template <bool is_ad>
InputParameters
ConvectionHeatFluxFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes a convection heat flux.");
  params.addRequiredParam<MooseFunctorName>("htc", "Heat transfer coefficient functor");
  params.addRequiredParam<MooseFunctorName>("T_solid", "Solid temperature functor");
  params.addRequiredParam<MooseFunctorName>("T_fluid", "Fluid temperature functor");
  params.addRequiredParam<bool>(
      "to_solid",
      "If true, compute the heat flux to the solid; else, compute the heat flux to the fluid");
  params.addRequiredParam<std::string>("heat_flux_name",
                                       "Name to give the heat flux functor material property");
  return params;
}

template <bool is_ad>
ConvectionHeatFluxFunctorMaterialTempl<is_ad>::ConvectionHeatFluxFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _htc(getFunctor<GenericReal<is_ad>>("htc")),
    _T_solid(getFunctor<GenericReal<is_ad>>("T_solid")),
    _T_fluid(getFunctor<GenericReal<is_ad>>("T_fluid")),
    _sign(getParam<bool>("to_solid") ? 1.0 : -1.0)
{
  addFunctorProperty<GenericReal<is_ad>>(
      getParam<std::string>("heat_flux_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto htc = _htc(r, t);
        const auto T_solid = _T_solid(r, t);
        const auto T_fluid = _T_fluid(r, t);
        return _sign * htc * (T_fluid - T_solid);
      });
}

template class ConvectionHeatFluxFunctorMaterialTempl<false>;
template class ConvectionHeatFluxFunctorMaterialTempl<true>;
