//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  return params;
}

WCNSFVEnergyTimeDerivative::WCNSFVEnergyTimeDerivative(const InputParameters & params)
  : INSFVEnergyTimeDerivative(params),
    _rho_dot(getFunctor<ADReal>(getParam<MooseFunctorName>(NS::time_deriv(NS::density))))
{
}

ADReal
WCNSFVEnergyTimeDerivative::computeQpResidual()
{
  const auto & elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  return INSFVEnergyTimeDerivative::computeQpResidual() +
         _rho_dot(elem_arg, state) * _cp(elem_arg, state) * _var(elem_arg, state);
}
