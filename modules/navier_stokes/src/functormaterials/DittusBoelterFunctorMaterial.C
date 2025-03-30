//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DittusBoelterFunctorMaterial.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", DittusBoelterFunctorMaterial);
registerMooseObject("NavierStokesApp", ADDittusBoelterFunctorMaterial);

template <bool is_ad>
InputParameters
DittusBoelterFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addRequiredParam<MooseFunctorName>("Hw", "Heat transfer coefficient material property");
  params.addRequiredParam<MooseFunctorName>("D_h", "Hydraulic diameter");
  params.addRequiredParam<MooseFunctorName>("k", "Heat conductivity of the fluid");
  params.addRequiredParam<MooseFunctorName>(NS::T_fluid, "Fluid temperature");
  params.addRequiredParam<MooseFunctorName>("T_wall", "Wall temperature");
  params.addRequiredParam<MooseFunctorName>(NS::Prandtl, "The Prandtl number");
  params.addRequiredParam<MooseFunctorName>(NS::Reynolds, "The Reynolds number");
  params.addClassDescription(
      "Computes wall heat transfer coefficient using Dittus-Boelter equation");
  return params;
}

template <bool is_ad>
DittusBoelterFunctorMaterialTempl<is_ad>::DittusBoelterFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _D_h(getFunctor<GenericReal<is_ad>>("D_h")),
    _k(getFunctor<GenericReal<is_ad>>("k")),
    _T(getFunctor<GenericReal<is_ad>>(NS::T_fluid)),
    _T_wall(getFunctor<GenericReal<is_ad>>("T_wall")),
    _prandtl(getFunctor<GenericReal<is_ad>>(NS::Prandtl)),
    _reynolds(getFunctor<GenericReal<is_ad>>(NS::Reynolds))
{
  addFunctorProperty<GenericReal<is_ad>>(
      "Hw",
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const Real n =
            (MetaPhysicL::raw_value(_T(r, t)) < MetaPhysicL::raw_value(_T_wall(r, t))) ? 0.4 : 0.3;
        const auto Re = _reynolds(r, t);
        if (const auto raw_Re = MetaPhysicL::raw_value(Re); raw_Re < 1e4 || raw_Re > 1.2e5)
          flagSolutionWarning(
              "Reynolds number out of the range of validity of the Dittus-Boelter correlation");
        const auto Pr = _prandtl(r, t);
        if (const auto raw_Pr = MetaPhysicL::raw_value(Pr); raw_Pr < 0.7 || raw_Pr > 160)
          flagSolutionWarning(
              "Prandtl number out of the range of validity of the Dittus-Boelter correlation");
        const auto Nu = 0.023 * std::pow(Re, 0.8) * std::pow(Pr, n);
        return NS::wallHeatTransferCoefficient(Nu, _k(r, t), _D_h(r, t));
      });
}

template class DittusBoelterFunctorMaterialTempl<false>;
template class DittusBoelterFunctorMaterialTempl<true>;
