//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVkEpsilonViscosityMaterial.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVkEpsilonViscosityMaterial);

InputParameters
INSFVkEpsilonViscosityMaterial::validParams()
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

INSFVkEpsilonViscosityMaterial::INSFVkEpsilonViscosityMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _rho(getFunctor<ADReal>(NS::density)),
    _k(getFunctor<ADReal>(NS::TKE)),
    _epsilon(getFunctor<ADReal>(NS::TKED)),
    _C_mu(getFunctor<ADReal>("C_mu"))
{
  addFunctorProperty<ADReal>(
      "mu_t",
      [this](const auto & r, const auto & t) -> ADReal
      { return _C_mu(r, t) * _rho(r, t) * Utility::pow<2>(_k(r, t)) / _epsilon(r, t); });
}
