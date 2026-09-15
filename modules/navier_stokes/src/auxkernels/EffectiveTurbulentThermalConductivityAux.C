//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EffectiveTurbulentThermalConductivityAux.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", EffectiveTurbulentThermalConductivityAux);

InputParameters
EffectiveTurbulentThermalConductivityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Calculates the effective turbulent thermal heat conductivity.");
  params.addRequiredParam<MooseFunctorName>(NS::cp, "Specific Heat Capacity functor");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "The turbulent viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::k, "The thermal conductivity.");
  params.addParam<MooseFunctorName>("Pr_t", 0.9, "Turbulent Prandtl number functor.");
  params.addParam<bool>(
      "turbulent_thermal_conductivity", false, "Whether to compute the turbulent thermal conductivity in the expression");
  return params;
}

EffectiveTurbulentThermalConductivityAux::EffectiveTurbulentThermalConductivityAux(const InputParameters & params)
  : AuxKernel(params),
    _cp(getFunctor<Real>(NS::cp)),
    _mu_t(getFunctor<Real>(NS::mu_t)),
    _k(getFunctor<Real>(NS::k)),
    _Pr_t(getFunctor<Real>("Pr_t")),
    _turbulent_thermal_conductivity(getParam<bool>("turbulent_thermal_conductivity"))

{
}

Real
EffectiveTurbulentThermalConductivityAux::computeValue()
{
  const auto current_argument = makeElemArg(_current_elem);
  const auto state = determineState();

  if (_turbulent_thermal_conductivity)
    return _cp(current_argument, state) * _mu_t(current_argument, state) /
          _Pr_t(current_argument, state);
  else
    return _cp(current_argument, state) * _mu_t(current_argument, state) /
          _Pr_t(current_argument, state) + _k(current_argument, state);

}
