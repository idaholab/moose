//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumZeroViscousStreeBC.h"
#include "MooseMesh.h"
#include "INSADObjectTracker.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSADMomentumZeroViscousStreeBC);

InputParameters
INSADMomentumZeroViscousStreeBC::validParams()
{
  InputParameters params = INSADMomentumImplicitStressBC::validParams();
  params.addClassDescription(
      "This class implements a zero normal viscous stress boundary condition.");
  return params;
}

INSADMomentumZeroViscousStreeBC::INSADMomentumZeroViscousStreeBC(const InputParameters & parameters)
  : INSADMomentumImplicitStressBC(parameters)
{
}

ADReal
INSADMomentumZeroViscousStreeBC::viscousStress()
{
  return 0;
}
