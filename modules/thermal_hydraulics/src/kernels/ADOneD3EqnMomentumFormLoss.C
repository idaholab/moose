//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADOneD3EqnMomentumFormLoss.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADOneD3EqnMomentumFormLoss);

InputParameters
ADOneD3EqnMomentumFormLoss::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");

  return params;
}

ADOneD3EqnMomentumFormLoss::ADOneD3EqnMomentumFormLoss(const InputParameters & parameters)
  : ADKernel(parameters),
    _A(adCoupledValue("A")),
    _rho(getADMaterialProperty<Real>("rho")),
    _vel(getADMaterialProperty<Real>("vel")),
    _K_prime(getADMaterialProperty<Real>("K_prime"))
{
}

ADReal
ADOneD3EqnMomentumFormLoss::computeQpResidual()
{
  return _K_prime[_qp] * 0.5 * _rho[_qp] * _vel[_qp] * std::abs(_vel[_qp]) * _A[_qp] *
         _test[_i][_qp];
}
