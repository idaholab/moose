//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVDensityTimeDerivative.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PCNSFVDensityTimeDerivative);

InputParameters
PCNSFVDensityTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription("A time derivative kernel for which the form is eps * ddt(rho*var).");
  params.addParam<MaterialPropertyName>(NS::porosity, NS::porosity, "The porosity");
  params.addRequiredCoupledVar(NS::density, "The density variable.");
  return params;
}

PCNSFVDensityTimeDerivative::PCNSFVDensityTimeDerivative(const InputParameters & parameters)
  : FVTimeKernel(parameters),
    _u_dot(_var.adUDot()),
    _eps(getMaterialProperty<Real>(NS::porosity)),
    _rho_dot(adCoupledDot(NS::density)),
    _rho(adCoupledValue(NS::density))
{
}

ADReal
PCNSFVDensityTimeDerivative::computeQpResidual()
{
  return _eps[_qp] * (_u[_qp] * _rho_dot[_qp] + _u_dot[_qp] * _rho[_qp]);
}
