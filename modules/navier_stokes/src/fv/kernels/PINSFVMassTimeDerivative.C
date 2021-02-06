//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMassTimeDerivative.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMassTimeDerivative);

InputParameters
PINSFVMassTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the porosity time derivative term to the porous media incompressible Navier-Stokes mass equation. "
      "This kernel is not required if the porosity is constant in time.");
  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");
  params.addRequiredCoupledVar("porosity", "Porosity auxiliary variable");

  return params;
}

PINSFVMassTimeDerivative::PINSFVMassTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params), _rho(getParam<Real>("rho")), _eps_dot(coupledDot("porosity"))
{
}

ADReal
PINSFVMassTimeDerivative::computeQpResidual()
{
  return _rho * _eps_dot[_qp];
}
