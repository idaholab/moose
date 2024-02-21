//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVv2fViscosityFunctorMaterial.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVv2fViscosityFunctorMaterial);
registerMooseObject("NavierStokesApp", INSFVv2fViscosityFunctorMaterialReal);

template <bool is_ad>
InputParameters
INSFVv2fViscosityFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();

  // Class description
  params.addClassDescription("Calculates the turbulent viscosity according to the v2f model.");

  // Coupled turbulent variables
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::TKED,
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::TV2, "Coupled turbulent wall normal fluctuations.");

  // Coupled thermophysical variables
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");

  // Closure parameters
  params.addParam<Real>("C_mu_2", 0.22, "Coupled turbulent viscosity closure.");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent viscosity closure.");
  return params;
}

template <bool is_ad>
INSFVv2fViscosityFunctorMaterialTempl<is_ad>::INSFVv2fViscosityFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _k(getFunctor<ADReal>(NS::TKE)),
    _epsilon(getFunctor<ADReal>(NS::TKED)),
    _v2(getFunctor<ADReal>(NS::TV2)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _C_mu_2(getParam<Real>("C_mu_2")),
    _C_mu(getParam<Real>("C_mu"))
{
  addFunctorProperty<GenericReal<is_ad>>(
      NS::mu_t,
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto time_scale_keps = _k(r, t) / _epsilon(r, t);
        const auto time_scale =
            std::max(time_scale_keps, 6 * std::sqrt((_mu(r, t) / _rho(r, t)) / _epsilon(r, t)));
        const Real switcher =
            (_C_mu * _k(r, t) * time_scale_keps < _C_mu_2 * _v2(r, t) * time_scale) ? 1.0 : 0.0;
        const auto mu_t_ad = _C_mu * _k(r, t) * time_scale_keps * switcher +
                             _C_mu_2 * _v2(r, t) * time_scale * (1. - switcher);
        if constexpr (!is_ad)
          return MetaPhysicL::raw_value(mu_t_ad);
        else
          return mu_t_ad;
      });
}
