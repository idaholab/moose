//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetTimeDerivativeSUPG.h"

registerMooseObject("LevelSetApp", LevelSetTimeDerivativeSUPG);

InputParameters
LevelSetTimeDerivativeSUPG::validParams()
{
  InputParameters params = ADTimeKernelGrad::validParams();
  params.addClassDescription(
      "SUPG stablization terms for the time derivative of the level set equation.");
  params.addRequiredCoupledVar("velocity", "Velocity vector variable.");
  return params;
}

LevelSetTimeDerivativeSUPG::LevelSetTimeDerivativeSUPG(const InputParameters & parameters)
  : ADTimeKernelGrad(parameters), _velocity(adCoupledVectorValue("velocity"))
{
}

ADRealVectorValue
LevelSetTimeDerivativeSUPG::precomputeQpResidual()
{
  ADReal tau =
      _current_elem->hmin() /
      (2 * (_velocity[_qp] + RealVectorValue(libMesh::TOLERANCE * libMesh::TOLERANCE)).norm());
  return tau * _velocity[_qp] * _u_dot[_qp];
}
