//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADOneD3EqnMomentumFriction.h"

registerMooseObject("ThermalHydraulicsApp", ADOneD3EqnMomentumFriction);

InputParameters
ADOneD3EqnMomentumFriction::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>("D_h", "Hydraulic diameter");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");
  params.addRequiredParam<MaterialPropertyName>("f_D",
                                                "Darcy friction factor coefficient property");
  params.addClassDescription("Computes wall friction term for single phase flow.");
  return params;
}

ADOneD3EqnMomentumFriction::ADOneD3EqnMomentumFriction(const InputParameters & parameters)
  : ADKernel(parameters),
    _A(adCoupledValue("A")),
    _D_h(getADMaterialProperty<Real>("D_h")),
    _rho(getADMaterialProperty<Real>("rho")),
    _vel(getADMaterialProperty<Real>("vel")),
    _f_D(getADMaterialProperty<Real>("f_D"))
{
}

ADReal
ADOneD3EqnMomentumFriction::computeQpResidual()
{
  return 0.5 * _f_D[_qp] * _rho[_qp] * _vel[_qp] * std::abs(_vel[_qp]) * _A[_qp] / _D_h[_qp] *
         _test[_i][_qp];
}
