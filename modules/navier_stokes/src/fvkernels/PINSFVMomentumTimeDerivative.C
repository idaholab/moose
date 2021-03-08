//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumTimeDerivative.h"
#include "PINSFVSuperficialVelocityVariable.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumTimeDerivative);

InputParameters
PINSFVMomentumTimeDerivative::validParams()
{
  InputParameters params = INSFVMomentumTimeDerivative::validParams();
  params.addClassDescription("Adds the time derivative term: d(rho u_d) / dt to the porous media "
                             "incompressible Navier-Stokes momentum equation.");
  return params;
}

PINSFVMomentumTimeDerivative::PINSFVMomentumTimeDerivative(const InputParameters & params)
  : INSFVMomentumTimeDerivative(params)
{
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumTimeDerivative may only be used with a superficial velocity, "
               "of variable type PINSFVSuperficialVelocityVariable.");
}
