//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVEnergyTimeDerivative.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVEnergyTimeDerivative);

InputParameters
PINSFVEnergyTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes energy equation.");
  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "The name of the specific heat capacity");
  params.addRequiredCoupledVar("porosity", "Porosity variable");
  params.addRequiredParam<bool>("is_solid", "Kernel for the solid temperature ?");
  return params;
}

PINSFVEnergyTimeDerivative::PINSFVEnergyTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params),
  _rho(getParam<Real>("rho")),
  _cp(getADMaterialProperty<Real>("cp_name")),
  _eps(adCoupledValue("porosity")),
  _eps_dot(adCoupledDot("porosity")),
  _is_solid(getParam<bool>("is_solid"))
{
}

ADReal
PINSFVEnergyTimeDerivative::computeQpResidual()
{
  // FIXME Neglecting time derivative of _cp
  // FIXME Possibly use a VarMat instead of querying variables as parameters
  return (_is_solid ? 1 - _eps[_qp] : _eps[_qp]) * _rho * _cp[_qp] * FVTimeKernel::computeQpResidual() +
     (_is_solid ? - _eps_dot[_qp] : _eps_dot[_qp]) * _rho * _cp[_qp] * _u[_qp];
}
