//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumGravity.h"
#include "PINSFVSuperficialVelocityVariable.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumGravity);

InputParameters
PINSFVMomentumGravity::validParams()
{
  InputParameters params = INSFVMomentumGravity::validParams();
  params.addClassDescription(
      "Computes a body force, $eps * \rho * g$ due to gravity on fluid in porous media.");
  params.addRequiredCoupledVar("porosity", "Porosity auxiliary variable");

  return params;
}

PINSFVMomentumGravity::PINSFVMomentumGravity(const InputParameters & params)
  : INSFVMomentumGravity(params), _eps(coupledValue("porosity"))
{
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumGravity may only be used with a superficial velocity "
               "variable, of variable type PINSFVSuperficialVelocityVariable.");
}

ADReal
PINSFVMomentumGravity::computeQpResidual()
{
  return _eps[_qp] * INSFVMomentumGravity::computeQpResidual();
}
