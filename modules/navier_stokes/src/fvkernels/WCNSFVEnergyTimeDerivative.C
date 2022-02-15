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
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes momentum equation.");
  params.addRequiredParam<MaterialPropertyName>(NS::density, "The density material property");
  params.addRequiredParam<MaterialPropertyName>(NS::cp,
                                                "The specific heat capacity material property");
  params.addRequiredParam<MaterialPropertyName>(
      NS::time_deriv(NS::density), "The time derivative of the density material property");
  params.addRequiredParam<MaterialPropertyName>(
      NS::time_deriv(NS::cp),
      "The time derivative of the specific heat capacity material property");
  return params;
}

WCNSFVEnergyTimeDerivative::WCNSFVEnergyTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params),
    _rho(getFunctor<ADReal>(NS::density)),
    _cp(getFunctor<ADReal>(NS::cp)),
    _rho_dot(getFunctor<ADReal>(NS::time_deriv(NS::density))),
    _cp_dot(getFunctor<ADReal>(NS::time_deriv(NS::cp)))
{
}

ADReal
WCNSFVEnergyTimeDerivative::computeQpResidual()
{
  return _rho_dot(makeElemArg(_current_elem)) * _cp(makeElemArg(_current_elem)) *
             _var(makeElemArg(_current_elem)) +
         _rho(makeElemArg(_current_elem)) * _cp_dot(makeElemArg(_current_elem)) *
             _var(makeElemArg(_current_elem)) +
         _rho(makeElemArg(_current_elem)) * _cp(makeElemArg(_current_elem)) *
             _var.dot(makeElemArg(_current_elem));
}
