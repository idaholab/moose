//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumTimeDerivative.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumTimeDerivative);

InputParameters
PINSFVMomentumTimeDerivative::validParams()
{
  InputParameters params = INSFVMomentumTimeDerivative::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the porous media incompressible Navier-Stokes momentum equation.");
  return params;
}

PINSFVMomentumTimeDerivative::PINSFVMomentumTimeDerivative(const InputParameters & params)
  : INSFVMomentumTimeDerivative(params)
{
  //TODO Check the variable type
}
