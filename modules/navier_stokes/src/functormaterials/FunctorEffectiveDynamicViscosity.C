//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorEffectiveDynamicViscosity.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", FunctorEffectiveDynamicViscosity);
registerMooseObject("NavierStokesApp", ADFunctorEffectiveDynamicViscosity);

template <bool is_ad>
InputParameters
FunctorEffectiveDynamicViscosityTempl<is_ad>::validParams()
{
  auto params = FunctorMaterial::validParams();
  params.addClassDescription(
      "Computes the effective dynamic viscosity mu_eff = mu + mu_t / factor");
  params.addRequiredParam<MooseFunctorName>(
      "property_name", "Name of the functor for the effective dynamic viscosity");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Name of the dynamic viscosity functor");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "Name of the turbulent viscosity functor");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t + "_inverse_factor",
                                            "Factor dividing the turbulent viscosity functor");
  params.addParam<Real>(NS::mu_t + "_extra_inverse_factor",
                        1.,
                        "Additional factor dividing the turbulent viscosity functor");

  return params;
}

template <bool is_ad>
FunctorEffectiveDynamicViscosityTempl<is_ad>::FunctorEffectiveDynamicViscosityTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _mu(getFunctor<GenericReal<is_ad>>(NS::mu)),
    _mu_t(getFunctor<GenericReal<is_ad>>(NS::mu_t)),
    _scale_factor(getFunctor<GenericReal<is_ad>>(NS::mu_t + "_inverse_factor")),
    _scale_factor_real(getParam<Real>(NS::mu_t + "_extra_inverse_factor"))
{
  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());
  addFunctorProperty<GenericReal<is_ad>>(
      getParam<MooseFunctorName>("property_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      { return _mu(r, t) + _mu_t(r, t) / _scale_factor(r, t) / _scale_factor_real; },
      clearance_schedule);
}

template class FunctorEffectiveDynamicViscosityTempl<false>;
template class FunctorEffectiveDynamicViscosityTempl<true>;
