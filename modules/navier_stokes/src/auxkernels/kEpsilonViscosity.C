//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "kEpsilonViscosity.h"
#include "INSFVMethods.h"

registerMooseObject("NavierStokesApp", kEpsilonViscosity);

InputParameters
kEpsilonViscosity::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates the turbulent viscosity according to the k-epsilon model.");
  params.addRequiredParam<MooseFunctorName>("k", "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>("epsilon", "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>("C_mu", "Coupled turbulent kinetic energy closure.");
  return params;
}

kEpsilonViscosity::kEpsilonViscosity(const InputParameters & params)
  : AuxKernel(params),
    _k(getFunctor<ADReal>("k")),
    _epsilon(getFunctor<ADReal>("epsilon")),
    _rho(getFunctor<ADReal>(NS::density)),
    _C_mu(getFunctor<ADReal>("C_mu"))
{
}

Real
kEpsilonViscosity::computeValue()
{

  constexpr Real protection_epsilon = 1e-10;

  return (_rho(makeElemArg(_current_elem))
          * _C_mu(makeElemArg(_current_elem))
          * Utility::pow<2>(_k(makeElemArg(_current_elem)))
          / (_epsilon(makeElemArg(_current_elem)) + protection_epsilon)).value();
}