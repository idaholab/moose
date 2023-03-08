//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVNoSlipWallBC.h"
#include "Function.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "FaceInfo.h"

#include "libmesh/vector_value.h"

registerMooseObject("NavierStokesApp", INSFVNoSlipWallBC);

InputParameters
INSFVNoSlipWallBC::validParams()
{
  InputParameters params = FVFunctionDirichletBC::validParams();
  params.addClassDescription("Implements a no slip boundary condition.");
  return params;
}

INSFVNoSlipWallBC::INSFVNoSlipWallBC(const InputParameters & params) : FVFunctionDirichletBC(params)
{
}
