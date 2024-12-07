//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMushyPorousFrictionFunctorMaterial.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMushyPorousFrictionFunctorMaterial);
registerMooseObjectRenamed("NavierStokesApp",
                           INSFVMushyPorousFrictionMaterial,
                           "08/01/2024 00:00",
                           INSFVMushyPorousFrictionFunctorMaterial);

InputParameters
INSFVMushyPorousFrictionFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription(
      "Computes the mushy zone porous resistance for solidification/melting problems.");
  params.addRequiredParam<MooseFunctorName>("liquid_fraction", "Liquid Fraction Functor.");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "The liquid dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("rho_l", "The liquid density (not the mixture one).");
  params.addParam<MooseFunctorName>(
      "dendrite_spacing_scaling", "1e-4", "The dendrite spacing scaling.");
  params.addParam<MooseFunctorName>(
      "Darcy_coef_name", "Darcy_coefficient", "Name of the Darcy friction coefficient");
  params.addParam<MooseFunctorName>("Forchheimer_coef_name",
                                    "Forchheimer_coefficient",
                                    "Name of the Forchheimer friction coefficient");

  return params;
}

INSFVMushyPorousFrictionFunctorMaterial::INSFVMushyPorousFrictionFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _fl(getFunctor<ADReal>("liquid_fraction")),
    _mu(getFunctor<ADReal>(NS::mu)),
    _rho_l(getFunctor<ADReal>("rho_l")),
    _dendrite_spacing_scaling(getFunctor<ADReal>("dendrite_spacing_scaling"))
{

  addFunctorProperty<ADReal>(
      getParam<MooseFunctorName>("Darcy_coef_name"),
      [this](const auto & r, const auto & t) -> ADReal
      {
        constexpr Real epsilon = 1e-15; // prevents explosion of sqrt(x) derivative to infinity
        const auto fs = 1.0 - _fl(r, t);
        mooseAssert(_dendrite_spacing_scaling(r, t) > 0,
                    "Dendrite spacing scaling should be positive!");
        const auto cs = _c / Utility::pow<2>(_dendrite_spacing_scaling(r, t));
        const auto Fk = 0.5 + std::atan(_s * (fs - _fs_crit)) / libMesh::pi;
        const auto K =
            Utility::pow<3>(_fl(r, t)) / ((Utility::pow<2>(fs) + epsilon) * Fk * cs) + epsilon;
        return _mu(r, t) / K;
      });

  addFunctorProperty<ADReal>(
      getParam<MooseFunctorName>("Forchheimer_coef_name"),
      [this](const auto & r, const auto & t) -> ADReal
      {
        constexpr Real epsilon = 1e-15; // prevents explosion of sqrt(x) derivative to infinity
        const auto fs = 1.0 - _fl(r, t);
        mooseAssert(_dendrite_spacing_scaling(r, t) > 0,
                    "Dendrite spacing scaling should be positive!");
        const auto cs = _c / Utility::pow<2>(_dendrite_spacing_scaling(r, t));
        const auto Fk = 0.5 + std::atan(_s * (fs - _fs_crit)) / libMesh::pi;
        const auto K =
            Utility::pow<3>(_fl(r, t)) / ((Utility::pow<2>(fs) + epsilon) * Fk * cs) + epsilon;
        return _forchheimer_coef * _rho_l(r, t) / std::sqrt(K);
      });
}
