//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVNaturalFreeSlipBC.h"

registerMooseObject("NavierStokesApp", INSFVNaturalFreeSlipBC);

InputParameters
INSFVNaturalFreeSlipBC::validParams()
{
  InputParameters params = INSFVSlipWallBC::validParams();
  params.addClassDescription("Implements a free slip boundary condition naturally.");
  return params;
}

INSFVNaturalFreeSlipBC::INSFVNaturalFreeSlipBC(const InputParameters & params)
  : INSFVSlipWallBC(params)
{
}
