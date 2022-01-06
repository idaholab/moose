//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVScalarTimeDerivative.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVScalarTimeDerivative);

InputParameters
WCNSFVScalarTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the weakly compressible Navier-Stokes momentum equation.");
  params.addRequiredParam<MaterialPropertyName>(NS::density, "The density material property");
  params.addRequiredParam<MaterialPropertyName>(
      NS::time_deriv(NS::density), "The time derivative of the density material property");
  return params;
}

WCNSFVScalarTimeDerivative::WCNSFVScalarTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params),
    _rho(getFunctor<ADReal>(NS::density)),
    _rho_dot(getFunctor<ADReal>(NS::time_deriv(NS::density)))
{
}

ADReal
WCNSFVScalarTimeDerivative::computeQpResidual()
{
  const auto elem_arg = makeElemArg(_current_elem);
  return _rho_dot(elem_arg) * _var(elem_arg) + _rho(elem_arg) * _var.dot(elem_arg);
}
