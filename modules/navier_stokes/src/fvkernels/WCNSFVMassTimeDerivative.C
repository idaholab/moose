//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVMassTimeDerivative.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVMassTimeDerivative);

InputParameters
WCNSFVMassTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes momentum equation.");
  params.addRequiredParam<MaterialPropertyName>(
      NS::time_deriv(NS::density), "The time derivative of the density material property");
  return params;
}

WCNSFVMassTimeDerivative::WCNSFVMassTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params), _rho_dot(getFunctor<ADReal>(NS::time_deriv(NS::density)))
{
}

ADReal
WCNSFVMassTimeDerivative::computeQpResidual()
{
  return _rho_dot(makeElemArg(_current_elem));
}
