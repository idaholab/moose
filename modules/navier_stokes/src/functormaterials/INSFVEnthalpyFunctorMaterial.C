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
#include "SinglePhaseFluidProperties.h"

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
      "equations.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The value for the density");
  params.addRequiredParam<MooseFunctorName>("temperature", "the temperature");
  params.addParam<MooseFunctorName>(
      NS::cp, NS::cp, "The constant value for the specific heat capacity");
  params.addParam<MooseFunctorName>(
      NS::enthalpy_density, NS::enthalpy_density, "the name of the (extensive) enthalpy");
  params.addParam<MooseFunctorName>(
      NS::specific_enthalpy, NS::specific_enthalpy, "the name of the specific enthalpy");

  // To handle non constant cp
  params.addParam<bool>("assumed_constant_cp", true, "Whether to assume cp is constant");
  params.addParam<UserObjectName>(
      NS::fluid, "Fluid properties, to be used when cp is not constant to compute enthalpy");
  params.addParam<MooseFunctorName>(
      NS::pressure, "Pressure functor, to be used when cp is not constant to compute enthalpy");
  params.addParam<MooseFunctorName>(
      NS::specific_enthalpy + "_in",
      "Specific enthalpy functor, to be used when cp is not constant to compute the enthalpy, as "
      "an alternative to using a 'fp' FluidProperties object");

  return params;
}

INSFVEnthalpyFunctorMaterial::INSFVEnthalpyFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _assumed_constant_cp(getParam<bool>("assumed_constant_cp")),
    _fp(isParamValid(NS::fluid)
            ? &UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)
            : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _temperature(getFunctor<ADReal>("temperature")),
    _pressure(isParamValid("pressure") ? &getFunctor<ADReal>("pressure") : nullptr),
    _cp(getFunctor<ADReal>(NS::cp)),
    _h(isParamValid(NS::specific_enthalpy + "_in")
           ? &getFunctor<ADReal>(NS::specific_enthalpy + "_in")
           : nullptr)
{
  // We have to use a warning because fp is often in the global parameters
  if (_assumed_constant_cp && _fp)
    paramWarning(
        "fp", "No need to specify fluid properties if assuming the specific enthalpy is constant");
  if (!_assumed_constant_cp && ((!_fp || !_pressure) && !_h))
    paramError("fp",
               "Must specify both fluid properties and pressure or an enthalpy functor if not "
               "assuming the specific enthalpy is constant");

  if (_assumed_constant_cp)
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
  else if (_h)
  {
    addFunctorProperty<ADReal>(NS::enthalpy_density,
                               [this](const auto & r, const auto & t)
                               { return _rho(r, t) * (*_h)(r, t); });

    addFunctorProperty<ADReal>(NS::time_deriv(getParam<MooseFunctorName>(NS::specific_enthalpy)),
                               [this](const auto & r, const auto & t) { return _h->dot(r, t); });
  }
  else
  {
    addFunctorProperty<ADReal>(
        NS::enthalpy_density,
        [this](const auto & r, const auto & t)
        { return _rho(r, t) * _fp->h_from_p_T((*_pressure)(r, t), _temperature(r, t)); });

    addFunctorProperty<ADReal>(NS::specific_enthalpy,
                               [this](const auto & r, const auto & t)
                               { return _fp->h_from_p_T((*_pressure)(r, t), _temperature(r, t)); });

    addFunctorProperty<ADReal>(
        NS::time_deriv(getParam<MooseFunctorName>(NS::specific_enthalpy)),
        [this](const auto & r, const auto & t)
        {
          Real h, dh_dp, dh_dT;
          _fp->h_from_p_T((*_pressure)(r, t).value(), _temperature(r, t).value(), h, dh_dp, dh_dT);
          return dh_dT * _temperature.dot(r, t) + dh_dp * _pressure->dot(r, t);
        });
  }
}
