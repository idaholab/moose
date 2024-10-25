//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVkEpsilonViscosityFunctorMaterial.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObjectRenamed("NavierStokesApp",
                           INSFVkEpsilonViscosityMaterial,
                           "02/01/2025 00:00",
                           INSFVkEpsilonViscosityFunctorMaterial);
registerMooseObject("NavierStokesApp", INSFVkEpsilonViscosityFunctorMaterial);

InputParameters
INSFVkEpsilonViscosityFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes the turbulent dynamic viscosity given k and epsilon.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The liquid density.");
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "The turbulence kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::TKED,
                                            "The turbulent kinetic energy dissipation rate.");
  params.addParam<MooseFunctorName>("C_mu", 0.09, "C_mu closure parameter");
  return params;
}

INSFVkEpsilonViscosityFunctorMaterial::INSFVkEpsilonViscosityFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _rho(getFunctor<ADReal>(NS::density)),
    _k(getFunctor<ADReal>(NS::TKE)),
    _epsilon(getFunctor<ADReal>(NS::TKED)),
    _C_mu(getFunctor<ADReal>("C_mu")),
    _preserve_sparsity_pattern(_fe_problem.preserveMatrixSparsityPattern())
{
  addFunctorProperty<ADReal>(
      NS::mu_t,
      [this](const auto & r, const auto & t) -> ADReal
      {
        if (_preserve_sparsity_pattern)
          return std::max(NS::mu_t_low_limit + 0 * _k(r, t) * _epsilon(r, t),
                          _C_mu(r, t) * _rho(r, t) * Utility::pow<2>(_k(r, t)) /
                              std::max(NS::epsilon_low_limit, _epsilon(r, t)));
        else
          return std::max(NS::mu_t_low_limit,
                          _C_mu(r, t) * _rho(r, t) * Utility::pow<2>(_k(r, t)) /
                              std::max(NS::epsilon_low_limit, _epsilon(r, t)));
      });
}
