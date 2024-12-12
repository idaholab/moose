//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVEnthalpyFunctorMaterial.h"
#include "NS.h" // Variable Term Names
#include "NavierStokesMethods.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", LinearFVEnthalpyFunctorMaterial);

InputParameters
LinearFVEnthalpyFunctorMaterial::validParams()
{
  auto params = FunctorMaterial::validParams();
  params.addClassDescription("Creates functor fluid properties using a (P, T) formulation");

  params.addRequiredParam<MooseFunctorName>(NS::pressure, "Pressure");
  params.addRequiredParam<MooseFunctorName>(NS::T_fluid, "Fluid temperature");
  params.addRequiredParam<MooseFunctorName>("h","Specific Enthalpy");

  // Please choose between providing a fluid properties
  params.addParam<UserObjectName>(NS::fluid, "Fluid properties functor userobject");
  // or the h_from_p_T and T_from_p_h functors.
  params.addParam<MooseFunctorName>("h_from_p_T_functor","User specified enthalpy from temperature functor");
  params.addParam<MooseFunctorName>("T_from_p_h_functor","User specified temperature from enthalpy functor");

  return params;
}

LinearFVEnthalpyFunctorMaterial::LinearFVEnthalpyFunctorMaterial(const InputParameters & params)
  : FunctorMaterial(params),
    _pressure(getFunctor<ADReal>(NS::pressure)),
    _T_fluid(getFunctor<ADReal>(NS::T_fluid)),
    _h(getFunctor<ADReal>("h")),
    _fluid(params.isParamValid(NS::fluid) ? &UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid) : nullptr),
    _h_from_p_T_functor(params.isParamValid("h_from_p_T_functor") ? &getFunctor<Real>("h_from_p_T_functor") : nullptr),
    _T_from_p_h_functor(params.isParamValid("T_from_p_h_functor") ? &getFunctor<Real>("T_from_p_h_functor") : nullptr)
{
  // Check parameters
  if (!( (_fluid && !_h_from_p_T_functor && !_T_from_p_h_functor) ||
         (!_fluid && _h_from_p_T_functor && _T_from_p_h_functor)))
    {
    mooseError(false,
                "For `LinearFVEnthalpyFunctorMaterial` , an unsupported combination of input parameters was given. Current"
                "supported combinations are either i) fp and no _h_from_p_T_functor and _T_from_p_h_functor, or ii) "
                "no fp and _h_from_p_T_functor and _T_from_p_h_functor.");
    }

  //
  // Set material properties functors
  //

  if (_fluid)
  {
    addFunctorProperty<Real>("h_from_p_T",
                                [this](const auto & r, const auto & t) -> Real
                                { return _fluid->h_from_p_T(raw_value(_pressure(r, t)), raw_value(_T_fluid(r, t))); });
    addFunctorProperty<Real>("T_from_p_h",
                                [this](const auto & r, const auto & t) -> Real
                                { return _fluid->T_from_p_h(raw_value(_pressure(r, t)), raw_value(_h(r, t))); });
  }
  else
  {
    addFunctorProperty<Real>("h_from_p_T",
                                [this](const auto & r, const auto & t) -> Real
                                { return (*_h_from_p_T_functor)(r, t); });
    addFunctorProperty<Real>("T_from_p_h",
                                [this](const auto & r, const auto & t) -> Real
                                { return (*_T_from_p_h_functor)(r, t); });
  }
}
