//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVNoSlipWallBC.h"
#include "InputParameters.h"

registerMooseObject("NavierStokesApp", INSFVNoSlipWallBC);

InputParameters
INSFVNoSlipWallBC::validParams()
{
  auto params = FVDirichletBC::validParams();
  params.set<Real>("value") = 0;
  return params;
}

INSFVNoSlipWallBC::INSFVNoSlipWallBC(const InputParameters & params) : FVDirichletBC(params) {}
