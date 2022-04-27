//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVEnergyAdvection.h"
#include "INSFVEnergyVariable.h"
#include "PINSFVSuperficialVelocityVariable.h"

registerMooseObject("NavierStokesApp", PINSFVEnergyAdvection);

InputParameters
PINSFVEnergyAdvection::validParams()
{
  return INSFVEnergyAdvection::validParams();
}

PINSFVEnergyAdvection::PINSFVEnergyAdvection(const InputParameters & params)
  : INSFVEnergyAdvection(params)
{
}
