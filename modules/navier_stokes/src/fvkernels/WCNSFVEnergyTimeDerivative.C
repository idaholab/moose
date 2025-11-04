//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVEnergyTimeDerivative.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVEnergyTimeDerivative);

InputParameters
WCNSFVEnergyTimeDerivative::validParams()
{
  InputParameters params = INSFVEnergyTimeDerivative::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes momentum equation.");

  params.addRequiredParam<MooseFunctorName>(NS::time_deriv(NS::density),
                                            "The time derivative of the density material property");
  params.addParam<MooseFunctorName>(
      NS::specific_enthalpy, NS::specific_enthalpy, "The specific enthalpy");
  return params;
}

WCNSFVEnergyTimeDerivative::WCNSFVEnergyTimeDerivative(const InputParameters & params)
  : INSFVEnergyTimeDerivative(params),
    _rho_dot(getFunctor<ADReal>(NS::time_deriv(NS::density))),
    _h(getFunctor<ADReal>(NS::specific_enthalpy))
{
}

ADReal
WCNSFVEnergyTimeDerivative::computeQpResidual()
{
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  return INSFVEnergyTimeDerivative::computeQpResidual() +
         _rho_dot(elem_arg, state) * _h(elem_arg, state);
}
