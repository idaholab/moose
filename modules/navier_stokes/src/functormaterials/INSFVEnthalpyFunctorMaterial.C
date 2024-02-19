//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVEnthalpyFunctorMaterial.h"
#include "NS.h"

registerMooseObjectRenamed("NavierStokesApp",
                           INSFVEnthalpyMaterial,
                           "02/01/2024 00:00",
                           INSFVEnthalpyFunctorMaterial);
registerMooseObject("NavierStokesApp", INSFVEnthalpyFunctorMaterial);

InputParameters
INSFVEnthalpyFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription(
      "This is the material class used to compute enthalpy for the "
      "incompressible/weakly-compressible finite-volume implementation of the Navier-Stokes "
      "equations. Note that this class assumes that cp is a constant");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The value for the density");
  params.addRequiredParam<MooseFunctorName>("temperature", "the temperature");
  params.addParam<MooseFunctorName>(
      NS::cp, NS::cp, "The constant value for the specific heat capacity");
  params.addParam<MooseFunctorName>(
      NS::enthalpy_density, NS::enthalpy_density, "the name of the (extensive) enthalpy");
  params.addParam<MooseFunctorName>(
      NS::specific_enthalpy, NS::specific_enthalpy, "the name of the specific enthalpy");
  return params;
}

INSFVEnthalpyFunctorMaterial::INSFVEnthalpyFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _rho(getFunctor<ADReal>(NS::density)),
    _temperature(getFunctor<ADReal>("temperature")),
    _cp(getFunctor<ADReal>(NS::cp))
{
  const auto & rho_h =
      addFunctorProperty<ADReal>(NS::enthalpy_density,
                                 [this](const auto & r, const auto & t)
                                 { return _rho(r, t) * _cp(r, t) * _temperature(r, t); });

  const auto & h = addFunctorProperty<ADReal>(NS::specific_enthalpy,
                                              [this](const auto & r, const auto & t)
                                              { return _cp(r, t) * _temperature(r, t); });

  addFunctorProperty<ADReal>(NS::time_deriv(getParam<MooseFunctorName>(NS::specific_enthalpy)),
                             [this](const auto & r, const auto & t)
                             { return _cp(r, t) * _temperature.dot(r, t); });

  addFunctorProperty<ADReal>(
      "rho_cp_temp", [&rho_h](const auto & r, const auto & t) -> ADReal { return rho_h(r, t); });

  addFunctorProperty<ADReal>("cp_temp",
                             [&h](const auto & r, const auto & t) -> ADReal { return h(r, t); });
}
