//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumGravity.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumGravity);

InputParameters
PINSFVMomentumGravity::validParams()
{
  InputParameters params = INSFVGravityForce::validParams();
  params.addClassDescription("Computes a body force due to gravity on fluid in porous media.");
  params.addRequiredCoupledVar("porosity", "Porosity auxiliary variable");

  return params;
}

PINSFVMomentumGravity::PINSFVMomentumGravity(const InputParameters & params)
  : INSFVGravityForce(params), _eps(coupledValue("porosity"))
{
}

ADReal
PINSFVMomentumGravity::computeQpResidual()
{
  return _eps[_qp] * INSFVGravityForce::computeQpResidual();
}
