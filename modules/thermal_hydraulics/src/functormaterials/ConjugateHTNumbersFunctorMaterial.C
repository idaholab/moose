//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConjugateHTNumbersFunctorMaterial.h"
#include "HeatTransferUtils.h"
#include "SinglePhaseFluidProperties.h"
#include "PhysicalConstants.h"

registerMooseObject("ThermalHydraulicsApp", ConjugateHTNumbersFunctorMaterial);
registerMooseObject("ThermalHydraulicsApp", ADConjugateHTNumbersFunctorMaterial);

template <bool is_ad>
InputParameters
ConjugateHTNumbersFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();

  params.addClassDescription(
      "Computes several non-dimensional numbers for conjugate heat transfer.");

  params.addRequiredParam<MooseFunctorName>("p_fluid", "Fluid pressure functor");
  params.addRequiredParam<MooseFunctorName>("T_fluid", "Fluid temperature functor");
  params.addRequiredParam<MooseFunctorName>("T_solid", "Solid temperature functor");
  params.addRequiredParam<Real>("length", "Characteristic length [m]");
  params.addRequiredParam<std::string>("Pr_name",
                                       "Name to give the Prandtl number functor material property");
  params.addRequiredParam<std::string>("Gr_name",
                                       "Name to give the Grashof number functor material property");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The SinglePhaseFluidProperties object for the fluid");

  return params;
}

template <bool is_ad>
ConjugateHTNumbersFunctorMaterialTempl<is_ad>::ConjugateHTNumbersFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _p_fluid(getFunctor<GenericReal<is_ad>>("p_fluid")),
    _T_fluid(getFunctor<GenericReal<is_ad>>("T_fluid")),
    _T_solid(getFunctor<GenericReal<is_ad>>("T_solid")),
    _length(getParam<Real>("length")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
  addFunctorProperty<GenericReal<is_ad>>(
      getParam<std::string>("Pr_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto p = _p_fluid(r, t);
        const auto T = _T_fluid(r, t);
        const auto cp = _fp.cp_from_p_T(p, T);
        const auto mu = _fp.mu_from_p_T(p, T);
        const auto k = _fp.k_from_p_T(p, T);

        return HeatTransferUtils::prandtl(cp, mu, k);
      });

  addFunctorProperty<GenericReal<is_ad>>(
      getParam<std::string>("Gr_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto p_fluid = _p_fluid(r, t);
        const auto T_fluid = _T_fluid(r, t);
        const auto T_solid = _T_solid(r, t);
        const auto beta_fluid = _fp.beta_from_p_T(p_fluid, T_fluid);
        const auto rho_fluid = _fp.rho_from_p_T(p_fluid, T_fluid);
        const auto mu_fluid = _fp.mu_from_p_T(p_fluid, T_fluid);

        return HeatTransferUtils::grashof(beta_fluid,
                                          T_solid,
                                          T_fluid,
                                          _length,
                                          rho_fluid,
                                          mu_fluid,
                                          PhysicalConstants::acceleration_of_gravity);
      });
}

template class ConjugateHTNumbersFunctorMaterialTempl<false>;
template class ConjugateHTNumbersFunctorMaterialTempl<true>;
