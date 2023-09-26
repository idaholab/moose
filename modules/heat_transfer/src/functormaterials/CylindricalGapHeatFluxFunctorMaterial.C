//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CylindricalGapHeatFluxFunctorMaterial.h"
#include "HeatTransferModels.h"

registerMooseObject("HeatConductionApp", CylindricalGapHeatFluxFunctorMaterial);
registerMooseObject("HeatConductionApp", ADCylindricalGapHeatFluxFunctorMaterial);

template <bool is_ad>
InputParameters
CylindricalGapHeatFluxFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();

  params.addClassDescription("Computes cylindrical gap heat flux due to conduction and radiation.");

  params.addRequiredParam<MooseFunctorName>("r_inner", "Inner surface radius functor [m]");
  params.addRequiredParam<MooseFunctorName>("r_outer", "Outer surface radius functor [m]");
  params.addRequiredParam<MooseFunctorName>("T_inner", "Inner surface temperature functor [K]");
  params.addRequiredParam<MooseFunctorName>("T_outer", "Outer surface temperature functor [K]");
  params.addRequiredParam<MooseFunctorName>("k_gap", "Gap thermal conductivity [W/(m-K)]");
  params.addRequiredParam<MooseFunctorName>("emissivity_inner", "Inner surface emissivity functor");
  params.addRequiredParam<MooseFunctorName>("emissivity_outer", "Outer surface emissivity functor");

  params.addParam<MooseFunctorName>(
      "conduction_heat_flux_name",
      "conduction_heat_flux",
      "Name to give the conduction heat flux functor material property");
  params.addParam<MooseFunctorName>(
      "radiation_heat_flux_name",
      "radiation_heat_flux",
      "Name to give the radiation heat flux functor material property");
  params.addParam<MooseFunctorName>("total_heat_flux_name",
                                    "total_heat_flux",
                                    "Name to give the total heat flux functor material property");

  return params;
}

template <bool is_ad>
CylindricalGapHeatFluxFunctorMaterialTempl<is_ad>::CylindricalGapHeatFluxFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _r_inner(getFunctor<GenericReal<is_ad>>("r_inner")),
    _r_outer(getFunctor<GenericReal<is_ad>>("r_outer")),
    _T_inner(getFunctor<GenericReal<is_ad>>("T_inner")),
    _T_outer(getFunctor<GenericReal<is_ad>>("T_outer")),
    _k_gap(getFunctor<GenericReal<is_ad>>("k_gap")),
    _emiss_inner(getFunctor<GenericReal<is_ad>>("emissivity_inner")),
    _emiss_outer(getFunctor<GenericReal<is_ad>>("emissivity_outer"))
{
  addFunctorProperty<GenericReal<is_ad>>(
      getParam<MooseFunctorName>("conduction_heat_flux_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad> {
        return HeatTransferModels::cylindricalGapConductionHeatFlux(
            _k_gap(r, t), _r_inner(r, t), _r_outer(r, t), _T_inner(r, t), _T_outer(r, t));
      });

  addFunctorProperty<GenericReal<is_ad>>(
      getParam<MooseFunctorName>("radiation_heat_flux_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad> {
        return HeatTransferModels::cylindricalGapRadiationHeatFlux(_r_inner(r, t),
                                                                   _r_outer(r, t),
                                                                   _emiss_inner(r, t),
                                                                   _emiss_outer(r, t),
                                                                   _T_inner(r, t),
                                                                   _T_outer(r, t));
      });

  addFunctorProperty<GenericReal<is_ad>>(
      getParam<MooseFunctorName>("total_heat_flux_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad> {
        return HeatTransferModels::cylindricalGapConductionHeatFlux(
                   _k_gap(r, t), _r_inner(r, t), _r_outer(r, t), _T_inner(r, t), _T_outer(r, t)) +
               HeatTransferModels::cylindricalGapRadiationHeatFlux(_r_inner(r, t),
                                                                   _r_outer(r, t),
                                                                   _emiss_inner(r, t),
                                                                   _emiss_outer(r, t),
                                                                   _T_inner(r, t),
                                                                   _T_outer(r, t));
      });
}

template class CylindricalGapHeatFluxFunctorMaterialTempl<false>;
template class CylindricalGapHeatFluxFunctorMaterialTempl<true>;
